#ifndef PARSING_H
#define PARSING_H

#include "../engine/Config.h"
#include "../tinyXML2/tinyxml2.h"

namespace xmlUtils {
    // Gets first child element by name (nullptr if not found)
    inline tinyxml2::XMLElement* getChild(tinyxml2::XMLElement* parent, const char* name) {
        return parent->FirstChildElement(name);
    }

    // Gets float attribute with default value
    inline float getFloatAttr(tinyxml2::XMLElement* elem, const char* name, float defaultVal = 0.0f) {
        float val = defaultVal;
        elem->QueryFloatAttribute(name, &val);
        return val;
    }

    // Gets int attribute with default value
    inline int getIntAttr(tinyxml2::XMLElement* elem, const char* name, int defaultVal = 0) {
        int val = defaultVal;
        elem->QueryIntAttribute(name, &val);
        return val;
    }

    // Iterates through child elements
    template<typename Func>
    inline void forEachChild(tinyxml2::XMLElement* parent, const char* name, Func func) {
        for (auto child = parent->FirstChildElement(name); child; child = child->NextSiblingElement(name)) {
            func(child);
        }
    }
}

class ConfigParser {
public:
    static World loadFromFile(const std::string& filename) {
        std::cout << "=== Loading config file: " << filename << " ===\n";
        using namespace xmlUtils;

        World config;
        tinyxml2::XMLDocument doc;

        // Load file
        std::filesystem::path path = getConfigPath(filename);
        std::cout << "Looking for config at: " << path << "\n";

        if (doc.LoadFile(path.string().c_str()) != tinyxml2::XML_SUCCESS) {
            throw std::runtime_error("Failed to load XML file: " + path.string());
        }
        std::cout << "File loaded successfully\n";

        // Parse world
        tinyxml2::XMLElement* worldElem = doc.FirstChildElement("world");
        if (!worldElem) throw std::runtime_error("Missing <world> element");

        std::cout << "\n[Parsing Window]\n";
        config.window = parseWindow(worldElem);

        std::cout << "\n[Parsing Camera]\n";
        config.camera = parseCamera(worldElem);

        std::cout << "\n[Parsing Groups Hierarchy]\n";
        config.groups = parseGroups(worldElem);

        std::cout << "\n=== Config parsing completed ===\n";
        return config;
    }

private:
    static Window parseWindow(tinyxml2::XMLElement* worldElem) {
        Window window;
        if (auto windowElem = xmlUtils::getChild(worldElem, "window")) {
            window.width = xmlUtils::getIntAttr(windowElem, "width", 512);
            window.height = xmlUtils::getIntAttr(windowElem, "height", 512);

            std::cout << "Window settings:\n"
                << "  Width: " << window.width << "\n"
                << "  Height: " << window.height << "\n";
        }
        else std::cout << "Using default window settings\n";
        return window;
    }

    static Camera parseCamera(tinyxml2::XMLElement* worldElem) {
        Camera camera;
        if (auto cameraElem = xmlUtils::getChild(worldElem, "camera")) {
            std::cout << "Found camera element\n";

            if (auto posElem = xmlUtils::getChild(cameraElem, "position")) {
                camera.position = {
                    xmlUtils::getFloatAttr(posElem, "x"),
                    xmlUtils::getFloatAttr(posElem, "y"),
                    xmlUtils::getFloatAttr(posElem, "z")
                };
                std::cout << "  Position: (" << camera.position[0] << ", "
                    << camera.position[1] << ", " << camera.position[2] << ")\n";
            }

            if (auto lookAtElem = xmlUtils::getChild(cameraElem, "lookAt")) {
                camera.lookAt = {
                    xmlUtils::getFloatAttr(lookAtElem, "x"),
                    xmlUtils::getFloatAttr(lookAtElem, "y"),
                    xmlUtils::getFloatAttr(lookAtElem, "z")
                };
                std::cout << "  LookAt: (" << camera.lookAt[0] << ", "
                    << camera.lookAt[1] << ", " << camera.lookAt[2] << ")\n";
            }

            if (auto upElem = xmlUtils::getChild(cameraElem, "up")) {
                camera.up = {
                    xmlUtils::getFloatAttr(upElem, "x"),
                    xmlUtils::getFloatAttr(upElem, "y"),
                    xmlUtils::getFloatAttr(upElem, "z")
                };
                std::cout << "  Up: (" << camera.up[0] << ", "
                    << camera.up[1] << ", " << camera.up[2] << ")\n";
            }

            if (auto projElem = xmlUtils::getChild(cameraElem, "projection")) {
                camera.fov = xmlUtils::getFloatAttr(projElem, "fov", 60.0f);
                camera.nearClip = xmlUtils::getFloatAttr(projElem, "near", 1.0f);
                camera.farClip = xmlUtils::getFloatAttr(projElem, "far", 1000.0f);

                std::cout << "  Projection:\n"
                    << "    FOV: " << camera.fov << "\n"
                    << "    Near: " << camera.nearClip << "\n"
                    << "    Far: " << camera.farClip << "\n";
            }
        }
        else std::cout << "Using default camera settings\n";
        return camera;
    }

    static std::vector<Group> parseGroups(tinyxml2::XMLElement* parentElem, int depth = 0) {
        std::vector<Group> groups;
        const std::string indent(depth * 2, ' ');

        xmlUtils::forEachChild(parentElem, "group", [&](tinyxml2::XMLElement* groupElem) {
            std::cout << indent << "Found group (level " << depth << ")\n";
            groups.push_back(parseGroup(groupElem, depth));
            });

        return groups;
    }

    static Group parseGroup(tinyxml2::XMLElement* groupElem, int depth = 0) {
        Group group;
        const std::string indent(depth * 2, ' ');

        if (auto colorElem = xmlUtils::getChild(groupElem, "colour")) {
            group.colour = {
                xmlUtils::getFloatAttr(colorElem, "r", 1.0f),
                xmlUtils::getFloatAttr(colorElem, "g", 1.0f),
                xmlUtils::getFloatAttr(colorElem, "b", 1.0f)
            };
            std::cout << indent << "  Colour: ("
                << group.colour.r << ", "
                << group.colour.g << ", "
                << group.colour.b << ")\n";
        }
        else {
            group.colour = { 1.0f, 1.0f, 1.0f };
        }

        // Parse transforms
        if (auto transformsElem = xmlUtils::getChild(groupElem, "transform")) {
            std::cout << indent << "  Parsing transforms:\n";
            xmlUtils::forEachChild(transformsElem, nullptr, [&](tinyxml2::XMLElement* transElem) {
                const std::string type = transElem->Name();

                if (type == "translate") {
                    Translation t{
                        xmlUtils::getFloatAttr(transElem, "x"),
                        xmlUtils::getFloatAttr(transElem, "y"),
                        xmlUtils::getFloatAttr(transElem, "z")
                    };
                    group.transforms.emplace_back(t);
                    std::cout << indent << "    Translation: ("
                        << t.x << ", " << t.y << ", " << t.z << ")\n";
                }
                else if (type == "rotate") {
                    Rotation r{
                        xmlUtils::getFloatAttr(transElem, "angle"),
                        xmlUtils::getFloatAttr(transElem, "x"),
                        xmlUtils::getFloatAttr(transElem, "y"),
                        xmlUtils::getFloatAttr(transElem, "z")
                    };
                    group.transforms.emplace_back(r);
                    std::cout << indent << "    Rotation: angle=" << r.angle
                        << " axis=(" << r.x << ", " << r.y << ", " << r.z << ")\n";
                }
                else if (type == "scale") {
                    Scaling s{
                        xmlUtils::getFloatAttr(transElem, "x", 1.0f),
                        xmlUtils::getFloatAttr(transElem, "y", 1.0f),
                        xmlUtils::getFloatAttr(transElem, "z", 1.0f)
                    };
                    group.transforms.emplace_back(s);
                    std::cout << indent << "    Scaling: ("
                        << s.x << ", " << s.y << ", " << s.z << ")\n";
                }
                });
        }

        // Parse models
        if (auto modelsElem = xmlUtils::getChild(groupElem, "models")) {
            std::cout << indent << "  Parsing models:\n";
            xmlUtils::forEachChild(modelsElem, "model", [&](tinyxml2::XMLElement* modelElem) {
                if (const char* file = modelElem->Attribute("file")) {
                    std::cout << indent << "    Loading model from: " << file << "\n";
                    group.models.push_back(modelFiles::load(file));
                }
                });
        }

        // Parse subgroups recursively
        group.subgroups = parseGroups(groupElem, depth + 1);

        return group;
    }

    static std::filesystem::path getConfigPath(const std::string& filename) {
        return std::filesystem::current_path().parent_path() / "xml" / filename;
    }
};

#endif