#include <cstring>
#include <cstdlib>
#include <iostream>

#include "Models.h"

void generateDefaultModels() {
    // Generate default models with some reasonable parameters
    std::cout << "Generating default models...\n";

    // Default plane
    modelFiles::save(
        generateVertices::plane(2.0f, 4),
        "default_plane.3d"
    );

    // Default box
    modelFiles::save(
        generateVertices::box(1.0f, 4),
        "default_box.3d"
    );

    // Default sphere
    modelFiles::save(
        generateVertices::sphere(1.0f, 16, 8),
        "default_sphere.3d"
    );

    // Default cone
    modelFiles::save(
        generateVertices::cone(1.0f, 2.0f, 16, 8),
        "default_cone.3d"
    );

    // Default tube
    modelFiles::save(
        generateVertices::tube(0.5f, 1.0f, 2.0f, 16),
        "default_tube.3d"
    );

    std::cout << "Generated default models:\n"
        << "  default_plane.3d\n"
        << "  default_box.3d\n"
        << "  default_sphere.3d\n"
        << "  default_cone.3d\n"
        << "  default_tube.3d\n";
}

int main(int argc, char** argv) {
    if (argc == 1) {
        generateDefaultModels();
        return 0;
    }

    if (argc < 5) {
        std::cerr << "Invalid number of arguments!\n"
            << "Usage:\n"
            << "  plane <float:length> <int:divisions> <string:output_filename>\n"
            << "  box <float:length> <int:divisions> <string:output_filename>\n"
            << "  sphere <float:radius> <int:slices> <int:stacks> <string:output_filename>\n"
            << "  cone <float:radius> <float:height> <int:slices> <int:stacks> <string:output_filename>\n"
            << "  tube <float:inner_radius> <float:outer_radius> <float:height> <int:slices> <string:output_filename>\n";
        return 1;
    }

    const char* shape = argv[1];
    const char* filepath = nullptr;

    if (strcmp(shape, "plane") == 0) {
        if (argc < 5) {
            std::cerr << "Invalid number of arguments for " << shape << "!\n";
            return 1;
        }

        float length = atof(argv[2]);
        int divisions = atoi(argv[3]);
        filepath = argv[4];

        modelFiles::save(
            generateVertices::plane(length, divisions),
            filepath
        );
    }
    else if (strcmp(shape, "box") == 0) {
        if (argc < 5) {
            std::cerr << "Invalid number of arguments for " << shape << "!\n";
            return 1;
        }

        float length = atof(argv[2]);
        int divisions = atoi(argv[3]);
        filepath = argv[4];

        modelFiles::save(
            generateVertices::box(length, divisions),
            filepath
        );
    }
    else if (strcmp(shape, "sphere") == 0) {
        if (argc < 6) {
            std::cerr << "Invalid number of arguments for " << shape << "!\n";
            return 1;
        }

        float radius = atof(argv[2]);
        int slices = atoi(argv[3]);
        int stacks = atoi(argv[4]);
        filepath = argv[5];

        modelFiles::save(
            generateVertices::sphere(radius, slices, stacks),
            filepath
        );
    }
    else if (strcmp(shape, "cone") == 0) {
        if (argc < 7) {
            std::cerr << "Invalid number of arguments for " << shape << "!\n";
            return 1;
        }

        float radius = atof(argv[2]);
        float height = atof(argv[3]);
        int slices = atoi(argv[4]);
        int stacks = atoi(argv[5]);
        filepath = argv[6];

        modelFiles::save(
            generateVertices::cone(radius, height, slices, stacks),
            filepath
        );
    }
    else if (strcmp(shape, "tube") == 0) {
        if (argc < 7) {
            std::cerr << "Invalid number of arguments for " << shape << "!\n";
            return 1;
        }

        float iradius = atof(argv[2]);
        float oradius = atof(argv[3]);
        float height = atoi(argv[4]);
        int slices = atoi(argv[5]);
        filepath = argv[6];

        modelFiles::save(
            generateVertices::tube(iradius, oradius, height, slices),
            filepath
        );
    }
    else {
        std::cerr << "Invalid shape specified!\n";
        return 1;
    }

    return 0;
}