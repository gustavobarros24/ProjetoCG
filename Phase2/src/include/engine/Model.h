#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <glm/glm.hpp>

class Model {
public:
    std::vector<float> vertices;
    
    inline void addVertex(const glm::vec3 vertex) {
        vertices.push_back(vertex.x);
        vertices.push_back(vertex.y);
        vertices.push_back(vertex.z);
    }
};

namespace modelFiles {

    inline std::filesystem::path getDirectory() {
        std::filesystem::path currentPath = std::filesystem::current_path();
        std::filesystem::path modelsPath = currentPath.parent_path() / "models";
        std::filesystem::create_directory(modelsPath);
        return modelsPath;
    }

    inline static Model load(const char* filename) {
        Model model;
        std::filesystem::path filepath = modelFiles::getDirectory() / filename;

        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filepath << std::endl;
            return model;
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string token;
            float x, y, z;

            if (!std::getline(iss, token, ',')) continue;
            x = std::stof(token);

            if (!std::getline(iss, token, ',')) continue;
            y = std::stof(token);

            if (!std::getline(iss, token, ',')) continue;
            z = std::stof(token);

            model.addVertex(glm::vec3(x, y, z));
        }

        file.close();

        return model;
    }
}

#endif
