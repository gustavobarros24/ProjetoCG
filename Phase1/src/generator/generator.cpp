#include <cstring>
#include <cstdlib>
#include <iostream>

#include "Model.h"
#include "SquarePlane.h"
#include "SquareBox.h"
#include "Sphere.h"
#include "Cone.h"

int main(int argc, char** argv) {

	if (argc < 5) {
		std::cerr << "Invalid number of arguments!\n"
			<< "Usage:\n"
			<< "  plane <float:length> <int:divisions> <string:output_filename>\n"
			<< "  box <float:length> <int:divisions> <string:output_filename>\n"
			<< "  sphere <float:radius> <int:slices> <int:stacks> <string:output_filename>\n"
			<< "  cone <float:radius> <float:height> <int:slices> <int:stacks> <string:output_filename>\n";
		return 1;
	}

	const char* shape = argv[1];
	const char* file_path = nullptr;

	if (strcmp(shape, "plane") == 0) {
		if (argc < 5) {
			std::cerr << "Invalid number of arguments for " << shape << "!\n";
			return 1;
		}
		
		float length = atof(argv[2]);
		int divisions = atoi(argv[3]);
		file_path = argv[4];

		SquarePlane(length, divisions).toFile(file_path);
	}
	else if (strcmp(shape, "box") == 0) {
		if (argc < 5) {
			std::cerr << "Invalid number of arguments for " << shape << "!\n";
			return 1;
		}
		
		float length = atof(argv[2]);
		int divisions = atoi(argv[3]);
		file_path = argv[4];

		SquareBox(length, divisions).toFile(file_path);
	}
	else if (strcmp(shape, "sphere") == 0) {
		if (argc < 6) {
			std::cerr << "Invalid number of arguments for sphere!\n";
			return 1;
		}
		
		float radius = atof(argv[2]);
		int slices = atoi(argv[3]);
		int stacks = atoi(argv[4]);
		file_path = argv[5];

		Sphere(radius, slices, stacks).toFile(file_path);
	}
	else if (strcmp(shape, "cone") == 0) {
		if (argc < 7) {
			std::cerr << "Invalid number of arguments for cone!\n";
			return 1;
		}
		
		float radius = atof(argv[2]);
		float height = atof(argv[3]);
		int slices = atoi(argv[4]);
		int stacks = atoi(argv[5]);
		file_path = argv[6];

		Cone(radius, height, slices, stacks).toFile(file_path);
	}
	else {
		std::cerr << "Invalid shape specified!\n";
		return 1;
	}

	return 0;
}