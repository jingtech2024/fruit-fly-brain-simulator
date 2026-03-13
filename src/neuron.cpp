#include "neuron.hpp"
#include <cmath>

namespace flybrain {

Neuron::Neuron(uint32_t id,
               NeuronType type,
               BrainRegion region)
    : id_(id)
    , type_(type)
    , region_(region)
    , membrane_potential_(-65.0)  // mV, typical resting potential
    , resting_potential_(-65.0)   // mV
    , threshold_(-50.0)           // mV, spike threshold
    , refractory_period_(25.0)    // ms, 最大饱和放电率被限制为 40 Hz
    , refractory_timer_(0.0)      // ms
    , tau_m_(20.0)                // ms, membrane time constant
    , spiked_last_step_(false)
{
}

void Neuron::update(double dt, double input_current) {
    spiked_last_step_ = false;

    // Check if in refractory period
    if (refractory_timer_ > 0) {
        refractory_timer_ -= dt;
        membrane_potential_ = resting_potential_;
        return;
    }

    // LIF 方程：dV/dt = (V_rest - V + I * R) / tau_m
    // R = 5 MΩ，阈值电流 I_th = (V_th - V_rest) / R = 15/5 = 3 nA
    double dv = (resting_potential_ - membrane_potential_ + input_current * 5.0) / tau_m_;
    membrane_potential_ += dv * dt;

    // Check for spike
    if (membrane_potential_ >= threshold_) {
        spiked_last_step_ = true;
        membrane_potential_ = 0.0;  // Spike peak
        refractory_timer_ = refractory_period_;
    }
}

void Neuron::reset() {
    membrane_potential_ = resting_potential_;
    refractory_timer_ = 0.0;
    spiked_last_step_ = false;
}

// AdaptiveNeuron implementation
AdaptiveNeuron::AdaptiveNeuron(uint32_t id,
                               NeuronType type,
                               BrainRegion region)
    : Neuron(id, type, region)
    , adaptation_current_(0.0)
    , adaptation_time_constant_(100.0)  // ms
    , adaptation_increment_(0.5)        // nA
{
}

void AdaptiveNeuron::update(double dt, double input_current) {
    spiked_last_step_ = false;

    // Update adaptation current
    double da = -adaptation_current_ / adaptation_time_constant_;
    adaptation_current_ += da * dt;

    if (refractory_timer_ > 0) {
        refractory_timer_ -= dt;
        membrane_potential_ = resting_potential_;
        return;
    }

    // Include adaptation in membrane dynamics
    double effective_current = input_current - adaptation_current_;
    double dv = (resting_potential_ - membrane_potential_ + effective_current * 5.0) / tau_m_;
    membrane_potential_ += dv * dt;

    // Check for spike
    if (membrane_potential_ >= threshold_) {
        spiked_last_step_ = true;
        adaptation_current_ += adaptation_increment_;
        membrane_potential_ = 0.0;
        refractory_timer_ = refractory_period_;
    }
}

void AdaptiveNeuron::reset() {
    Neuron::reset();
    adaptation_current_ = 0.0;
}

} // namespace flybrain
