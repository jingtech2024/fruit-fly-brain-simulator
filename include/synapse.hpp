#ifndef SYNAPSE_HPP
#define SYNAPSE_HPP

#include <cstdint>
#include <random>

namespace flybrain {

/**
 * @brief Types of synapses
 */
enum class SynapseType : uint8_t {
    CHEMICAL = 0,    // Chemical synapse with neurotransmitter release
    ELECTRICAL = 1,  // Gap junction (electrical coupling)
    MODULATORY = 2   // Neuromodulatory synapse
};

/**
 * @brief Neurotransmitter types for chemical synapses
 */
enum class Neurotransmitter : uint8_t {
    GLUTAMATE = 0,   // Primary excitatory
    GABA = 1,        // Primary inhibitory
    ACETYLCHOLINE = 2,
    DOPAMINE = 3,    // Modulatory
    SEROTONIN = 4,   // Modulatory
    OCTOPAMINE = 5,  // Modulatory (invertebrate specific)
    HISTAMINE = 6,
    PEPTIDE = 7
};

/**
 * @brief Synaptic plasticity type
 */
enum class PlasticityType : uint8_t {
    NONE = 0,           // Fixed weight
    STDP = 1,           // Spike-timing dependent plasticity
    FACILITATION = 2,   // Short-term facilitation
    DEPRESSION = 3,     // Short-term depression
    DYNAMIC = 4         // Combined facilitation and depression
};

/**
 * @brief Synapse connecting two neurons
 *
 * Models chemical synapse with:
 * - Conductance-based transmission
 * - Short-term plasticity
 * - Propagation delay
 */
class Synapse {
public:
    Synapse(uint32_t id = 0,
            uint32_t pre_id = 0,
            uint32_t post_id = 0,
            SynapseType type = SynapseType::CHEMICAL,
            uint32_t num_contacts = 1);

    virtual ~Synapse() = default;

    /**
     * @brief Process pre-synaptic spike
     * @param current_time Current simulation time
     */
    void onPreSpike(double current_time);

    /**
     * @brief Update synapse state
     * @param dt Time step
     * @param post_voltage Post-synaptic membrane potential
     * @return Synaptic current contribution
     */
    virtual double update(double dt, double post_voltage);

    /**
     * @brief Update synaptic weight via STDP
     * @param pre_spike_time Time of pre-synaptic spike
     * @param post_spike_time Time of post-synaptic spike
     */
    void updateSTDP(double pre_spike_time, double post_spike_time);

    // Getters
    uint32_t id() const { return id_; }
    uint32_t preId() const { return pre_id_; }
    uint32_t postId() const { return post_id_; }
    SynapseType type() const { return type_; }
    double weight() const { return base_weight_; }
    uint32_t numContacts() const { return num_contacts_; }
    Neurotransmitter neurotransmitter() const { return neurotransmitter_; }

    // Setters
    void setWeight(double w) { base_weight_ = w; }
    void setDelay(double d) { delay_ms_ = d; }
    void setPlasticityType(PlasticityType pt) { plasticity_type_ = pt; }
    void setReversalPotential(double e) { reversal_potential_ = e; }

    // Check if synapse is active (has pending release)
    bool isActive() const { return release_timer_ > 0; }

protected:
    uint32_t id_;
    uint32_t pre_id_;
    uint32_t post_id_;

    SynapseType type_;
    Neurotransmitter neurotransmitter_;
    uint32_t num_contacts_;  // Number of synaptic contacts (multi-contact)

    // Synaptic properties
    double base_weight_;     // Maximum conductance (nS)
    double current_weight_;  // Current effective weight
    double delay_ms_;        // Transmission delay (ms)
    double reversal_potential_; // mV

    // Dynamics
    double conductance_;     // Current conductance (nS)
    double release_timer_;   // Time until neurotransmitter release

    // Short-term plasticity
    PlasticityType plasticity_type_;
    double facilitation_;    // Facilitation variable (0-1)
    double depression_;      // Depression variable (0-1)

    // STDP parameters
    double stdp_tau_plus_;   // ms
    double stdp_tau_minus_;  // ms
    double stdp_a_plus_;     // Learning rate
    double stdp_a_minus_;    // Learning rate
};

/**
 * @brief Electrical synapse (gap junction)
 */
class ElectricalSynapse : public Synapse {
public:
    ElectricalSynapse(uint32_t id = 0,
                      uint32_t pre_id = 0,
                      uint32_t post_id = 0,
                      double conductance = 1.0);

    double update(double dt, double post_voltage) override;

private:
    double junction_conductance_;  // nS
};

} // namespace flybrain

#endif // SYNAPSE_HPP
