#include "synapse.hpp"
#include <cmath>

namespace flybrain {

Synapse::Synapse(uint32_t id,
                 uint32_t pre_id,
                 uint32_t post_id,
                 SynapseType type,
                 uint32_t num_contacts)
    : id_(id)
    , pre_id_(pre_id)
    , post_id_(post_id)
    , type_(type)
    , neurotransmitter_(Neurotransmitter::GLUTAMATE)
    , num_contacts_(num_contacts)
    , base_weight_(1.0)
    , current_weight_(1.0)
    , delay_ms_(1.5)
    , reversal_potential_(0.0)
    , conductance_(0.0)
    , release_timer_(0.0)
    , plasticity_type_(PlasticityType::NONE)
    , facilitation_(0.0)
    , depression_(1.0)
    , stdp_tau_plus_(20.0)
    , stdp_tau_minus_(20.0)
    , stdp_a_plus_(0.005)
    , stdp_a_minus_(0.005)
{
    // Set reversal potential based on neurotransmitter type
    switch (neurotransmitter_) {
        case Neurotransmitter::GLUTAMATE:
        case Neurotransmitter::ACETYLCHOLINE:
            reversal_potential_ = 0.0;    // Excitatory
            break;
        case Neurotransmitter::GABA:
        case Neurotransmitter::HISTAMINE:
            reversal_potential_ = -70.0;  // Inhibitory
            break;
        default:
            reversal_potential_ = 0.0;
    }
}

void Synapse::onPreSpike(double current_time) {
    // Schedule neurotransmitter release after delay
    release_timer_ = delay_ms_;

    // Update short-term plasticity
    if (plasticity_type_ == PlasticityType::FACILITATION ||
        plasticity_type_ == PlasticityType::DYNAMIC) {
        facilitation_ += 0.5 * (1.0 - facilitation_);
    }
    if (plasticity_type_ == PlasticityType::DEPRESSION ||
        plasticity_type_ == PlasticityType::DYNAMIC) {
        depression_ *= 0.5;  // Reduce available vesicles
    }

    // Update effective weight
    current_weight_ = base_weight_ * (1.0 + facilitation_) * depression_;
}

double Synapse::update(double dt, double post_voltage) {
    // 先检查释放窗口，再递减计时器，避免条件竞争
    // 判断本时间步内是否触发释放（计时器即将归零）
    // Neurotransmitter release happens after delay
    if (release_timer_ > 0.0) {
        release_timer_ -= dt;
        if (release_timer_ <= 0.0) {
            conductance_ += current_weight_ * num_contacts_;
            release_timer_ = 0.0;  // 确保刚好卡在 0，不会继续触发
        }
    }

    // 电导指数衰减
    double tau_decay = 5.0;  // ms
    conductance_ *= std::exp(-dt / tau_decay);

    // 短时程抑制恢复
    if (plasticity_type_ == PlasticityType::DEPRESSION ||
        plasticity_type_ == PlasticityType::DYNAMIC) {
        depression_ += 0.01 * (1.0 - depression_);
    }

    // 促进衰减
    if (plasticity_type_ == PlasticityType::FACILITATION ||
        plasticity_type_ == PlasticityType::DYNAMIC) {
        facilitation_ -= 0.05 * facilitation_;
    }

    // 突触电流：I = g * (E_rev - V)
    // 兴奋性突触 E_rev=0mV, V≈-65mV → I>0 → 去极化（正确方向）
    // 抑制性突触 E_rev=-70mV, V≈-65mV → I<0 → 超极化（正确方向）
    if (conductance_ > 1e-10) {
        return conductance_ * (reversal_potential_ - post_voltage);
    }
    return 0.0;
}

void Synapse::updateSTDP(double pre_spike_time, double post_spike_time) {
    if (plasticity_type_ != PlasticityType::STDP) return;

    double dt = post_spike_time - pre_spike_time;
    double dw = 0.0;

    if (dt > 0) {
        // Post after pre: potentiation
        dw = stdp_a_plus_ * std::exp(-dt / stdp_tau_plus_);
    } else if (dt < 0) {
        // Pre after post: depression
        dw = -stdp_a_minus_ * std::exp(dt / stdp_tau_minus_);
    }

    // Update weight with bounds
    base_weight_ += dw;
    base_weight_ = std::max(0.0, std::min(2.0 * base_weight_, base_weight_));
}

// ElectricalSynapse implementation
ElectricalSynapse::ElectricalSynapse(uint32_t id,
                                     uint32_t pre_id,
                                     uint32_t post_id,
                                     double conductance)
    : Synapse(id, pre_id, post_id, SynapseType::ELECTRICAL, 1)
    , junction_conductance_(conductance)
{
    reversal_potential_ = 0.0;  // Not used for electrical synapses
}

double ElectricalSynapse::update(double dt, double post_voltage) {
    // Electrical synapses pass current based on voltage difference
    // This is simplified - would need pre-synaptic voltage for full model
    (void)dt;  // Unused in electrical synapse model
    return 0.0;  // Current computed differently for electrical synapses
}

} // namespace flybrain
