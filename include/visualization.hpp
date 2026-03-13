#ifndef VISUALIZATION_HPP
#define VISUALIZATION_HPP

#include "connectome.hpp"
#include "simulator.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace flybrain {
namespace viz {

/**
 * @brief 3D position for neuron visualization
 */
struct NeuronPosition {
    uint32_t neuron_id;
    double x, y, z;  // 3D coordinates in micrometers
};

/**
 * @brief Color scheme for brain regions
 */
struct RegionColor {
    double r, g, b;
    const char* name;
};

/**
 * @brief Generate 3D positions for neurons based on brain region
 *
 * Creates a realistic 3D layout of the fruit fly brain
 */
class BrainLayout3D {
public:
    BrainLayout3D();

    /**
     * @brief Generate positions for all neurons
     * @param connectome The connectome to layout
     */
    void generatePositions(const Connectome& connectome);

    /**
     * @brief Get position for a specific neuron
     */
    const NeuronPosition& getPosition(uint32_t neuron_id) const;

    /**
     * @brief Get all positions
     */
    const std::vector<NeuronPosition>& getPositions() const { return positions_; }

    /**
     * @brief Get color for a brain region
     */
    static RegionColor getRegionColor(BrainRegion region);

    /**
     * @brief Export positions to JSON
     */
    bool exportToJSON(const std::string& filename) const;

private:
    std::vector<NeuronPosition> positions_;
    std::vector<std::vector<NeuronPosition>> region_positions_;

    // Generate positions for specific brain regions
    void generateOpticLobe(size_t start_id, size_t count);
    void generateAntennalLobe(size_t start_id, size_t count);
    void generateMushroomBody(size_t start_id, size_t count);
    void generateCentralComplex(size_t start_id, size_t count);
    void generateLateralHorn(size_t start_id, size_t count);
    void generateSubesophagealZone(size_t start_id, size_t count);
    void generateVentralNerveCord(size_t start_id, size_t count);
};

/**
 * @brief Spike activity visualization data
 */
struct ActivityFrame {
    double time_ms;
    std::vector<uint32_t> active_neurons;
    std::vector<double> intensities;
};

/**
 * @brief Generate visualization data for simulation
 */
class ActivityVisualizer {
public:
    /**
     * @brief Process spike records into animation frames
     */
    static std::vector<ActivityFrame> processSpikes(
        const std::vector<SpikeRecord>& spikes,
        double frame_duration_ms = 10.0);

    /**
     * @brief Export activity data to JSON
     */
    static bool exportToJSON(const std::string& filename,
                            const std::vector<ActivityFrame>& frames);
};

/**
 * @brief Generate complete HTML visualization
 */
class HTMLVisualizer {
public:
    /**
     * @brief Generate standalone HTML file with 3D visualization
     * @param output_file Output HTML file path
     * @param positions Neuron 3D positions
     * @param frames Activity animation frames (optional)
     */
    static bool generateHTML(const std::string& output_file,
                            const BrainLayout3D& positions,
                            const std::vector<ActivityFrame>& frames = {});

private:
    static std::string getHTMLTemplate();
    static std::string getThreeJSScene(const BrainLayout3D& positions);
    static std::string getAnimationScript(const std::vector<ActivityFrame>& frames);
};

/**
 * @brief Export simulation data for external visualization tools
 */
class DataExporter {
public:
    /**
     * @brief Export to NeuroML format
     */
    static bool exportToNeuroML(const Connectome& connectome,
                               const BrainLayout3D& layout,
                               const std::string& filename);

    /**
     * @brief Export to simple CSV format
     */
    static bool exportToCSV(const Connectome& connectome,
                           const BrainLayout3D& layout,
                           const std::string& filename);

    /**
     * @brief Export to binary format for fast loading
     */
    static bool exportToBinary(const Connectome& connectome,
                              const BrainLayout3D& layout,
                              const std::string& filename);
};

} // namespace viz
} // namespace flybrain

#endif // VISUALIZATION_HPP
