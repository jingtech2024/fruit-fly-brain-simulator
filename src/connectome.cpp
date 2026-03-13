#include "connectome.hpp"
#include <iostream>
#include <algorithm>
#include <random>

namespace flybrain {

ConnectivityMatrix::ConnectivityMatrix(size_t num_neurons)
    : num_neurons_(num_neurons)
    , outgoing_(num_neurons)
    , incoming_(num_neurons)
    , next_synapse_id_(0)
{
}

uint32_t ConnectivityMatrix::addConnection(uint32_t pre_neuron, uint32_t post_neuron,
                                           SynapseType type, uint32_t num_contacts) {
    if (pre_neuron >= num_neurons_ || post_neuron >= num_neurons_) {
        return UINT32_MAX;  // Invalid
    }

    // Check for existing connection
    uint64_t key = (static_cast<uint64_t>(pre_neuron) << 32) | post_neuron;
    auto it = connection_map_.find(key);
    if (it != connection_map_.end()) {
        // Update existing synapse
        synapses_[it->second].setWeight(
            synapses_[it->second].weight() + 1.0 * num_contacts);
        return it->second;
    }

    // Create new synapse
    uint32_t synapse_id = next_synapse_id_++;
    synapses_.emplace_back(synapse_id, pre_neuron, post_neuron, type, num_contacts);

    // Update adjacency lists
    outgoing_[pre_neuron].push_back(synapse_id);
    incoming_[post_neuron].push_back(synapse_id);

    // Update connection map
    connection_map_[key] = synapse_id;

    return synapse_id;
}

const std::vector<uint32_t>& ConnectivityMatrix::getOutgoing(uint32_t neuron_id) const {
    static std::vector<uint32_t> empty;
    if (neuron_id >= num_neurons_) return empty;
    return outgoing_[neuron_id];
}

const std::vector<uint32_t>& ConnectivityMatrix::getIncoming(uint32_t neuron_id) const {
    static std::vector<uint32_t> empty;
    if (neuron_id >= num_neurons_) return empty;
    return incoming_[neuron_id];
}

bool ConnectivityMatrix::hasConnection(uint32_t pre, uint32_t post) const {
    uint64_t key = (static_cast<uint64_t>(pre) << 32) | post;
    return connection_map_.find(key) != connection_map_.end();
}

Synapse& ConnectivityMatrix::getSynapse(uint32_t synapse_id) {
    return synapses_[synapse_id];
}

const Synapse& ConnectivityMatrix::getSynapse(uint32_t synapse_id) const {
    return synapses_[synapse_id];
}

std::vector<Synapse>& ConnectivityMatrix::synapses() {
    return synapses_;
}

const std::vector<Synapse>& ConnectivityMatrix::synapses() const {
    return synapses_;
}

void ConnectivityMatrix::buildIndex() {
    // Already indexed via adjacency lists
    // This method could add additional spatial indexing if needed
}

// Connectome implementation
Connectome::Connectome()
    : connectivity_(0)
    , initialized_(false)
{
}

Connectome::~Connectome() = default;

void Connectome::initialize(size_t num_neurons) {
    neurons_.resize(num_neurons);
    connectivity_ = ConnectivityMatrix(num_neurons);

    // Initialize neurons with IDs
    for (size_t i = 0; i < num_neurons; ++i) {
        neurons_[i] = Neuron(static_cast<uint32_t>(i));
    }

    // Create brain regions
    regions_.clear();
    region_index_.clear();

    auto add_region = [this](BrainRegion type, const std::string& name) {
        region_index_[static_cast<int>(type)] = regions_.size();
        regions_.push_back(std::make_unique<BrainRegionImpl>(type, name));
    };

    add_region(BrainRegion::OPTIC_LOBE, "Optic Lobe");
    add_region(BrainRegion::ANTENNAL_LOBE, "Antennal Lobe");
    add_region(BrainRegion::MUSHROOM_BODY, "Mushroom Body");
    add_region(BrainRegion::CENTRAL_COMPLEX, "Central Complex");
    add_region(BrainRegion::LATERAL_HORN, "Lateral Horn");
    add_region(BrainRegion::SUBESOPHAGEAL_ZONE, "Subesophageal Zone");
    add_region(BrainRegion::VENTRAL_NERVE_CORD, "Ventral Nerve Cord");

    initialized_ = true;
}

bool Connectome::loadFromFile(const std::string& /*filename*/, const std::string& format) {
    // Placeholder - actual implementation in data_loader
    std::cout << "Loading from file: " << /*filename << " (" << */ format << ")" << std::endl;
    return false;
}

bool Connectome::loadFromNeuPrint(const std::string& /*filename*/) {
    // Placeholder for neuPrint format
    return false;
}

void Connectome::generateSynthetic(size_t num_neurons,
                                   double connection_probability,
                                   bool use_distance_decay) {
    if (!initialized_) {
        initialize(num_neurons);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> prob(0.0, 1.0);

    // 按 80/20 比例随机分配兴奋/抑制类型（符合生物学中的 Dale 法则）
    for (size_t i = 0; i < num_neurons; ++i) {
        NeuronType ntype = (prob(gen) < 0.8) ? NeuronType::EXCITATORY : NeuronType::INHIBITORY;
        neurons_[i] = Neuron(static_cast<uint32_t>(i), ntype, neurons_[i].region());
    }

    // Assign neurons to regions (simplified distribution)
    size_t optic_lobe_size = num_neurons * 35 / 100;      // ~35%
    size_t antennal_lobe_size = num_neurons * 5 / 100;     // ~5%
    size_t mushroom_body_size = num_neurons * 10 / 100;    // ~10%
    size_t central_complex_size = num_neurons * 5 / 100;   // ~5%
    size_t lateral_horn_size = num_neurons * 5 / 100;      // ~5%
    size_t sez_size = num_neurons * 10 / 100;              // ~10%
    size_t vnc_size = num_neurons * 30 / 100;              // ~30%

    size_t idx = 0;
    for (size_t i = 0; i < optic_lobe_size; ++i, ++idx)
        assignToRegion(static_cast<uint32_t>(idx), BrainRegion::OPTIC_LOBE);
    for (size_t i = 0; i < antennal_lobe_size; ++i, ++idx)
        assignToRegion(static_cast<uint32_t>(idx), BrainRegion::ANTENNAL_LOBE);
    for (size_t i = 0; i < mushroom_body_size; ++i, ++idx)
        assignToRegion(static_cast<uint32_t>(idx), BrainRegion::MUSHROOM_BODY);
    for (size_t i = 0; i < central_complex_size; ++i, ++idx)
        assignToRegion(static_cast<uint32_t>(idx), BrainRegion::CENTRAL_COMPLEX);
    for (size_t i = 0; i < lateral_horn_size; ++i, ++idx)
        assignToRegion(static_cast<uint32_t>(idx), BrainRegion::LATERAL_HORN);
    for (size_t i = 0; i < sez_size; ++i, ++idx)
        assignToRegion(static_cast<uint32_t>(idx), BrainRegion::SUBESOPHAGEAL_ZONE);
    for (size_t i = 0; i < vnc_size && idx < num_neurons; ++i, ++idx)
        assignToRegion(static_cast<uint32_t>(idx), BrainRegion::VENTRAL_NERVE_CORD);

    // Generate connections with distance-dependent probability
    // 恢复标准平衡网络参数：适当的兴奋权重配合强抑制
    std::uniform_real_distribution<> weight_dist(0.01, 0.05);
    for (size_t pre = 0; pre < num_neurons; ++pre) {
        for (size_t post = 0; post < num_neurons; ++post) {
            if (pre == post) continue;  // No self-connections

            double effective_prob = connection_probability;

            if (use_distance_decay) {
                // Simplified: neurons in same region more likely to connect
                auto pre_region = neurons_[pre].region();
                auto post_region = neurons_[post].region();
                if (pre_region == post_region && pre_region != BrainRegion::UNKNOWN) {
                    effective_prob *= 10.0;  // 10x more likely within region
                }
            }

            if (prob(gen) < effective_prob) {
                // Determine synapse type (mostly chemical)
                SynapseType type = SynapseType::CHEMICAL;
                if (prob(gen) < 0.05) {  // ~5% electrical
                    type = SynapseType::ELECTRICAL;
                }

                // Number of synaptic contacts (1-5)
                std::uniform_int_distribution<> contacts(1, 5);
                uint32_t num_contacts = static_cast<uint32_t>(contacts(gen));

                uint32_t syn_id = connectivity_.addConnection(
                    static_cast<uint32_t>(pre),
                    static_cast<uint32_t>(post),
                    type, num_contacts);

                // 根据突触前神经元类型设置突触属性
                if (syn_id != UINT32_MAX && type == SynapseType::CHEMICAL) {
                    auto& syn = connectivity_.getSynapse(syn_id);
                    double w = weight_dist(gen);

                    if (neurons_[pre].type() == NeuronType::INHIBITORY) {
                        // 抑制性突触：E_rev=-80mV，权重 8× 兴奋性（真实平衡计算：4(数量比) × 1.67(驱动力比) ≈ 6.7）
                        // 设为 8.0 以提供净抑制保证网络的稳定异步状态
                        syn.setWeight(w * 8.0);
                        syn.setReversalPotential(-80.0);
                        syn.setDelay(1.5);  // 标准延迟
                    } else {
                        // 兴奋性突触：E_rev=0mV
                        syn.setWeight(w);
                        syn.setReversalPotential(0.0);
                        syn.setDelay(1.5);  // 标准延迟
                    }
                }
            }
        }
    }
}

Neuron& Connectome::getNeuron(uint32_t id) {
    return neurons_[id];
}

const Neuron& Connectome::getNeuron(uint32_t id) const {
    return neurons_[id];
}

std::vector<Neuron>& Connectome::neurons() {
    return neurons_;
}

const std::vector<Neuron>& Connectome::neurons() const {
    return neurons_;
}

ConnectivityMatrix& Connectome::connectivity() {
    return connectivity_;
}

const ConnectivityMatrix& Connectome::connectivity() const {
    return connectivity_;
}

std::vector<std::unique_ptr<BrainRegionImpl>>& Connectome::regions() {
    return regions_;
}

void Connectome::assignToRegion(uint32_t neuron_id, BrainRegion region_type) {
    if (neuron_id >= neurons_.size()) return;

    neurons_[neuron_id] = Neuron(neuron_id, neurons_[neuron_id].type(), region_type);

    auto it = region_index_.find(static_cast<int>(region_type));
    if (it != region_index_.end() && it->second < regions_.size()) {
        regions_[it->second]->addNeuron(neuron_id);
    }
}

ConnectomeStats Connectome::getStats() const {
    ConnectomeStats stats{};
    stats.total_neurons = neurons_.size();
    stats.total_synapses = connectivity_.numConnections();
    stats.total_edges = stats.total_synapses;  // Simplified

    // Count neuron types
    stats.excitatory_neurons = 0;
    stats.inhibitory_neurons = 0;
    for (const auto& neuron : neurons_) {
        if (neuron.type() == NeuronType::EXCITATORY)
            stats.excitatory_neurons++;
        else if (neuron.type() == NeuronType::INHIBITORY)
            stats.inhibitory_neurons++;
    }

    // Count synapse types
    stats.chemical_synapses = 0;
    stats.electrical_synapses = 0;
    for (const auto& synapse : connectivity_.synapses()) {
        if (synapse.type() == SynapseType::CHEMICAL)
            stats.chemical_synapses++;
        else if (synapse.type() == SynapseType::ELECTRICAL)
            stats.electrical_synapses++;
    }

    // Compute mean degrees
    if (stats.total_neurons > 0) {
        stats.mean_degree = static_cast<double>(stats.total_edges) / stats.total_neurons;
        stats.mean_in_degree = stats.mean_degree;
        stats.mean_out_degree = stats.mean_degree;
    }

    return stats;
}

bool Connectome::saveToFile(const std::string& /*filename*/, const std::string& /*format*/) {
    // Placeholder
    return false;
}

void Connectome::printStats() const {
    auto stats = getStats();
    std::cout << "=== Connectome Statistics ===" << std::endl;
    std::cout << "Total neurons: " << stats.total_neurons << std::endl;
    std::cout << "Total synapses: " << stats.total_synapses << std::endl;
    std::cout << "Excitatory neurons: " << stats.excitatory_neurons << std::endl;
    std::cout << "Inhibitory neurons: " << stats.inhibitory_neurons << std::endl;
    std::cout << "Chemical synapses: " << stats.chemical_synapses << std::endl;
    std::cout << "Electrical synapses: " << stats.electrical_synapses << std::endl;
    std::cout << "Mean degree: " << stats.mean_degree << std::endl;
}

} // namespace flybrain
