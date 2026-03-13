#include "connectome.hpp"
#include "simulator.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

using namespace flybrain;

bool test_neuron() {
    std::cout << "Testing Neuron... ";

    Neuron neuron(0, NeuronType::EXCITATORY, BrainRegion::CENTRAL_COMPLEX);

    // Test initial state
    assert(neuron.membranePotential() == -65.0);
    assert(!neuron.hasSpiked());
    assert(!neuron.isRefractory());

    // Test update with strong current
    neuron.update(0.1, 10.0);  // Strong depolarizing current

    // Test reset
    neuron.reset();
    assert(neuron.membranePotential() == -65.0);

    std::cout << "PASSED" << std::endl;
    return true;
}

bool test_synapse() {
    std::cout << "Testing Synapse... ";

    Synapse synapse(0, 0, 1, SynapseType::CHEMICAL, 1);
    synapse.setWeight(2.0);
    synapse.setDelay(1.5);

    assert(synapse.preId() == 0);
    assert(synapse.postId() == 1);
    assert(synapse.weight() == 2.0);

    // Test pre-synaptic spike
    synapse.onPreSpike(0.0);
    assert(synapse.isActive());

    // Test update
    double current = synapse.update(0.1, -65.0);

    std::cout << "PASSED" << std::endl;
    return true;
}

bool test_connectome() {
    std::cout << "Testing Connectome... ";

    Connectome connectome;
    connectome.initialize(100);

    assert(connectome.numNeurons() == 100);

    // Add some connections
    auto& conn = connectome.connectivity();
    conn.addConnection(0, 1);
    conn.addConnection(1, 2);
    conn.addConnection(2, 0);

    assert(conn.numConnections() == 3);
    assert(conn.hasConnection(0, 1));
    assert(!conn.hasConnection(0, 2));

    // Test stats
    auto stats = connectome.getStats();
    assert(stats.total_neurons == 100);
    assert(stats.total_synapses == 3);

    std::cout << "PASSED" << std::endl;
    return true;
}

bool test_synthetic_connectome() {
    std::cout << "Testing Synthetic Connectome... ";

    Connectome connectome;
    connectome.generateSynthetic(1000, 0.01, true);

    auto stats = connectome.getStats();

    // Should have some connections
    assert(stats.total_synapses > 0);

    std::cout << "PASSED (" << stats.total_synapses << " synapses)" << std::endl;
    return true;
}

bool test_simulator() {
    std::cout << "Testing Simulator... ";

    Connectome connectome;
    connectome.initialize(50);

    // Add some connections
    for (size_t i = 0; i < 49; ++i) {
        connectome.connectivity().addConnection(static_cast<uint32_t>(i),
                                                static_cast<uint32_t>(i + 1));
    }

    SimulationConfig config;
    config.dt = 0.1;
    config.duration = 50.0;
    config.report_interval = 0;  // Suppress output

    Simulator simulator(connectome);
    simulator.setConfig(config);

    // Add stimulus
    Stimulus stimulus;
    stimulus.type = StimulusType::STEP;
    stimulus.amplitude = 10.0;
    stimulus.start_time = 0.0;
    stimulus.end_time = 50.0;
    stimulus.target_neurons.push_back(0);

    simulator.addStimulus(stimulus);

    bool result = simulator.run();

    assert(result);
    assert(simulator.getStats().total_spikes >= 0);

    std::cout << "PASSED (" << simulator.getStats().total_spikes << " spikes)" << std::endl;
    return true;
}

bool test_brain_region() {
    std::cout << "Testing BrainRegion... ";

    BrainRegionImpl region(flybrain::BrainRegion::OPTIC_LOBE, "Optic Lobe");

    region.addNeuron(0);
    region.addNeuron(1);
    region.addNeuron(2);

    assert(region.getNeuronCount() == 3);
    assert(region.getNeurons().size() == 3);

    auto stats = region.getStats();
    assert(stats.total_neurons == 3);

    std::cout << "PASSED" << std::endl;
    return true;
}

int main() {
    std::cout << "=== Running Unit Tests ===" << std::endl << std::endl;

    int passed = 0;
    int total = 0;

    total++; if (test_neuron()) passed++;
    total++; if (test_synapse()) passed++;
    total++; if (test_brain_region()) passed++;
    total++; if (test_connectome()) passed++;
    total++; if (test_synthetic_connectome()) passed++;
    total++; if (test_simulator()) passed++;

    std::cout << std::endl;
    std::cout << "=== Results: " << passed << "/" << total << " tests passed ===" << std::endl;

    return (passed == total) ? 0 : 1;
}
