#include "brain_region.hpp"

namespace flybrain {

BrainRegionImpl::BrainRegionImpl(flybrain::BrainRegion type, const std::string& name)
    : type_(type)
    , name_(name)
{
}

void BrainRegionImpl::addNeuron(uint32_t neuron_id) {
    neuron_ids_.push_back(neuron_id);
}

std::vector<uint32_t> BrainRegionImpl::getNeuronsByType(NeuronType type) const {
    auto it = neurons_by_type_.find(type);
    if (it != neurons_by_type_.end()) {
        return it->second;
    }
    return {};
}

BrainRegionImpl::RegionStats BrainRegionImpl::getStats() const {
    RegionStats stats{};
    stats.total_neurons = neuron_ids_.size();
    stats.excitatory_count = getNeuronsByType(NeuronType::EXCITATORY).size();
    stats.inhibitory_count = getNeuronsByType(NeuronType::INHIBITORY).size();
    stats.sensory_count = getNeuronsByType(NeuronType::SENSORY).size();
    stats.motor_count = getNeuronsByType(NeuronType::MOTOR).size();
    stats.interneuron_count = getNeuronsByType(NeuronType::INTERNEURON).size();
    return stats;
}

// OpticLobe implementation
OpticLobe::OpticLobe()
    : BrainRegionImpl(flybrain::BrainRegion::OPTIC_LOBE, "Optic Lobe")
{
    // Initialize layers (Lamina, Medulla, Lobula, Lobula Plate)
    layers_ = {
        {"Lamina", 0.0, 0},
        {"Medulla", 50.0, 0},
        {"Lobula", 100.0, 0},
        {"Lobula Plate", 75.0, 0}
    };
}

void OpticLobe::addNeuronWithLayer(uint32_t neuron_id, size_t layer_index) {
    if (layer_index < layers_.size()) {
        layers_[layer_index].neuron_count++;
        addNeuron(neuron_id);
    }
}

// MushroomBody implementation
MushroomBody::MushroomBody()
    : BrainRegionImpl(flybrain::BrainRegion::MUSHROOM_BODY, "Mushroom Body")
{
    // Initialize compartments
    compartments_ = {
        {"calyx", {}},
        {"peduncle", {}},
        {"alpha_lobe", {}},
        {"beta_lobe", {}},
        {"gamma_lobe", {}}
    };
}

void MushroomBody::addNeuronToCompartment(uint32_t neuron_id, const std::string& compartment) {
    auto it = compartments_.find(compartment);
    if (it != compartments_.end()) {
        it->second.push_back(neuron_id);
        addNeuron(neuron_id);
    }
}

} // namespace flybrain
