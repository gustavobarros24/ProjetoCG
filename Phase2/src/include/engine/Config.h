#ifndef CONFIG_H
#define CONFIG_H

#include "Model.h"
#include <variant>

struct Translation {
    const float x;
    const float y;
    const float z;
};

struct Rotation {
    const float angle;
    const float x;
    const float y;
    const float z;
};

struct Scaling {
    const float x;
    const float y;
    const float z;
};

using Transform = std::variant<
    Translation,
    Rotation,
    Scaling
>;

struct Window {
    int width = 512;
    int height = 512;
};

struct Camera {
    std::array<double, 3> position = { 0, 0, 0 };
    std::array<double, 3> lookAt = { 0, 0, 0 };
    std::array<double, 3> up = { 0, 1, 0 };
    
    double fov = 60.0;
    double nearClip = 1.0;
    double farClip = 1000.0;
};


struct Group {
    glm::vec3 colour = { 1.0f, 1.0f, 1.0f };
    std::vector<Model> models;
    std::vector<Transform> transforms;
    std::vector<Group> subgroups;
};

struct World {
    Window window;
    Camera camera;
    std::vector<Group> groups;
};

#endif