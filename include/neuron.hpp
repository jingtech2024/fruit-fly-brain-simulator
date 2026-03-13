#ifndef NEURON_HPP
#define NEURON_HPP

#include <vector>
#include <random>
#include <cstdint>

namespace flybrain {

/**
 * @brief Types of neurons in the fruit fly brain
 */
enum class NeuronType : uint8_t {
    EXCITATORY = 0,
    INHIBITORY = 1,
    MODULATORY = 2,
    SENSORY = 3,
    MOTOR = 4,
    INTERNEURON = 5
};

/**
 * @brief Brain region identifiers
 */
enum class BrainRegion : uint8_t {
    UNKNOWN = 0,
    OPTIC_LOBE = 1,          // Visual processing
    ANTENNAL_LOBE = 2,       // Olfactory processing
    MUSHROOM_BODY = 3,       // Learning and memory
    CENTRAL_COMPLEX = 4,     // Navigation and motor control
    LATERAL_HORN = 5,        // Motor output
    SUBESOPHAGEAL_ZONE = 6,  // Feeding and taste
    VENTRAL_NERVE_CORD = 7   // Motor neurons
};

/**
 * @brief Simplified integrate-and-fire neuron model
 *
 * This model simulates basic neuron dynamics:
 * - Membrane potential integration
 * - Spike generation when threshold is reached
 * - Refractory period after spiking
 */
class Neuron {
public:
    Neuron(uint32_t id = 0,
           NeuronType type = NeuronType::INTERNEURON,
           BrainRegion region = BrainRegion::UNKNOWN);

    virtual ~Neuron() = default;

    /**
     * @brief Update neuron state for one time step
     * @param dt Time step in milliseconds
     * @param input_current Total input current from synapses
     */
    virtual void update(double dt, double input_current);

    /**
     * @brief Check if neuron spiked in last time step
     */
    bool hasSpiked() const { return spiked_last_step_; }

    /**
     * @brief Reset neuron state
     */
    virtual void reset();

    // Getters
    uint32_t id() const { return id_; }
    NeuronType type() const { return type_; }
    BrainRegion region() const { return region_; }
    double membranePotential() const { return membrane_potential_; }
    bool isRefractory() const { return refractory_timer_ > 0; }

    // Setters
    void setRestingPotential(double v) { resting_potential_ = v; }
    void setThreshold(double v) { threshold_ = v; }
    void setRefractoryPeriod(double v) { refractory_period_ = v; }
    void setMembraneTimeConstant(double v) { tau_m_ = v; }

protected:
    uint32_t id_;
    NeuronType type_;
    BrainRegion region_;

    // Membrane properties (protected for derived classes)
    double membrane_potential_;
    double resting_potential_;
    double threshold_;
    double refractory_period_;  // ms
    double refractory_timer_;   // ms
    double tau_m_;              // Membrane time constant (ms)

    bool spiked_last_step_;

private:
};

/**
 * @brief Leaky integrate-and-fire neuron with adaptive threshold
 *
 * Extends basic model with:
 * - Adaptive threshold (spike-frequency adaptation)
 * - Multiple ion channel types
 */
class AdaptiveNeuron : public Neuron {
public:
    AdaptiveNeuron(uint32_t id = 0,
                   NeuronType type = NeuronType::INTERNEURON,
                   BrainRegion region = BrainRegion::UNKNOWN);

    void update(double dt, double input_current);
    void reset();

private:
    double adaptation_current_;
    double adaptation_time_constant_;
    double adaptation_increment_;
};

} // namespace flybrain

#endif // NEURON_HPP
