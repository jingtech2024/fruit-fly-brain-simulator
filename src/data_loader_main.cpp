#include "connectome.hpp"
#include "io_utils.hpp"
#include <iostream>
#include <cstring>

using namespace flybrain;

void printUsage(const char* program) {
    std::cout << "Usage: " << program << " <command> [options]\n"
              << "\nCommands:\n"
              << "  convert <input> <output>   Convert between formats\n"
              << "  stats <file>               Show connectome statistics\n"
              << "  generate <n> <output>      Generate synthetic connectome\n"
              << "  extract <file> <region>    Extract neurons from region\n"
              << "\nOptions:\n"
              << "  -f, --format <fmt>         Input format (csv, hdf5, neuprint)\n"
              << "  -n, --neurons <N>          Number of neurons for generation\n"
              << "  -p, --probability <p>      Connection probability\n"
              << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];

    if (command == "generate" && argc >= 4) {
        size_t num_neurons = std::stoul(argv[2]);
        std::string output_file = argv[3];

        double probability = 0.0003;  // Default for fruit fly
        if (argc > 4) {
            probability = std::stod(argv[4]);
        }

        std::cout << "Generating synthetic connectome..." << std::endl;
        std::cout << "  Neurons: " << num_neurons << std::endl;
        std::cout << "  Connection probability: " << probability << std::endl;

        Connectome connectome;
        connectome.generateSynthetic(num_neurons, probability, true);

        // Save to CSV
        io::CSVWriter writer(output_file);
        if (writer.open()) {
            writer.writeRow(std::vector<std::string>{"pre", "post", "type", "contacts"});

            for (const auto& synapse : connectome.connectivity().synapses()) {
                writer.writeRow(std::vector<std::string>{
                    std::to_string(synapse.preId()),
                    std::to_string(synapse.postId()),
                    std::to_string(static_cast<int>(synapse.type())),
                    std::to_string(synapse.numContacts())
                });
            }
            writer.close();

            std::cout << "Saved to: " << output_file << std::endl;
            connectome.printStats();
        } else {
            std::cerr << "Failed to open output file: " << output_file << std::endl;
            return 1;
        }

    } else if (command == "stats" && argc >= 3) {
        std::string input_file = argv[2];

        std::cout << "Loading connectome from: " << input_file << std::endl;

        // Try to load and show stats
        io::CSVReader reader(input_file);
        if (!reader.open()) {
            std::cerr << "Failed to open file: " << input_file << std::endl;
            return 1;
        }

        size_t edge_count = 0;
        size_t neuron_set = 0;
        std::vector<bool> neuron_seen(200000, false);  // Max 200k neurons

        // Skip header
        auto header = reader.nextRow();

        while (reader.hasNext()) {
            auto row = reader.nextRow();
            if (row.size() >= 2) {
                uint32_t pre = std::stoul(row[0]);
                uint32_t post = std::stoul(row[1]);

                if (!neuron_seen[pre]) {
                    neuron_seen[pre] = true;
                    neuron_set++;
                }
                if (!neuron_seen[post]) {
                    neuron_seen[post] = true;
                    neuron_set++;
                }

                edge_count++;
            }
        }

        std::cout << "\n=== Connectome Statistics ===" << std::endl;
        std::cout << "Neurons: " << neuron_set << std::endl;
        std::cout << "Synapses: " << edge_count << std::endl;
        std::cout << "Mean degree: " << (double)edge_count / neuron_set << std::endl;

    } else if (command == "convert" && argc >= 4) {
        std::string input_file = argv[2];
        std::string output_file = argv[3];

        std::cout << "Converting " << input_file << " -> " << output_file << std::endl;
        // TODO: Implement format conversion

    } else if (command == "-h" || command == "--help") {
        printUsage(argv[0]);
        return 0;

    } else {
        std::cerr << "Unknown command or missing arguments" << std::endl;
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}
