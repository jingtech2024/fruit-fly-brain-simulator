#include "connectome.hpp"
#include "simulator.hpp"
#include "io_utils.hpp"
#include "visualization.hpp"
#include <iostream>
#include <cstring>

using namespace flybrain;

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " [options]\n"
              << "\nOptions:\n"
              << "  -h, --help              Show this help message\n"
              << "  -n, --neurons <N>       Number of neurons (default: 1000)\n"
              << "  -t, --time <ms>         Simulation duration in ms (default: 1000)\n"
              << "  -d, --dt <ms>           Time step in ms (default: 0.1)\n"
              << "  -s, --seed <N>          Random seed (default: 42)\n"
              << "  -i, --input <file>      Load connectome from file\n"
              << "  -o, --output <file>     Save results to file\n"
              << "  --synthetic             Generate synthetic connectome\n"
              << "  --no-parallel           Disable parallel execution\n"
              << "  --viz <file>            Generate 3D visualization HTML file\n"
              << "  --export <file>         Export connectome (CSV format)\n"
              << "\nExample:\n"
              << "  " << program << " --neurons 10000 --time 5000 --synthetic --viz brain.html\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    // Default parameters
    size_t num_neurons = 1000;
    double duration = 1000.0;  // ms
    double dt = 0.1;           // ms
    uint32_t seed = 42;
    std::string input_file;
    std::string output_file;
    std::string viz_file;
    std::string export_file;
    bool use_synthetic = false;
    bool use_parallel = true;

    // Parse command line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        }
        else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--neurons") == 0) {
            if (i + 1 < argc) {
                num_neurons = std::stoul(argv[++i]);
            }
        }
        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--time") == 0) {
            if (i + 1 < argc) {
                duration = std::stod(argv[++i]);
            }
        }
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dt") == 0) {
            if (i + 1 < argc) {
                dt = std::stod(argv[++i]);
            }
        }
        else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--seed") == 0) {
            if (i + 1 < argc) {
                seed = std::stoul(argv[++i]);
            }
        }
        else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
            if (i + 1 < argc) {
                input_file = argv[++i];
            }
        }
        else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            }
        }
        else if (strcmp(argv[i], "--synthetic") == 0) {
            use_synthetic = true;
        }
        else if (strcmp(argv[i], "--no-parallel") == 0) {
            use_parallel = false;
        }
        else if (strcmp(argv[i], "--viz") == 0) {
            if (i + 1 < argc) {
                viz_file = argv[++i];
            }
        }
        else if (strcmp(argv[i], "--export") == 0) {
            if (i + 1 < argc) {
                export_file = argv[++i];
            }
        }
    }

    std::cout << "=== Fruit Fly Brain Simulator ===" << std::endl;
    std::cout << "Neurons: " << num_neurons << std::endl;
    std::cout << "Duration: " << duration << " ms" << std::endl;
    std::cout << "Time step: " << dt << " ms" << std::endl;
    std::cout << "Seed: " << seed << std::endl;
    std::cout << "Parallel: " << (use_parallel ? "yes" : "no") << std::endl;
    std::cout << std::endl;

    // Create connectome
    Connectome connectome;

    if (!input_file.empty()) {
        std::cout << "Loading connectome from: " << input_file << std::endl;
        // TODO: Implement file loading
        // connectome.loadFromFile(input_file);
    } else if (use_synthetic) {
        std::cout << "Generating synthetic connectome..." << std::endl;

        // Use realistic connection probability for fruit fly
        // ~140k neurons, ~50M synapses -> p ~ 0.0003
        double connection_prob = 0.0003;
        if (num_neurons < 10000) {
            // Scale up for smaller networks to maintain activity
            connection_prob = 0.01;
        }

        connectome.generateSynthetic(num_neurons, connection_prob, true);
    } else {
        std::cout << "Creating small test connectome..." << std::endl;
        connectome.initialize(num_neurons);

        // Create some random connections for testing
        std::mt19937 rng(seed);
        std::uniform_int_distribution<> neuron_dist(0, num_neurons - 1);
        std::uniform_real_distribution<> prob_dist(0.0, 1.0);

        size_t num_connections = num_neurons * 10;  // 10 connections per neuron on average
        for (size_t i = 0; i < num_connections; ++i) {
            uint32_t pre = static_cast<uint32_t>(neuron_dist(rng));
            uint32_t post = static_cast<uint32_t>(neuron_dist(rng));
            if (pre != post) {
                connectome.connectivity().addConnection(pre, post);
            }
        }
    }

    // Print connectome statistics
    connectome.printStats();
    std::cout << std::endl;

    // Configure simulation
    SimulationConfig config;
    config.dt = dt;
    config.duration = duration;
    config.record_spikes = true;
    config.record_voltage = false;
    config.report_interval = 100;
    config.use_parallel = use_parallel;

    // Create simulator
    Simulator simulator(connectome);
    // 局部感觉输入驱动模式
    if (num_neurons > 10) {
        // 主刺激：0.5% 的神经元接受近阈值电流驱动 (种子)
        Stimulus stimulus;
        stimulus.type = StimulusType::STEP;
        stimulus.amplitude = 3.1;  // nA — 刚超阈值(I_th=3nA)
        stimulus.start_time = 5.0;
        stimulus.end_time = duration - 5.0;

        size_t num_stimulated = std::max(size_t(1), num_neurons / 200);
        for (size_t i = 0; i < num_stimulated; ++i) {
            stimulus.target_neurons.push_back(static_cast<uint32_t>(i));
        }
        simulator.addStimulus(stimulus);

        // 全局背景噪声
        Stimulus noise;
        noise.type = StimulusType::NOISE;
        noise.amplitude = 2.0;  // nA
        noise.start_time = 0.0;
        noise.end_time = duration;

        for (size_t i = 0; i < num_neurons; ++i) {
            noise.target_neurons.push_back(static_cast<uint32_t>(i));
        }
        simulator.addStimulus(noise);
        
        std::cout << "Added stimulus to " << num_stimulated << " neurons + global noise" << std::endl;
    }
    std::cout << std::endl;

    // Run simulation
    std::cout << "Starting simulation..." << std::endl;
    if (simulator.run()) {
        auto stats = simulator.getStats();
        std::cout << "\n=== Simulation Results ===" << std::endl;
        std::cout << "Total spikes: " << stats.total_spikes << std::endl;
        std::cout << "Steps per second: " << stats.steps_per_second << std::endl;
        std::cout << "Speedup: " << stats.speedup << "x" << std::endl;

        if (!output_file.empty()) {
            std::cout << "\nSaving results to: " << output_file << std::endl;
            const auto& spikes = simulator.getSpikes();
            io::CSVWriter writer(output_file);
            if (writer.open()) {
                writer.writeRow(std::vector<std::string>{"neuron_id", "time_ms"});
                for (const auto& spike : spikes) {
                    writer.writeRow(std::vector<std::string>{
                        std::to_string(spike.neuron_id),
                        std::to_string(spike.time)
                    });
                }
                writer.close();
                std::cout << "Saved " << spikes.size() << " spike records" << std::endl;
            }
        }
    } else {
        std::cerr << "Simulation failed!" << std::endl;
        return 1;
    }

    // Generate visualization if requested
    if (!viz_file.empty()) {
        std::cout << "\nGenerating 3D visualization..." << std::endl;
        viz::BrainLayout3D layout;
        layout.generatePositions(connectome);

        // Process spikes for animation
        std::vector<viz::ActivityFrame> frames;
        if (!simulator.getSpikes().empty()) {
            frames = viz::ActivityVisualizer::processSpikes(simulator.getSpikes(), 10.0);
        }

        if (viz::HTMLVisualizer::generateHTML(viz_file, layout, frames)) {
            std::cout << "Generated 3D visualization: " << viz_file << std::endl;
        } else {
            std::cerr << "Failed to generate visualization" << std::endl;
        }
    }

    // Export connectome if requested
    if (!export_file.empty()) {
        std::cout << "\nExporting connectome..." << std::endl;
        viz::BrainLayout3D layout;
        layout.generatePositions(connectome);
        if (viz::DataExporter::exportToCSV(connectome, layout, export_file)) {
            std::cout << "Exported to: " << export_file << std::endl;
        }
    }

    std::cout << "\nDone!" << std::endl;
    return 0;
}
