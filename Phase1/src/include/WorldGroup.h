#ifndef WORLDGROUP_H
#define WORLDGROUP_H

#include <vector>
#include "Model.h"
#include "tinyXML2/tinyxml2.h"

class WorldGroup {
public:
    std::vector<Model> models;
    std::vector<WorldGroup> subgroups;

    WorldGroup() = default;

    bool fromXML(tinyxml2::XMLElement* groupElement) {
        if (!groupElement) {
            return false; // Invalid XML element
        }

        // Parse models
        tinyxml2::XMLElement* modelsElement = groupElement->FirstChildElement("models");
        if (modelsElement) {
            tinyxml2::XMLElement* modelElement = modelsElement->FirstChildElement("model");
            while (modelElement) {
                const char* file = modelElement->Attribute("file");
                if (file) {
                    // Load the model from the file using Model::fromFile
                    Model model = Model::fromFile(file);
                    models.push_back(model);
                }
                modelElement = modelElement->NextSiblingElement("model");
            }
        }

        // Parse subgroups
        tinyxml2::XMLElement* subgroupElement = groupElement->FirstChildElement("group");
        while (subgroupElement) {
            WorldGroup subgroup;
            if (subgroup.fromXML(subgroupElement)) {
                subgroups.push_back(subgroup);
            }
            subgroupElement = subgroupElement->NextSiblingElement("group");
        }

        return true;
    }
};

#endif