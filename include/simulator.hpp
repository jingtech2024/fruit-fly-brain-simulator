#ifndef SIMULATOR_HPP
#define SIMULATOR_HPP

#include "connectome.hpp"
#include <vector>
#include <chrono>
#include <functional>
#include <memory>

namespace flybrain {

/**
 * @brief Simulation configuration
 */
struct SimulationConfig {
    double dt = 0.1;              // Time step (ms)
    double duration = 1000.0;     // Total simulation time (ms)
    double temperature = 25.0;    // Temperature (Celsius) - affects kinetics
    bool record_spikes = true;    // Record spike times
    bool record_voltage = false;  // Record membrane potentials
    size_t report_interval = 100; // Report progress every N steps
    bool use_parallel = true;     // Use OpenMP parallelization
};

/**
 * @brief Spike recording data
 */
struct SpikeRecord {
    uint32_t neuron_id;
    double time;  // ms
};

/**
 * @brief Voltage recording data
 */
struct VoltageRecord {
    uint32_t neuron_id;
    std::vector<double> voltages;
    std::vector<double> times;
};

/**
 * @brief Simulation statistics
 */
struct SimulationStats {
    size_t total_steps;
    size_t total_spikes;
    double simulated_time;     // ms
    double real_time;          // seconds
    double speedup;            // simulated_time / real_time
    size_t steps_per_second;
};

/**
 * @brief Stimulus types for simulation input
 */
enum class StimulusType {
    NONE,           // No stimulus
    STEP,           // Step current
    RAMP,           // Ramp current
    SINUSOIDAL,     // Sinusoidal current
    NOISE,          // White noise current
    SPIKE_TRAIN,    // Prespecified spike times
    PATTERN         // Spatiotemporal pattern
};

/**
 * @brief Stimulus configuration
 */
struct Stimulus {
    StimulusType type = StimulusType::NONE;
    std::vector<uint32_t> target_neurons;
    double amplitude = 0.0;    // nA
    double start_time = 0.0;   // ms
    double end_time = 0.0;     // ms
    double frequency = 0.0;    // Hz (for sinusoidal)
    std::vector<double> spike_times;  // For SPIKE_TRAIN
};

/**
 * @brief Main simulator class
 *
 * Implements parallelized simulation of the fruit fly brain:
 * - Time-stepping with configurable dt
 * - Parallel neuron updates via OpenMP
 * - Synaptic current accumulation
 * - Spike propagation with delays
 */
class Simulator {
public:
    Simulator();
    explicit Simulator(Connectome& connectome);
    ~Simulator();

    /**
     * @brief Set the connectome to simulate
     */
    void setConnectome(Connectome& connectome);

    /**
     * @brief Configure simulation parameters
     */
    void setConfig(const SimulationConfig& config);

    /**
     * @brief Add stimulus to specific neurons
     */
    void addStimulus(const Stimulus& stimulus);

    /**
     * @brief Clear all stimuli
     */
    void clearStimuli();

    /**
     * @brief Add callback for spike events
     */
    using SpikeCallback = std::function<void(uint32_t neuron_id, double time)>;
    void addSpikeCallback(SpikeCallback callback);

    /**
     * @brief Run the simulation
     * @return true if successful
     */
    bool run();

    /**
     * @brief Run simulation asynchronously (in separate thread)
     */
    void runAsync();

    /**
     * @brief Stop a running simulation
     */
    void stop();

    /**
     * @brief Check if simulation is running
     */
    bool isRunning() const { return running_; }

    /**
     * @brief Get current simulation time
     */
    double currentTime() const { return current_time_; }

    /**
     * @brief Get simulation progress (0.0 to 1.0)
     */
    double progress() const;

    /**
     * @brief Get recorded spikes
     */
    const std::vector<SpikeRecord>& getSpikes() const { return spike_records_; }

    /**
     * @brief Get recorded voltages for a neuron
     */
    VoltageRecord getVoltageRecord(uint32_t neuron_id) const;

    /**
     * @brief Get simulation statistics
     */
    SimulationStats getStats() const { return stats_; }

    /**
     * @brief Reset simulation state
     */
    void reset();

    /**
     * @brief Set random seed for reproducibility
     */
    void setSeed(uint32_t seed);

private:
    /**
     * @brief Compute synaptic currents for all neurons
     */
    void computeSynapticCurrents();

    /**
     * @brief Update all neurons for one time step
     */
    void updateNeurons();

    /**
     * @brief Propagate spikes through synapses
     */
    void propagateSpikes();

    /**
     * @brief Apply stimuli
     */
    void applyStimuli();

    /**
     * @brief Initialize delayed spikes
     */
    void initializeDelayedSpikes();

    Connectome* connectome_;
    SimulationConfig config_;
    std::vector<Stimulus> stimuli_;

    // Simulation state
    double current_time_;
    double end_time_;
    size_t current_step_;
    size_t total_steps_;
    bool running_;
    bool stop_requested_;

    // Spike management
    std::vector<SpikeRecord> spike_records_;
    std::vector<std::pair<uint32_t, double>> delayed_spikes_;  // (synapse_id, release_time)
    std::vector<SpikeCallback> spike_callbacks_;

    // Voltage recording
    std::vector<VoltageRecord> voltage_records_;

    // Synaptic currents (one per neuron)
    std::vector<double> synaptic_currents_;

    // Statistics
    SimulationStats stats_;

    // Random number generation
    uint32_t seed_;
    std::mt19937 rng_;

    // Timing
    std::chrono::steady_clock::time_point start_time_;
};

/**
 * @brief Network analysis utilities
 */
class NetworkAnalyzer {
public:
    /**
     * @brief Compute firing rate for a neuron
     */
    static double computeFiringRate(const std::vector<SpikeRecord>& spikes,
                                    uint32_t neuron_id,
                                    double window_ms);

    /**
     * @brief Compute population firing rate
     */
    static double computePopulationRate(const std::vector<SpikeRecord>& spikes,
                                        double window_ms,
                                        double time_ms);

    /**
     * @brief Detect synchronous firing events
     */
    static std::vector<std::pair<double, size_t>> detectSynchrony(
        const std::vector<SpikeRecord>& spikes,
        double bin_size_ms);

    /**
     * @brief Compute correlation matrix (subset of neurons)
     */
    static std::vector<std::vector<double>> computeCorrelationMatrix(
        const std::vector<SpikeRecord>& spikes,
        const std::vector<uint32_t>& neuron_ids,
        double bin_size_ms);
};

} // namespace flybrain

#endif // SIMULATOR_HPP
