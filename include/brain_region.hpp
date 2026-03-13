#ifndef BRAIN_REGION_HPP
#define BRAIN_REGION_HPP

#include "neuron.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace flybrain {

/**
 * @brief Represents a structural region of the fruit fly brain
 *
 * Each region contains multiple neurons and has specific:
 * - Anatomical properties
 * - Neuron type distributions
 * - Connectivity patterns
 */
class BrainRegionImpl {
public:
    BrainRegionImpl(BrainRegion type, const std::string& name);

    /**
     * @brief Add a neuron to this region
     */
    void addNeuron(uint32_t neuron_id);

    /**
     * @brief Get all neurons in this region
     */
    const std::vector<uint32_t>& getNeurons() const { return neuron_ids_; }

    /**
     * @brief Get number of neurons in this region
     */
    size_t getNeuronCount() const { return neuron_ids_.size(); }

    /**
     * @brief Get neurons by type
     */
    std::vector<uint32_t> getNeuronsByType(NeuronType type) const;

    /**
     * @brief Get region type
     */
    BrainRegion type() const { return type_; }

    /**
     * @brief Get region name
     */
    const std::string& name() const { return name_; }

    /**
     * @brief Get statistics about this region
     */
    struct RegionStats {
        size_t total_neurons;
        size_t excitatory_count;
        size_t inhibitory_count;
        size_t sensory_count;
        size_t motor_count;
        size_t interneuron_count;
    };

    RegionStats getStats() const;

private:
    BrainRegion type_;
    std::string name_;
    std::vector<uint32_t> neuron_ids_;

    // Index by neuron type for fast lookup
    std::unordered_map<NeuronType, std::vector<uint32_t>> neurons_by_type_;
};

/**
 * @brief Layer information for layered brain structures
 */
struct Layer {
    std::string name;
    double depth;  // micrometers from surface
    size_t neuron_count;
};

/**
 * @brief Optic lobe with layered structure
 */
class OpticLobe : public BrainRegionImpl {
public:
    OpticLobe();

    void addNeuronWithLayer(uint32_t neuron_id, size_t layer_index);

private:
    std::vector<Layer> layers_;
};

/**
 * @brief Mushroom body with compartment structure
 */
class MushroomBody : public BrainRegionImpl {
public:
    MushroomBody();

    void addNeuronToCompartment(uint32_t neuron_id, const std::string& compartment);

private:
    std::unordered_map<std::string, std::vector<uint32_t>> compartments_;
};

} // namespace flybrain

#endif // BRAIN_REGION_HPP
