#include "visualization.hpp"
#include <cmath>
#include <random>
#include <fstream>
#include <iomanip>

namespace flybrain {
namespace viz {

static const RegionColor REGION_COLORS[] = {
    {0.8, 0.2, 0.2, "Optic Lobe"},
    {0.2, 0.8, 0.2, "Antennal Lobe"},
    {0.2, 0.2, 0.8, "Mushroom Body"},
    {0.8, 0.8, 0.2, "Central Complex"},
    {0.8, 0.2, 0.8, "Lateral Horn"},
    {0.2, 0.8, 0.8, "Subesophageal Zone"},
    {0.8, 0.5, 0.2, "Ventral Nerve Cord"},
    {0.5, 0.5, 0.5, "Unknown"}
};

BrainLayout3D::BrainLayout3D() : region_positions_(8) {}

void BrainLayout3D::generatePositions(const Connectome& connectome) {
    positions_.clear();
    positions_.resize(connectome.numNeurons());

    std::vector<std::vector<uint32_t>> neurons_by_region(8);
    for (size_t i = 0; i < connectome.numNeurons(); ++i) {
        const auto& neuron = connectome.getNeuron(static_cast<uint32_t>(i));
        int region_idx = static_cast<int>(neuron.region());
        if (region_idx < 0 || region_idx >= 8) region_idx = 0;
        neurons_by_region[region_idx].push_back(static_cast<uint32_t>(i));
    }

    size_t offset = 0;
    generateOpticLobe(offset, neurons_by_region[1].size());
    offset += neurons_by_region[1].size();
    generateAntennalLobe(offset, neurons_by_region[2].size());
    offset += neurons_by_region[2].size();
    generateMushroomBody(offset, neurons_by_region[3].size());
    offset += neurons_by_region[3].size();
    generateCentralComplex(offset, neurons_by_region[4].size());
    offset += neurons_by_region[4].size();
    generateLateralHorn(offset, neurons_by_region[5].size());
    offset += neurons_by_region[5].size();
    generateSubesophagealZone(offset, neurons_by_region[6].size());
    offset += neurons_by_region[6].size();
    generateVentralNerveCord(offset, neurons_by_region[7].size());
}

void BrainLayout3D::generateOpticLobe(size_t start_id, size_t count) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<> layer_dist(0.0, 1.0);
    std::uniform_real_distribution<> angle_dist(0.0, 2.0 * M_PI);
    for (size_t i = 0; i < count; ++i) {
        uint32_t id = start_id + static_cast<uint32_t>(i);
        double layer = layer_dist(gen);
        double base_x = -150.0 - layer * 50.0;
        double radius = 30.0 + layer * 20.0;
        double angle = angle_dist(gen);
        positions_[id] = {id, base_x + layer_dist(gen) * 20.0,
                         radius * std::cos(angle) * 0.5, radius * std::sin(angle)};
    }
}

void BrainLayout3D::generateAntennalLobe(size_t start_id, size_t count) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dist(0.0, 1.0);
    for (size_t i = 0; i < count; ++i) {
        uint32_t id = start_id + static_cast<uint32_t>(i);
        double theta = dist(gen) * 2.0 * M_PI;
        double phi = std::acos(2.0 * dist(gen) - 1.0);
        double r = 20.0 * std::pow(dist(gen), 1.0/3.0);
        positions_[id] = {id, r * std::sin(phi) * std::cos(theta),
                         r * std::sin(phi) * std::sin(theta) + 50.0, r * std::cos(phi)};
    }
}

void BrainLayout3D::generateMushroomBody(size_t start_id, size_t count) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dist(0.0, 1.0);
    for (size_t i = 0; i < count; ++i) {
        uint32_t id = start_id + static_cast<uint32_t>(i);
        double comp = dist(gen);
        if (comp < 0.3) {
            double theta = dist(gen) * 2.0 * M_PI;
            double phi = dist(gen) * M_PI / 2.0;
            double r = 25.0 + dist(gen) * 15.0;
            positions_[id] = {id, r * std::sin(phi) * std::cos(theta),
                             r * std::sin(phi) * std::sin(theta) - 30.0, r * std::cos(phi)};
        } else if (comp < 0.6) {
            positions_[id] = {id, (dist(gen)-0.5)*10.0, -30.0-dist(gen)*40.0, (dist(gen)-0.5)*20.0};
        } else {
            double lobe = dist(gen);
            double lx = (dist(gen)-0.5)*60.0;
            if (lobe < 0.33) positions_[id] = {id, lx, -70.0, 30.0+dist(gen)*30.0};
            else if (lobe < 0.66) positions_[id] = {id, lx, -70.0-dist(gen)*30.0, 10.0};
            else positions_[id] = {id, lx, -70.0, -20.0-dist(gen)*20.0};
        }
    }
}

void BrainLayout3D::generateCentralComplex(size_t start_id, size_t count) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dist(0.0, 1.0);
    for (size_t i = 0; i < count; ++i) {
        uint32_t id = start_id + static_cast<uint32_t>(i);
        double s = dist(gen);
        if (s < 0.4) {
            positions_[id] = {id, (dist(gen)-0.5)*40.0, -50.0-dist(gen)*20.0, (dist(gen)-0.5)*80.0};
        } else if (s < 0.7) {
            double angle = dist(gen)*2.0*M_PI, r = 30.0+dist(gen)*10.0;
            positions_[id] = {id, r*std::cos(angle), -40.0, r*std::sin(angle)};
        } else {
            positions_[id] = {id, (dist(gen)-0.5)*80.0, -30.0, (dist(gen)-0.5)*10.0};
        }
    }
}

void BrainLayout3D::generateLateralHorn(size_t start_id, size_t count) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dist(0.0, 1.0);
    for (size_t i = 0; i < count; ++i) {
        uint32_t id = static_cast<uint32_t>(start_id + i);
        positions_[id] = {id, -50.0-dist(gen)*40.0, dist(gen)*40.0-20.0, (dist(gen)-0.5)*60.0};
    }
}

void BrainLayout3D::generateSubesophagealZone(size_t start_id, size_t count) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dist(0.0, 1.0);
    for (size_t i = 0; i < count; ++i) {
        uint32_t id = static_cast<uint32_t>(start_id + i);
        positions_[id] = {id, (dist(gen)-0.5)*60.0, dist(gen)*30.0-50.0, (dist(gen)-0.5)*40.0};
    }
}

void BrainLayout3D::generateVentralNerveCord(size_t start_id, size_t count) {
    std::mt19937 gen(42);
    std::uniform_real_distribution<> dist(0.0, 1.0);
    for (size_t i = 0; i < count; ++i) {
        uint32_t id = static_cast<uint32_t>(start_id + i);
        double seg = dist(gen);
        positions_[id] = {id, (dist(gen)-0.5)*30.0, -60.0-seg*100.0, (dist(gen)-0.5)*20.0};
    }
}

const NeuronPosition& BrainLayout3D::getPosition(uint32_t neuron_id) const {
    return positions_[neuron_id];
}

RegionColor BrainLayout3D::getRegionColor(BrainRegion region) {
    int idx = static_cast<int>(region);
    if (idx < 0 || idx >= 8) idx = 7;
    return REGION_COLORS[idx];
}

bool BrainLayout3D::exportToJSON(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file) return false;
    file << "{\n  \"neurons\": [\n";
    for (size_t i = 0; i < positions_.size(); ++i) {
        RegionColor color = getRegionColor(static_cast<BrainRegion>(i % 8));
        const auto& pos = positions_[i];
        file << "  {\"id\":" << pos.neuron_id << ",\"x\":" << std::fixed << std::setprecision(2)
             << pos.x << ",\"y\":" << pos.y << ",\"z\":" << pos.z
             << ",\"r\":" << color.r << ",\"g\":" << color.g << ",\"b\":" << color.b << "}";
        if (i < positions_.size()-1) file << ",";
        file << "\n";
    }
    file << "  ]\n}\n";
    return file.good();
}

std::vector<ActivityFrame> ActivityVisualizer::processSpikes(
    const std::vector<SpikeRecord>& spikes, double frame_duration_ms) {
    std::vector<ActivityFrame> frames;
    if (spikes.empty()) return frames;
    double min_time = spikes.front().time, max_time = spikes.front().time;
    for (const auto& spike : spikes) {
        if (spike.time < min_time) min_time = spike.time;
        if (spike.time > max_time) max_time = spike.time;
    }
    for (double t = min_time; t <= max_time; t += frame_duration_ms) {
        ActivityFrame frame;
        frame.time_ms = t;
        for (const auto& spike : spikes) {
            if (spike.time >= t && spike.time < t + frame_duration_ms) {
                frame.active_neurons.push_back(spike.neuron_id);
                frame.intensities.push_back(1.0 - (spike.time - t) / frame_duration_ms);
            }
        }
        frames.push_back(frame);
    }
    return frames;
}

bool ActivityVisualizer::exportToJSON(const std::string& filename,
                                     const std::vector<ActivityFrame>& frames) {
    std::ofstream file(filename);
    if (!file) return false;
    file << "{\n  \"frames\": [\n";
    for (size_t i = 0; i < frames.size(); ++i) {
        const auto& frame = frames[i];
        file << "  {\"time\":" << std::fixed << std::setprecision(2) << frame.time_ms << ",\"neurons\":[";
        for (size_t j = 0; j < frame.active_neurons.size(); ++j) {
            if (j > 0) file << ",";
            file << frame.active_neurons[j];
        }
        file << "],\"intensities\":[";
        for (size_t j = 0; j < frame.intensities.size(); ++j) {
            if (j > 0) file << ",";
            file << std::setprecision(3) << frame.intensities[j];
        }
        file << "]}";
        if (i < frames.size()-1) file << ",";
        file << "\n";
    }
    file << "  ]\n}\n";
    return file.good();
}

bool HTMLVisualizer::generateHTML(const std::string& output_file,
                                  const BrainLayout3D& positions,
                                  const std::vector<ActivityFrame>& frames) {
    std::ofstream file(output_file);
    if (!file) return false;

    file << "<!DOCTYPE html>\n<html><head>\n";
    file << "<meta charset=\"UTF-8\"><title>Fruit Fly Brain 3D Activity</title>\n";
    file << "<style>\n"
         << "body{margin:0;background:#000;overflow:hidden;}\n"
         << "#ui{position:absolute;bottom:20px;left:20px;right:20px;color:#fff;font-family:Arial,sans-serif;z-index:10;display:flex;align-items:center;background:rgba(0,0,0,0.6);padding:10px;border-radius:8px;}\n"
         << "#playBtn{background:#4caf50;color:white;border:none;padding:8px 15px;border-radius:4px;cursor:pointer;margin-right:15px;font-weight:bold;}\n"
         << "#timeline{flex-grow:1;margin:0 15px;cursor:pointer;}\n"
         << "#timeLabel{min-width:70px;text-align:right;font-family:monospace;}\n"
         << "</style>\n";
    file << "</head><body>\n";
    file << "<div id=\"info\" style=\"position:absolute;top:10px;left:10px;color:#fff;font-family:Arial,sans-serif;z-index:10;\">";
    size_t num_neurons = positions.getPositions().size();
    file << "Fruit Fly Brain 3D - " << num_neurons << " neurons<br/>";
    file << "<span style=\"font-size:12px;color:#aaa;\">Left click: rotate | Right click: pan | Scroll: zoom</span></div>\n";
    
    // 注入 UI 控制条
    file << "<div id=\"ui\">\n"
         << "  <button id=\"playBtn\">PLAY</button>\n"
         << "  <input type=\"range\" id=\"timeline\" min=\"0\" max=\"1\" step=\"1\" value=\"0\">\n"
         << "  <div id=\"timeLabel\">0.0 ms</div>\n"
         << "</div>\n";

    // 使用 ES module import map 加载 Three.js r162 和官方 OrbitControls
    file << "<script type=\"importmap\">\n";
    file << "{\n";
    file << "  \"imports\": {\n";
    file << "    \"three\": \"https://unpkg.com/three@0.162.0/build/three.module.js\",\n";
    file << "    \"three/addons/\": \"https://unpkg.com/three@0.162.0/examples/jsm/\"\n";
    file << "  }\n";
    file << "}\n";
    file << "</script>\n";

    // 输出神经元数据为全局变量（供 ES module 脚本使用）
    const auto& poss = positions.getPositions();
    file << "<script>\n";
    file << "window.neuronData=[";
    for (size_t i = 0; i < poss.size(); ++i) {
        const auto& pos = poss[i];
        BrainRegion region = BrainRegion::OPTIC_LOBE;
        if (pos.x < -100) region = BrainRegion::OPTIC_LOBE;
        else if (pos.y > 40) region = BrainRegion::ANTENNAL_LOBE;
        else if (pos.y < -60) region = BrainRegion::VENTRAL_NERVE_CORD;
        else if (pos.z > 25 && pos.y < -25) region = BrainRegion::MUSHROOM_BODY;
        else if (std::abs(pos.z) < 15 && pos.y > -60 && pos.y < -25) region = BrainRegion::CENTRAL_COMPLEX;
        else if (pos.x < -40 && pos.x > -100) region = BrainRegion::LATERAL_HORN;
        else region = BrainRegion::SUBESOPHAGEAL_ZONE;

        RegionColor color = BrainLayout3D::getRegionColor(region);
        if (i > 0) file << ",";
        file << "{x:" << pos.x << ",y:" << pos.y << ",z:" << pos.z;
        file << ",r:" << color.r << ",g:" << color.g << ",b:" << color.b << "}";
    }
    file << "];\n";
    
    // 输出活动帧数据为全局变量
    file << "window.activityData = [\n";
    for (size_t i = 0; i < frames.size(); ++i) {
        file << "{ t: " << frames[i].time_ms << ", acts: [";
        for (size_t j = 0; j < frames[i].active_neurons.size(); ++j) {
            if (j > 0) file << ",";
            file << "[" << frames[i].active_neurons[j] << "," << std::fixed << std::setprecision(2) << frames[i].intensities[j] << "]";
        }
        file << "]}";
        if (i < frames.size() - 1) file << ",\n";
    }
    file << "\n];\n";
    file << "</script>\n";

    // ES module 脚本：加载 Three.js 和 OrbitControls 并初始化 3D 场景
    file << "<script type=\"module\">\n";
    file << "import * as THREE from 'three';\n";
    file << "import { OrbitControls } from 'three/addons/controls/OrbitControls.js';\n";
    file << "\n";
    file << "const neurons = window.neuronData;\n";
    file << "const scene = new THREE.Scene();\n";
    file << "scene.background = new THREE.Color(0x0a0a1a);\n";
    file << "const camera = new THREE.PerspectiveCamera(60, window.innerWidth/window.innerHeight, 0.1, 2000);\n";
    file << "camera.position.set(300, 200, 300);\n";
    file << "const renderer = new THREE.WebGLRenderer({antialias:true});\n";
    file << "renderer.setSize(window.innerWidth, window.innerHeight);\n";
    file << "renderer.setPixelRatio(window.devicePixelRatio);\n";
    file << "document.body.appendChild(renderer.domElement);\n";
    file << "const controls = new OrbitControls(camera, renderer.domElement);\n";
    file << "controls.enableDamping = true;\n";
    file << "controls.dampingFactor = 0.05;\n";
    file << "controls.target.set(0, -40, 0);\n";
    file << "const geom = new THREE.BufferGeometry();\n";
    file << "const positions = new Float32Array(neurons.length*3);\n";
    file << "const colors = new Float32Array(neurons.length*3);\n";
    file << "for(let i=0; i<neurons.length; i++){\n";
    file << "  positions[i*3]=neurons[i].x; positions[i*3+1]=neurons[i].y; positions[i*3+2]=neurons[i].z;\n";
    file << "  colors[i*3]=neurons[i].r; colors[i*3+1]=neurons[i].g; colors[i*3+2]=neurons[i].b;\n";
    file << "}\n";
    file << "geom.setAttribute('position', new THREE.BufferAttribute(positions,3));\n";
    file << "geom.setAttribute('color', new THREE.BufferAttribute(colors,3));\n";
    file << "const mat = new THREE.PointsMaterial({size:3, vertexColors:true, transparent:true, opacity:0.85, sizeAttenuation:true});\n";
    file << "const points = new THREE.Points(geom, mat);\n";
    file << "scene.add(points);\n";
    file << "const ambientLight = new THREE.AmbientLight(0x404040);\n";
    file << "scene.add(ambientLight);\n";
    
    // UI 控制和动画逻辑
    file << "const activityData = window.activityData || [];\n";
    file << "const maxFrame = Math.max(0, activityData.length - 1);\n";
    file << "const timeline = document.getElementById('timeline');\n";
    file << "const playBtn = document.getElementById('playBtn');\n";
    file << "const timeLabel = document.getElementById('timeLabel');\n";
    file << "timeline.max = maxFrame;\n";
    
    file << "let isPlaying = false;\n";
    file << "let currentFrame = 0;\n";
    file << "let lastTime = performance.now();\n";
    file << "const FPS = 30;\n"; // 每秒播放帧数
    file << "const frameDuration = 1000 / FPS;\n";

    file << "playBtn.addEventListener('click', () => {\n";
    file << "  isPlaying = !isPlaying;\n";
    file << "  playBtn.textContent = isPlaying ? 'PAUSE' : 'PLAY';\n";
    file << "  if(isPlaying) lastTime = performance.now();\n";
    file << "});\n";
    
    file << "timeline.addEventListener('input', (e) => {\n";
    file << "  currentFrame = parseInt(e.target.value);\n";
    file << "  updateColors();\n";
    file << "});\n";

    file << "function updateColors() {\n";
    file << "  if(activityData.length === 0) return;\n";
    file << "  const frame = activityData[currentFrame];\n";
    file << "  if(!frame) return;\n";
    file << "  timeLabel.textContent = frame.t.toFixed(1) + ' ms';\n";
    file << "  timeline.value = currentFrame;\n";
    
    file << "  // Reset all colors to base\n";
    file << "  const targetColors = points.geometry.attributes.color.array;\n";
    file << "  for(let i=0; i<neurons.length; i++) {\n";
    file << "    targetColors[i*3] = neurons[i].r * 0.4;\n";
    file << "    targetColors[i*3+1] = neurons[i].g * 0.4;\n";
    file << "    targetColors[i*3+2] = neurons[i].b * 0.4;\n";
    file << "  }\n";
    
    file << "  // Highlight active neurons\n";
    file << "  for(const act of frame.acts) {\n";
    file << "    const idx = act[0];\n";
    file << "    const intensity = act[1] * 0.6 + 0.4;\n"; // 亮度增强
    file << "    targetColors[idx*3] = neurons[idx].r + (1.0 - neurons[idx].r) * intensity * 0.8;\n";
    file << "    targetColors[idx*3+1] = neurons[idx].g + (1.0 - neurons[idx].g) * intensity * 0.8;\n";
    file << "    targetColors[idx*3+2] = neurons[idx].b + (1.0 - neurons[idx].b) * intensity * 0.8;\n";
    file << "  }\n";
    file << "  points.geometry.attributes.color.needsUpdate = true;\n";
    file << "}\n";

    file << "window.addEventListener('resize', () => {\n";
    file << "  camera.aspect = window.innerWidth/window.innerHeight;\n";
    file << "  camera.updateProjectionMatrix();\n";
    file << "  renderer.setSize(window.innerWidth, window.innerHeight);\n";
    file << "});\n";
    file << "function animate(now){\n";
    file << "  requestAnimationFrame(animate);\n";
    file << "  controls.update();\n";
    file << "  if(isPlaying && now - lastTime > frameDuration) {\n";
    file << "    currentFrame = (currentFrame + 1) % (maxFrame + 1);\n";
    file << "    updateColors();\n";
    file << "    lastTime = now;\n";
    file << "  }\n";
    file << "  renderer.render(scene, camera);\n";
    file << "}\n";
    file << "updateColors();\n";
    file << "animate(performance.now());\n";
    file << "</script>\n</body></html>\n";

    return file.good();
}

bool DataExporter::exportToNeuroML(const Connectome& connectome,
                                   const BrainLayout3D& layout,
                                   const std::string& filename) {
    std::ofstream file(filename);
    if (!file) return false;
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    file << "<neuroml xmlns=\"http://www.neuroml.org/schema/neuroml2\">\n";
    file << "  <network id=\"fruit_fly_brain\" temperature=\"295.0\">\n";
    for (size_t i = 0; i < connectome.numNeurons(); ++i) {
        const auto& pos = layout.getPosition(static_cast<uint32_t>(i));
        file << "    <cell id=\"cell_" << i << "\"><morphology>";
        file << "<point x=\"" << pos.x << "\" y=\"" << pos.y << "\" z=\"" << pos.z << "\"/>";
        file << "</morphology></cell>\n";
    }
    const auto& conn = connectome.connectivity();
    for (const auto& syn : conn.synapses()) {
        file << "    <synapseFrom=\"cell_" << syn.preId() << "\" synapseTo=\"cell_" << syn.postId() << "\"/>\n";
    }
    file << "  </network>\n</neuroml>\n";
    return file.good();
}

bool DataExporter::exportToCSV(const Connectome& connectome,
                              const BrainLayout3D& layout,
                              const std::string& filename) {
    std::ofstream file(filename);
    if (!file) return false;
    file << "id,x,y,z,region\n";
    for (size_t i = 0; i < connectome.numNeurons(); ++i) {
        const auto& neuron = connectome.getNeuron(static_cast<uint32_t>(i));
        const auto& pos = layout.getPosition(static_cast<uint32_t>(i));
        file << i << "," << pos.x << "," << pos.y << "," << pos.z << "," << static_cast<int>(neuron.region()) << "\n";
    }
    return file.good();
}

bool DataExporter::exportToBinary(const Connectome& connectome,
                                  const BrainLayout3D& layout,
                                  const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) return false;
    uint32_t magic = 0x464C5942, version = 1;
    uint32_t num_neurons = static_cast<uint32_t>(connectome.numNeurons());
    uint32_t num_synapses = static_cast<uint32_t>(connectome.numSynapses());
    file.write(reinterpret_cast<const char*>(&magic), sizeof(magic));
    file.write(reinterpret_cast<const char*>(&version), sizeof(version));
    file.write(reinterpret_cast<const char*>(&num_neurons), sizeof(num_neurons));
    file.write(reinterpret_cast<const char*>(&num_synapses), sizeof(num_synapses));
    for (size_t i = 0; i < num_neurons; ++i) {
        const auto& pos = layout.getPosition(static_cast<uint32_t>(i));
        const auto& neuron = connectome.getNeuron(static_cast<uint32_t>(i));
        file.write(reinterpret_cast<const char*>(&pos.x), sizeof(double));
        file.write(reinterpret_cast<const char*>(&pos.y), sizeof(double));
        file.write(reinterpret_cast<const char*>(&pos.z), sizeof(double));
        uint8_t region = static_cast<uint8_t>(neuron.region());
        file.write(reinterpret_cast<const char*>(&region), sizeof(region));
    }
    const auto& conn = connectome.connectivity();
    for (const auto& syn : conn.synapses()) {
        uint32_t pre = syn.preId(), post = syn.postId();
        uint8_t type = static_cast<uint8_t>(syn.type());
        uint32_t contacts = syn.numContacts();
        file.write(reinterpret_cast<const char*>(&pre), sizeof(pre));
        file.write(reinterpret_cast<const char*>(&post), sizeof(post));
        file.write(reinterpret_cast<const char*>(&type), sizeof(type));
        file.write(reinterpret_cast<const char*>(&contacts), sizeof(contacts));
    }
    return file.good();
}

} // namespace viz
} // namespace flybrain
