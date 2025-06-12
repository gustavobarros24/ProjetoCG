#ifndef WORLDCONFIG_H
#define WORLDCONFIG_H

#include <array>
#include <vector>
#include <filesystem>

#include "WorldGroup.h"
#include "tinyXML2/tinyxml2.h"

class WorldConfig {
public:
    // Window
    int windowWidth;
    int windowHeight;

    // Camera: initial position
    std::array<double, 3> cameraPos;
    std::array<double, 3> cameraLookAt;
    std::array<double, 3> cameraUp;

    // Camera: initial projection
    double cameraFOV;
    double cameraNearClip;
    double cameraFarClip;

    // Hierarchical encapsulations of transformations and models
    std::vector<WorldGroup> groups;

    // Constructor
    WorldConfig(const std::string& filename) {
        std::filesystem::path configPath = getConfigDirectory() / filename;

        if (!fromXML(configPath.string())) {
            throw std::runtime_error("Failed to load XML file: " + configPath.string());
        }
    }

private:
    // Load instance from XML file
    inline bool fromXML(const std::string& filename) {
        tinyxml2::XMLDocument doc;
        if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
            return false;  // Failed to load the XML file
        }

        tinyxml2::XMLElement* worldElement = doc.FirstChildElement("world");
        if (!worldElement) {
            return false;  // No <world> element found
        }

        // Parse window settings
        tinyxml2::XMLElement* windowElement = worldElement->FirstChildElement("window");
        if (windowElement) {
            windowElement->QueryIntAttribute("width", &windowWidth);
            windowElement->QueryIntAttribute("height", &windowHeight);
        }

        // Parse camera settings
        tinyxml2::XMLElement* cameraElement = worldElement->FirstChildElement("camera");
        if (cameraElement) {
            tinyxml2::XMLElement* posElement = cameraElement->FirstChildElement("position");
            if (posElement) {
                posElement->QueryDoubleAttribute("x", &cameraPos[0]);
                posElement->QueryDoubleAttribute("y", &cameraPos[1]);
                posElement->QueryDoubleAttribute("z", &cameraPos[2]);
            }

            tinyxml2::XMLElement* lookAtElement = cameraElement->FirstChildElement("lookAt");
            if (lookAtElement) {
                lookAtElement->QueryDoubleAttribute("x", &cameraLookAt[0]);
                lookAtElement->QueryDoubleAttribute("y", &cameraLookAt[1]);
                lookAtElement->QueryDoubleAttribute("z", &cameraLookAt[2]);
            }

            tinyxml2::XMLElement* upElement = cameraElement->FirstChildElement("up");
            if (upElement) {
                upElement->QueryDoubleAttribute("x", &cameraUp[0]);
                upElement->QueryDoubleAttribute("y", &cameraUp[1]);
                upElement->QueryDoubleAttribute("z", &cameraUp[2]);
            }

            tinyxml2::XMLElement* projElement = cameraElement->FirstChildElement("projection");
            if (projElement) {
                projElement->QueryDoubleAttribute("fov", &cameraFOV);
                projElement->QueryDoubleAttribute("near", &cameraNearClip);
                projElement->QueryDoubleAttribute("far", &cameraFarClip);
            }
        }

        // Parse groups
        tinyxml2::XMLElement* groupElement = worldElement->FirstChildElement("group");
        while (groupElement) {
            WorldGroup group;
            if (group.fromXML(groupElement)) {
                groups.push_back(group);
            }
            groupElement = groupElement->NextSiblingElement("group");
        }

        return true;
    }

    // Helper function to get the configuration directory
    inline std::filesystem::path getConfigDirectory() {
        return std::filesystem::current_path().parent_path() / "xml";
    }
};

#endif