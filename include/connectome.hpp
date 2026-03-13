#ifndef CONNECTOME_HPP
#define CONNECTOME_HPP

#include "neuron.hpp"
#include "synapse.hpp"
#include "brain_region.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

namespace flybrain {

/**
 * @brief Statistics about the connectome
 */
struct ConnectomeStats {
    size_t total_neurons;
    size_t total_synapses;
    size_t total_edges;  // Unique connections

    size_t excitatory_neurons;
    size_t inhibitory_neurons;

    size_t chemical_synapses;
    size_t electrical_synapses;

    double mean_degree;
    double mean_in_degree;
    double mean_out_degree;
};

/**
 * @brief Sparse connectivity matrix for efficient storage
 *
 * Stores synaptic connections in compressed format.
 * The fruit fly brain has ~140k neurons and ~50M synapses,
 * giving ~0.025% connectivity (very sparse).
 */
class ConnectivityMatrix {
public:
    ConnectivityMatrix(size_t num_neurons);

    /**
     * @brief Add a synaptic connection
     * @return Synapse ID
     */
    uint32_t addConnection(uint32_t pre_neuron, uint32_t post_neuron,
                           SynapseType type = SynapseType::CHEMICAL,
                           uint32_t num_contacts = 1);

    /**
     * @brief Get all outgoing connections from a neuron
     */
    const std::vector<uint32_t>& getOutgoing(uint32_t neuron_id) const;

    /**
     * @brief Get all incoming connections to a neuron
     */
    const std::vector<uint32_t>& getIncoming(uint32_t neuron_id) const;

    /**
     * @brief Check if connection exists
     */
    bool hasConnection(uint32_t pre, uint32_t post) const;

    /**
     * @brief Get synapse by ID
     */
    Synapse& getSynapse(uint32_t synapse_id);
    const Synapse& getSynapse(uint32_t synapse_id) const;

    /**
     * @brief Get all synapses
     */
    std::vector<Synapse>& synapses();
    const std::vector<Synapse>& synapses() const;

    /**
     * @brief Get number of connections
     */
    size_t numConnections() const { return synapses_.size(); }

    /**
     * @brief Build adjacency index for fast lookup
     */
    void buildIndex();

private:
    size_t num_neurons_;
    std::vector<Synapse> synapses_;

    // Adjacency lists (store synapse indices)
    std::vector<std::vector<uint32_t>> outgoing_;
    std::vector<std::vector<uint32_t>> incoming_;

    // Fast lookup for existing connections
    std::unordered_map<uint64_t, uint32_t> connection_map_;  // (pre, post) -> synapse_id

    uint32_t next_synapse_id_;
};

/**
 * @brief Complete connectome of the fruit fly brain
 *
 * Manages:
 * - All neurons and their properties
 * - All synaptic connections
 * - Brain region organization
 * - Data loading from external sources
 */
class Connectome {
public:
    Connectome();
    ~Connectome();

    /**
     * @brief Initialize with specified number of neurons
     */
    void initialize(size_t num_neurons);

    /**
     * @brief Load connectome from data file
     * @param filename Path to connectivity data (CSV, HDF5, etc.)
     * @param format Data format specifier
     */
    bool loadFromFile(const std::string& filename, const std::string& format = "auto");

    /**
     * @brief Load from neuPrint database format
     */
    bool loadFromNeuPrint(const std::string& filename);

    /**
     * @brief Generate synthetic connectome with realistic statistics
     * @param num_neurons Number of neurons
     * @param connection_probability Probability of connection (0.0-1.0)
     * @param use_distance_decay Use distance-dependent connection probability
     */
    void generateSynthetic(size_t num_neurons,
                          double connection_probability = 0.0003,
                          bool use_distance_decay = true);

    /**
     * @brief Get neuron by ID
     */
    Neuron& getNeuron(uint32_t id);
    const Neuron& getNeuron(uint32_t id) const;

    /**
     * @brief Get all neurons
     */
    std::vector<Neuron>& neurons();
    const std::vector<Neuron>& neurons() const;

    /**
     * @brief Get connectivity matrix
     */
    ConnectivityMatrix& connectivity();
    const ConnectivityMatrix& connectivity() const;

    /**
     * @brief Get brain regions
     */
    std::vector<std::unique_ptr<BrainRegionImpl>>& regions();

    /**
     * @brief Assign neuron to a brain region
     */
    void assignToRegion(uint32_t neuron_id, BrainRegion region_type);

    /**
     * @brief Get connectome statistics
     */
    ConnectomeStats getStats() const;

    /**
     * @brief Get number of neurons
     */
    size_t numNeurons() const { return neurons_.size(); }

    /**
     * @brief Get number of synapses
     */
    size_t numSynapses() const { return connectivity_.numConnections(); }

    /**
     * @brief Save connectome to file
     */
    bool saveToFile(const std::string& filename, const std::string& format = "csv");

    /**
     * @brief Print summary statistics
     */
    void printStats() const;

private:
    std::vector<Neuron> neurons_;
    ConnectivityMatrix connectivity_;
    std::vector<std::unique_ptr<BrainRegionImpl>> regions_;
    std::unordered_map<int, size_t> region_index_;

    bool initialized_;
};

} // namespace flybrain

#endif // CONNECTOME_HPP
