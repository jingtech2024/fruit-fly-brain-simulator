#include "simulator.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

#ifdef _OPENMP
#include <omp.h>
#endif

namespace flybrain {

Simulator::Simulator()
    : connectome_(nullptr)
    , current_time_(0.0)
    , end_time_(0.0)
    , current_step_(0)
    , total_steps_(0)
    , running_(false)
    , stop_requested_(false)
    , seed_(42)
    , rng_(42)
{
}

Simulator::Simulator(Connectome& connectome)
    : connectome_(&connectome)
    , current_time_(0.0)
    , end_time_(0.0)
    , current_step_(0)
    , total_steps_(0)
    , running_(false)
    , stop_requested_(false)
    , seed_(42)
    , rng_(42)
{
}

Simulator::~Simulator() = default;

void Simulator::setConnectome(Connectome& connectome) {
    connectome_ = &connectome;
}

void Simulator::setConfig(const SimulationConfig& config) {
    config_ = config;
}

void Simulator::addStimulus(const Stimulus& stimulus) {
    stimuli_.push_back(stimulus);
}

void Simulator::clearStimuli() {
    stimuli_.clear();
}

void Simulator::addSpikeCallback(SpikeCallback callback) {
    spike_callbacks_.push_back(std::move(callback));
}

void Simulator::reset() {
    current_time_ = 0.0;
    current_step_ = 0;
    spike_records_.clear();
    delayed_spikes_.clear();
    synaptic_currents_.clear();
    running_ = false;
    stop_requested_ = false;

    // Reset all neurons
    if (connectome_) {
        for (auto& neuron : connectome_->neurons()) {
            neuron.reset();
        }
    }
}

void Simulator::setSeed(uint32_t seed) {
    seed_ = seed;
    rng_.seed(seed);
}

void Simulator::initializeDelayedSpikes() {
    // Initialize delay line for spike propagation
    const auto& conn = connectome_->connectivity();
    for (const auto& synapse : conn.synapses()) {
        delayed_spikes_.emplace_back(synapse.id(), 0.0);
    }
}

bool Simulator::run() {
    if (!connectome_) {
        std::cerr << "Error: No connectome set" << std::endl;
        return false;
    }

    reset();

    end_time_ = config_.duration;
    total_steps_ = static_cast<size_t>(config_.duration / config_.dt);
    synaptic_currents_.resize(connectome_->numNeurons(), 0.0);

    initializeDelayedSpikes();

    running_ = true;
    start_time_ = std::chrono::steady_clock::now();
    size_t spike_count = 0;

    #ifdef _OPENMP
    if (config_.use_parallel) {
        std::cout << "Using OpenMP with " << omp_get_max_threads() << " threads" << std::endl;
    }
    #endif

    // Main simulation loop
    while (current_time_ < end_time_ && !stop_requested_) {
        // 先计算突触电流（内部会清零 synaptic_currents_ 再累加突触贡献）
        computeSynapticCurrents();

        // 再叠加外部刺激电流（在突触电流基础上叠加，不会被清零）
        applyStimuli();

        // Update neurons
        updateNeurons();

        // Record spikes
        if (config_.record_spikes) {
            for (size_t i = 0; i < connectome_->numNeurons(); ++i) {
                if (connectome_->getNeuron(i).hasSpiked()) {
                    spike_records_.push_back({static_cast<uint32_t>(i), current_time_});
                    spike_count++;

                    // Call callbacks
                    for (auto& callback : spike_callbacks_) {
                        callback(static_cast<uint32_t>(i), current_time_);
                    }
                }
            }
        }

        // Propagate spikes through synapses
        propagateSpikes();

        current_time_ += config_.dt;
        current_step_++;

        // Progress reporting
        if (config_.report_interval > 0 && current_step_ % config_.report_interval == 0) {
            double elapsed = std::chrono::duration<double>(
                std::chrono::steady_clock::now() - start_time_).count();
            double progress = 100.0 * current_time_ / config_.duration;
            std::cout << "Progress: " << progress << "% ("
                      << current_time_ << "/" << config_.duration << " ms), "
                      << spike_count << " spikes" << std::endl;
        }
    }

    running_ = false;

    // Compute statistics
    double real_time = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - start_time_).count();

    stats_.total_steps = current_step_;
    stats_.total_spikes = spike_count;
    stats_.simulated_time = current_time_;
    stats_.real_time = real_time;
    stats_.speedup = stats_.simulated_time / (stats_.real_time * 1000.0);
    stats_.steps_per_second = static_cast<size_t>(current_step_ / real_time);

    std::cout << "\nSimulation complete!" << std::endl;
    std::cout << "Simulated " << current_time_ << " ms in " << real_time << " s" << std::endl;
    std::cout << "Speedup: " << stats_.speedup << "x" << std::endl;
    std::cout << "Total spikes: " << spike_count << std::endl;

    return true;
}

void Simulator::runAsync() {
    // Would run in separate thread in full implementation
    run();
}

void Simulator::stop() {
    stop_requested_ = true;
}

double Simulator::progress() const {
    if (total_steps_ == 0) return 0.0;
    return static_cast<double>(current_step_) / total_steps_;
}

VoltageRecord Simulator::getVoltageRecord(uint32_t neuron_id) const {
    for (const auto& record : voltage_records_) {
        if (record.neuron_id == neuron_id) {
            return record;
        }
    }
    return VoltageRecord{neuron_id, {}, {}};
}

void Simulator::computeSynapticCurrents() {
    std::fill(synaptic_currents_.begin(), synaptic_currents_.end(), 0.0);

    const auto& conn = connectome_->connectivity();
    auto& neurons = connectome_->neurons();

    #ifdef _OPENMP
    if (config_.use_parallel) {
        #pragma omp parallel for schedule(dynamic, 64)
        for (size_t i = 0; i < neurons.size(); ++i) {
            double current = 0.0;
            const auto& incoming = conn.getIncoming(static_cast<uint32_t>(i));

            for (uint32_t syn_id : incoming) {
                auto& syn = const_cast<Synapse&>(conn.getSynapse(syn_id));
                current += syn.update(config_.dt, neurons[i].membranePotential());
            }
            synaptic_currents_[i] = current;
        }
        return;
    }
    #endif

    // Serial version
    for (size_t i = 0; i < neurons.size(); ++i) {
        double current = 0.0;
        const auto& incoming = conn.getIncoming(static_cast<uint32_t>(i));

        for (uint32_t syn_id : incoming) {
            auto& syn = const_cast<Synapse&>(conn.getSynapse(syn_id));
            current += syn.update(config_.dt, neurons[i].membranePotential());
        }
        synaptic_currents_[i] = current;
    }
}

void Simulator::updateNeurons() {
    auto& neurons = connectome_->neurons();

    #ifdef _OPENMP
    if (config_.use_parallel) {
        #pragma omp parallel for schedule(dynamic, 64)
        for (size_t i = 0; i < neurons.size(); ++i) {
            neurons[i].update(config_.dt, synaptic_currents_[i]);
        }
        return;
    }
    #endif

    // Serial version
    for (size_t i = 0; i < neurons.size(); ++i) {
        neurons[i].update(config_.dt, synaptic_currents_[i]);
    }
}

void Simulator::propagateSpikes() {
    const auto& conn = connectome_->connectivity();

    for (size_t i = 0; i < connectome_->numNeurons(); ++i) {
        if (connectome_->getNeuron(i).hasSpiked()) {
            const auto& outgoing = conn.getOutgoing(static_cast<uint32_t>(i));

            for (uint32_t syn_id : outgoing) {
                auto& syn = const_cast<Synapse&>(conn.getSynapse(syn_id));
                syn.onPreSpike(current_time_);
            }
        }
    }
}

void Simulator::applyStimuli() {
    auto& neurons = connectome_->neurons();

    for (const auto& stimulus : stimuli_) {
        if (current_time_ < stimulus.start_time || current_time_ > stimulus.end_time) {
            continue;
        }

        double amplitude = stimulus.amplitude;

        // Modulate amplitude based on stimulus type
        switch (stimulus.type) {
            case StimulusType::SINUSOIDAL: {
                double freq_rad = 2.0 * M_PI * stimulus.frequency * (current_time_ / 1000.0);
                amplitude *= std::sin(freq_rad);
                break;
            }
            case StimulusType::RAMP: {
                double ratio = (current_time_ - stimulus.start_time) /
                               (stimulus.end_time - stimulus.start_time);
                amplitude *= ratio;
                break;
            }
            case StimulusType::NOISE: {
                std::uniform_real_distribution<> dist(-1.0, 1.0);
                amplitude *= dist(rng_);
                break;
            }
            default:
                break;
        }

        // Apply to target neurons
        if (stimulus.type == StimulusType::NOISE) {
            // 噪声模式：每个神经元独立随机，避免同步放电
            std::uniform_real_distribution<> noise_dist(-1.0, 1.0);
            for (uint32_t neuron_id : stimulus.target_neurons) {
                if (neuron_id < neurons.size()) {
                    synaptic_currents_[neuron_id] += stimulus.amplitude * noise_dist(rng_);
                }
            }
        } else {
            for (uint32_t neuron_id : stimulus.target_neurons) {
                if (neuron_id < neurons.size()) {
                    synaptic_currents_[neuron_id] += amplitude;
                }
            }
        }
    }
}

// NetworkAnalyzer implementation
double NetworkAnalyzer::computeFiringRate(const std::vector<SpikeRecord>& spikes,
                                          uint32_t neuron_id,
                                          double window_ms) {
    size_t count = 0;
    double max_time = 0.0;

    for (const auto& spike : spikes) {
        if (spike.neuron_id == neuron_id) {
            count++;
            max_time = std::max(max_time, spike.time);
        }
    }

    if (max_time < window_ms) return 0.0;
    return static_cast<double>(count) / (max_time / 1000.0);  // Hz
}

double NetworkAnalyzer::computePopulationRate(const std::vector<SpikeRecord>& spikes,
                                              double window_ms,
                                              double time_ms) {
    size_t count = 0;

    for (const auto& spike : spikes) {
        if (spike.time >= time_ms - window_ms && spike.time <= time_ms) {
            count++;
        }
    }

    return static_cast<double>(count) / (window_ms / 1000.0);  // spikes/s
}

std::vector<std::pair<double, size_t>> NetworkAnalyzer::detectSynchrony(
    const std::vector<SpikeRecord>& spikes,
    double bin_size_ms) {

    std::vector<std::pair<double, size_t>> synchrony;

    if (spikes.empty()) return synchrony;

    // Bin spikes and count per bin
    double current_bin = 0.0;
    size_t bin_count = 0;

    for (const auto& spike : spikes) {
        double bin = std::floor(spike.time / bin_size_ms) * bin_size_ms;

        if (bin != current_bin) {
            if (bin_count > 1) {  // More than 1 spike = synchronous
                synchrony.emplace_back(current_bin, bin_count);
            }
            current_bin = bin;
            bin_count = 0;
        }
        bin_count++;
    }

    return synchrony;
}

std::vector<std::vector<double>> NetworkAnalyzer::computeCorrelationMatrix(
    const std::vector<SpikeRecord>& spikes,
    const std::vector<uint32_t>& neuron_ids,
    double bin_size_ms) {

    size_t n = neuron_ids.size();
    std::vector<std::vector<double>> matrix(n, std::vector<double>(n, 0.0));

    // Simplified correlation computation
    // Full implementation would use proper Pearson correlation

    for (size_t i = 0; i < n; ++i) {
        matrix[i][i] = 1.0;
        for (size_t j = i + 1; j < n; ++j) {
            // Compute correlation coefficient
            matrix[i][j] = 0.0;  // Placeholder
            matrix[j][i] = matrix[i][j];
        }
    }

    return matrix;
}

} // namespace flybrain
