#ifndef MODEL_H
#define MODEL_H

#include <iostream>
#include <fstream>
#include <sstream>

#include <filesystem>

#include <vector>
#include "Vec3.h"
#include "Constants.h"

class Model {
public:
    std::vector<Vec3> vertices;
    
    inline void toFile(const char* filename) {

        std::filesystem::path modelsPath = getModelsDirectory();
        std::filesystem::path filePath = modelsPath / filename;

        if (std::filesystem::exists(filePath)) {
            std::filesystem::remove(filePath);
        }

        std::ofstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filePath << std::endl;
            return;
        }

        for (const auto& vertex : vertices) {
            file << vertex.x << "," << vertex.y << "," << vertex.z << std::endl;
        }

        file.close();
    }

    inline static Model fromFile(const char* filename) {
        Model model;
        std::filesystem::path modelsPath = model.getModelsDirectory();
        std::filesystem::path filePath = modelsPath / filename;

        std::ifstream file(filePath);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << filePath << std::endl;
            return model; // Return an empty model if the file cannot be opened
        }

        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string token;
            float x, y, z;

            // Read x
            if (!std::getline(iss, token, ',')) continue;
            x = std::stof(token);

            // Read y
            if (!std::getline(iss, token, ',')) continue;
            y = std::stof(token);

            // Read z
            if (!std::getline(iss, token, ',')) continue;
            z = std::stof(token);

            model.vertices.emplace_back(x, y, z);
        }

        file.close();
        return model;
    }

    virtual ~Model() = default;
    
private:
    inline std::filesystem::path getModelsDirectory() {
        
        std::filesystem::path currentPath = std::filesystem::current_path();
        std::filesystem::path modelsPath = currentPath.parent_path() / "models";

        std::filesystem::create_directory(modelsPath);

        return modelsPath;
    }
};

#endif
