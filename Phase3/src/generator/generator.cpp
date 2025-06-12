#include <cstring>
#include <cstdlib>
#include <iostream>

#include "Models.h"

void generateDefaultModels() {
    // Generate default models with some reasonable parameters
    std::cout << "No arguments provided. Generating default models...\n";

    fileManagement::exportToOBJ(
        generateVertices::sphere(1.0f, 30, 30),
        "sphere.3d"
    );

    fileManagement::exportToOBJ(
        generateVertices::tube(0.6f, 1.0f, 1.0f, 30),
        "saturn_ring.3d"
    );

    fileManagement::exportToOBJ(
        generateVertices::bezier("teapot.patch", 10),
        "bezier_10.3d"
    );

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
            std::cerr << "plane <float:length> <int:divisions> <string:output_filename>";
            return 1;
        }

        float length = atof(argv[2]);
        int divisions = atoi(argv[3]);
        filepath = argv[4];

        fileManagement::exportToOBJ(
            generateVertices::plane(length, divisions),
            filepath
        );
    }
    else if (strcmp(shape, "box") == 0) {
        if (argc < 5) {
            std::cerr << "box <float:length> <int:divisions> <string:output_filename>";
            return 1;
        }

        float length = atof(argv[2]);
        int divisions = atoi(argv[3]);
        filepath = argv[4];

        fileManagement::exportToOBJ(
            generateVertices::box(length, divisions),
            filepath
        );
    }
    else if (strcmp(shape, "sphere") == 0) {
        if (argc < 6) {
            std::cerr << "sphere <float:radius> <int:slices> <int:stacks> <string:output_filename>";
            return 1;
        }

        float radius = atof(argv[2]);
        int slices = atoi(argv[3]);
        int stacks = atoi(argv[4]);
        filepath = argv[5];

        fileManagement::exportToOBJ(
            generateVertices::sphere(radius, slices, stacks),
            filepath
        );
    }
    else if (strcmp(shape, "cone") == 0) {
        if (argc < 7) {
            std::cerr << "cone <float:radius> <float:height> <int:slices> <int:stacks> <string:output_filename>";
            return 1;
        }

        float radius = atof(argv[2]);
        float height = atof(argv[3]);
        int slices = atoi(argv[4]);
        int stacks = atoi(argv[5]);
        filepath = argv[6];

        fileManagement::exportToOBJ(
            generateVertices::cone(radius, height, slices, stacks),
            filepath
        );
    }
    else if (strcmp(shape, "tube") == 0) {
        if (argc < 7) {
            std::cerr << "tube <float:inner_radius> <float:outer_radius> <float:height> <int:slices> <string:output_filename>";
            return 1;
        }

        float iradius = atof(argv[2]);
        float oradius = atof(argv[3]);
        float height = atoi(argv[4]);
        int slices = atoi(argv[5]);
        filepath = argv[6];

        fileManagement::exportToOBJ(
            generateVertices::tube(iradius, oradius, height, slices),
            filepath
        );
    }
    else if (strcmp(shape, "bezier") == 0) {
        if (argc < 5) {
            std::cerr << "bezier <string:input_filename> <int:tessellation_level> <string:output_filepath>";
            return 1;
        }

        const std::string inputFilename = argv[2];
        const int tessellationLevel = atoi(argv[3]);
        filepath = argv[4];

        //auto [controlPoints, patches] = fileManagement::PatchFileData(inputFilename);
        
        fileManagement::exportToOBJ(
            //generateVertices::bezier(controlPoints, patches, tessellationLevel),
            generateVertices::bezier(inputFilename, tessellationLevel),
            filepath
        );
    }

    else {
        std::cerr << "Invalid shape specified!\n";
        return 1;
    }

    return 0;
}