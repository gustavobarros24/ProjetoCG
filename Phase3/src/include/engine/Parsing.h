#ifndef PARSING_H
#define PARSING_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace modelFileManagement {

	using namespace std::filesystem;

	static path ModelsFolder() {
		path modelsFolder = current_path().parent_path() / "models";
		create_directory(modelsFolder);
		return modelsFolder;
	}

	std::vector<float> loadModelVerticesOld(const std::string& filename) {
		auto filepath = ModelsFolder() / filename;
		std::ifstream file(filepath);
		if (!file) throw std::runtime_error("Failed to open file: " + filepath.string());

		std::vector<glm::vec3> temp_vertices;  // Temporary storage for raw vertices
		std::vector<float> vertices;           // Final vertex data with proper ordering

		std::string line;

		while (std::getline(file, line)) {
			// Skip empty lines or comments
			if (line.empty() || line[0] == '#') continue;

			std::istringstream iss(line);
			std::string prefix;
			iss >> prefix;

			if (prefix == "v") {
				// Parse vertex data
				glm::vec3 vertex;
				if (!(iss >> vertex.x >> vertex.y >> vertex.z)) {
					throw std::runtime_error("Failed to parse vertex line: " + line);
				}
				temp_vertices.push_back(vertex);
			}
			else if (prefix == "f") {
				std::string faceData;
				while (iss >> faceData) {
					// handle different face formats: v, v/vt, v//vn, v/vt/vn
					std::istringstream faceStream(faceData);
					std::string vertexIndexStr;
					std::getline(faceStream, vertexIndexStr, '/'); // we only care about the vertex index

					try {
						int vertexIndex = std::stoi(vertexIndexStr);
						if (vertexIndex > 0 && vertexIndex <= temp_vertices.size()) { // obj indices are 1-based
							const auto& vertex = temp_vertices[vertexIndex - 1];
							vertices.insert(vertices.end(), { vertex.x, vertex.y, vertex.z });
						}
						else {
							throw std::runtime_error("Invalid vertex index in face: " + line);
						}
					}
					catch (...) {
						throw std::runtime_error("Failed to parse face vertex index: " + line);
					}
				}
			}
		}

		// If there were no face definitions, just return all vertices in order
		if (vertices.empty()) {
			for (const auto& vertex : temp_vertices) {
				vertices.insert(vertices.end(), { vertex.x, vertex.y, vertex.z });
			}
		}

		return vertices;
	}

	std::vector<float> loadModelVertices(const std::string& filename) {
		auto filepath = ModelsFolder() / filename;
		std::ifstream file(filepath);
		if (!file) throw std::runtime_error("Failed to open file: " + filepath.string());

		std::vector<glm::vec3> temp_vertices;  // Temporary storage for raw vertices
		std::vector<float> vertices;           // Final vertex data with proper ordering

		std::string line;

		while (std::getline(file, line)) {
			if (line.empty() || line[0] == '#') continue;

			std::istringstream iss(line);
			std::string prefix;
			iss >> prefix;

			if (prefix == "v") {
				glm::vec3 vertex;
				if (!(iss >> vertex.x >> vertex.y >> vertex.z)) {
					throw std::runtime_error("Failed to parse vertex line: " + line);
				}
				temp_vertices.push_back(vertex);
			}
			else if (prefix == "f") {
				std::vector<int> vertexIndices;
				std::string faceData;
				while (iss >> faceData) {
					// Handle different face formats: v, v/vt, v//vn, v/vt/vn
					std::istringstream faceStream(faceData);
					std::string vertexIndexStr;
					std::getline(faceStream, vertexIndexStr, '/'); // We only care about the vertex index

					try {
						int vertexIndex = std::stoi(vertexIndexStr);
						if (vertexIndex > 0 && vertexIndex <= temp_vertices.size()) {
							vertexIndices.push_back(vertexIndex);
						}
						else {
							throw std::runtime_error("Invalid vertex index in face: " + line);
						}
					}
					catch (...) {
						throw std::runtime_error("Failed to parse face vertex index: " + line);
					}
				}

				// triangulate the face assuming convex polygons
				// there will be more than 1 tri if over 3 elements are mentioned: n vertices -> n-2 tris
				if (vertexIndices.size() >= 3) {
					for (size_t i = 1; i < vertexIndices.size() - 1; ++i) {
						// vertexIndices[0], vertexIndices[i], vertexIndices[i+1]
						vertices.insert(vertices.end(), {
							temp_vertices[vertexIndices[0] - 1].x,
							temp_vertices[vertexIndices[0] - 1].y,
							temp_vertices[vertexIndices[0] - 1].z
							});
						vertices.insert(vertices.end(), {
							temp_vertices[vertexIndices[i] - 1].x,
							temp_vertices[vertexIndices[i] - 1].y,
							temp_vertices[vertexIndices[i] - 1].z
							});
						vertices.insert(vertices.end(), {
							temp_vertices[vertexIndices[i + 1] - 1].x,
							temp_vertices[vertexIndices[i + 1] - 1].y,
							temp_vertices[vertexIndices[i + 1] - 1].z
							});
					}
				}
			}
		}

		// If there were no face definitions, just return all vertices in order
		if (vertices.empty()) {
			for (const auto& vertex : temp_vertices) {
				vertices.insert(vertices.end(), { vertex.x, vertex.y, vertex.z });
			}
		}

		return vertices;
	}
};

#include "Config.h"
#include <pugixml.hpp>


bool saysTrue(const std::string& input) {
	if (input.length() != 4) return false;

	return (tolower(input[0]) == 't' &&
		tolower(input[1]) == 'r' &&
		tolower(input[2]) == 'u' &&
		tolower(input[3]) == 'e');
}

namespace configParser {

	using namespace std::filesystem;

	static path ConfigFile(const std::string& filename) {
		return current_path().parent_path() / "xml" / filename;
	}

	std::vector<std::string> getUniqueModelFilenames(const pugi::xml_document& doc) {
		std::unordered_set<std::string> filenames;

		pugi::xpath_node_set models = doc.select_nodes("//model[@file]");

		for (pugi::xpath_node node : models) {
			filenames.insert(node.node().attribute("file").value());
		}

		std::vector<std::string> filenames_vector;
		filenames_vector.assign(filenames.begin(), filenames.end());

		return filenames_vector;
	}

	void printIndent(int depth) {
		for (int i = 0; i < depth; ++i) {
			std::cout << " || ";
		}
	}

	glm::vec3 readColour(const pugi::xml_node& colourNode) {

		/*
		std::cout
			<< std::format(
				"Colour: r={} g={} b={}",
				colourNode.attribute("r").value(),
				colourNode.attribute("g").value(),
				colourNode.attribute("b").value())
			<< std::endl;
		*/

		return glm::vec3(
			std::stof(colourNode.attribute("r").value()),
			std::stof(colourNode.attribute("g").value()),
			std::stof(colourNode.attribute("b").value())
		);
	}

	std::vector<Transform> readTransforms(const pugi::xml_node& transformsNode, int depth) {

		std::vector<Transform> detectedTransforms = {};

		/*
		std::cout
			<< "Transform:"
			<< std::endl;
		*/

		for (pugi::xml_node childNode : transformsNode.children()) {

			// printIndent(depth + 1);

			if (std::strcmp(childNode.name(), "translate") == 0) {

				/*
				std::cout
					<< std::format(
						"Translate: x={} y={} z={}",
						childNode.attribute("x").value(),
						childNode.attribute("y").value(),
						childNode.attribute("z").value())
					<< std::endl;
				*/

				if (childNode.attribute("time")) {
					
					float time = std::stof(childNode.attribute("time").value());
					
					bool aligned = childNode.attribute("align") &&
						saysTrue(childNode.attribute("align").value());

					std::vector<glm::vec3> controlPoints;
					for (pugi::xml_node pointNode : childNode.children("point")) {
						controlPoints.emplace_back(
							std::stof(pointNode.attribute("x").value()),
							std::stof(pointNode.attribute("y").value()),
							std::stof(pointNode.attribute("z").value())
						);
					}

					std::cout 
						<< std::format(
							"AnimatedTranslate:\n"
							"  Period: {:.2f}s\n"
							"  Aligned: {}\n"
							"  Control Points [{}]:",
							time,
							aligned ? "true" : "false",
							controlPoints.size()) 
						<< std::endl;

					if (controlPoints.size() >= 4)
						detectedTransforms.push_back(AnimatedTranslation(
							controlPoints, time, aligned
						));
					else 
						std::cerr 
							<< std::format(
								"Warning: Animated translation needs at least 4 control points. Found {}",
								controlPoints.size())
							<< std::endl;
				}
				else {
				detectedTransforms.push_back(Translation(
					std::stof(childNode.attribute("x").value()),
					std::stof(childNode.attribute("y").value()),
					std::stof(childNode.attribute("z").value())
				));
				}
			}
			else if (std::strcmp(childNode.name(), "rotate") == 0) {

				// Check if it's an animated rotation (has time attribute)
				if (childNode.attribute("time")) {

					// Animated rotation
					float time = std::stof(childNode.attribute("time").value());
					glm::vec3 axis(
						std::stof(childNode.attribute("x").value()),
						std::stof(childNode.attribute("y").value()),
						std::stof(childNode.attribute("z").value())
					);

					std::cout 
						<< std::format(
							"AnimatedRotate: period={:.2f}s axis=({:.2f}, {:.2f}, {:.2f})",
							time,
							axis.x, axis.y, axis.z) 
						<< std::endl;

					detectedTransforms.push_back(AnimatedRotation(axis, time));
				}
				else {

					/*
					std::cout
						<< std::format(
								"Rotate: angle={} x={} y={} z={}",
								childNode.attribute("angle").value(),
								childNode.attribute("x").value(),
								childNode.attribute("y").value(),
								childNode.attribute("z").value())
						<< std::endl;
					*/

					detectedTransforms.push_back(Rotation(
						std::stof(childNode.attribute("angle").value()),
						std::stof(childNode.attribute("x").value()),
						std::stof(childNode.attribute("y").value()),
						std::stof(childNode.attribute("z").value())
					));
				}
			}
			else if (std::strcmp(childNode.name(), "scale") == 0) {

				/*
				std::cout
					<< std::format(
						"Scale: x={} y={} z={}",
						childNode.attribute("x").value(),
						childNode.attribute("y").value(),
						childNode.attribute("z").value())
					<< std::endl;
				*/

				detectedTransforms.push_back(Scaling(
					std::stof(childNode.attribute("x").value()),
					std::stof(childNode.attribute("y").value()),
					std::stof(childNode.attribute("z").value())
				));
			}
		}

		for (auto& t : detectedTransforms) {
			std::cout << TransformToString(t) << std::endl;
		}

		return detectedTransforms;
	}

	std::vector<std::string> readModels(const pugi::xml_node& modelsNode, int depth) {

		/*
		std::cout
			<< "Models:"
			<< std::endl;
		*/

		std::vector<std::string> detectedModels;

		for (pugi::xml_node modelNode : modelsNode.children("model")) {

			/*
			std::vector<float> modelVerts = fileManagement::loadModelVertices(modelNode.attribute("file").value());

			printIndent(depth + 1);
			std::cout
				<< std::format(
					"Model: \"{}\" ({} vertices) (fourth vertex: {},{},{})",
					modelNode.attribute("file").value(),
					modelVerts.size()/3,
					modelVerts[3*3],
					modelVerts[3*3+1],
					modelVerts[3*3+2])
				<< Model(modelNode.attribute("file").value()).toString()
				<< std::endl;
			*/

			detectedModels.push_back(modelNode.attribute("file").value());
		}

		return detectedModels;
	}

	Group readGroup(const pugi::xml_node& groupNode, int depth);

	std::vector<Group> readGroups(const pugi::xml_node& parentGroupNode, int depth) {

		std::vector<Group> detectedGroups = {};

		for (pugi::xml_node group : parentGroupNode.children("group")) {
			detectedGroups.push_back(readGroup(group, depth));
		}

		return detectedGroups;
	}

	Group readGroup(const pugi::xml_node& groupNode, int depth) {

		Group group;

		/*
		printIndent(depth);
		if (depth == 0) std::cout << std::endl;
		std::cout
			<< std::format("Group ({}):", depth)
			<< std::endl;


		if (!groupNode.attribute("desc").empty()) {

			printIndent(depth+1);
			std::cout
				<< std::format(
					"Description: {}",
					groupNode.attribute("desc").value())
				<< std::endl;
		}
		*/

		for (pugi::xml_node childNode : groupNode.children()) {

			if (std::strcmp(childNode.name(), "colour") == 0) {

				//printIndent(depth + 1);

				group.colour = std::optional<glm::vec3>{
					readColour(childNode)
				};
			}
			else if (std::strcmp(childNode.name(), "transform") == 0) {

				//printIndent(depth + 1);

				group.transforms = readTransforms(childNode, depth + 1);
			}
			else if (std::strcmp(childNode.name(), "models") == 0) {

				//printIndent(depth + 1);

				group.modelFilenames = readModels(childNode, depth + 1);
			}
			else if (std::strcmp(childNode.name(), "group") == 0) {

				group.subgroups = readGroups(groupNode, depth + 1);
			}
		}

		//std::cout << std::string(group) << std::endl;

		return group;
	}

	Camera readCamera(const pugi::xml_node& cameraNode) {
		pugi::xml_node position = cameraNode.child("position");
		pugi::xml_node lookAt = cameraNode.child("lookAt");
		pugi::xml_node up = cameraNode.child("up");
		pugi::xml_node projection = cameraNode.child("projection");

		/*
		std::cout
			<< "Camera:"
			<< std::endl;

		printIndent(1);
		std::cout
			<< std::format(
				"Position: x={} y={} z={}",
				position.attribute("x").value(),
				position.attribute("y").value(),
				position.attribute("z").value())
			<< std::endl;

		printIndent(1);
		std::cout
			<< std::format(
				"LookAt: x={} y={} z={}",
				lookAt.attribute("x").value(),
				lookAt.attribute("y").value(),
				lookAt.attribute("z").value())
			<< std::endl;

		printIndent(1);
		std::cout
			<< std::format(
				"Up: x={} y={} z={}",
				up.attribute("x").value(),
				up.attribute("y").value(),
				up.attribute("z").value())
			<< std::endl;

		printIndent(1);
		std::cout
			<< std::format(
				"Projection: fov={} near={} far={}",
				projection.attribute("fov").value(),
				projection.attribute("near").value(),
				projection.attribute("far").value())
			<< std::endl;
		*/

		return Camera(
			glm::vec3(
				std::stof(position.attribute("x").value()),
				std::stof(position.attribute("y").value()),
				std::stof(position.attribute("z").value())
			),
			glm::vec3(
				std::stof(lookAt.attribute("x").value()),
				std::stof(lookAt.attribute("y").value()),
				std::stof(lookAt.attribute("z").value())
			),
			glm::vec3(
				std::stof(up.attribute("x").value()),
				std::stof(up.attribute("y").value()),
				std::stof(up.attribute("z").value())
			),
			std::stod(projection.attribute("fov").value()),
			std::stod(projection.attribute("near").value()),
			std::stod(projection.attribute("far").value())
		);
	}

	Window readWindow(const pugi::xml_node& windowNode) {
		/*
		std::cout
			<< std::format(
				"Window: width={} height={}",
				windowNode.attribute("width").value(),
				windowNode.attribute("height").value())
			<< std::endl;
		*/

		return Window(
			std::stoi(windowNode.attribute("width").value()),
			std::stoi(windowNode.attribute("height").value())
		);
	}

	World loadWorld(std::string configFilename) {

		path configPath = ConfigFile(configFilename);

		pugi::xml_document doc;
		if (!doc.load_file(configPath.string().c_str())) {
			std::cerr
				<< "Could not load XML file at: " << configPath
				<< std::endl;
		}

		World w;
		w.window = readWindow(doc.child("world").child("window"));
		w.camera = readCamera(doc.child("world").child("camera"));
		w.modelStorage = ModelStorage(getUniqueModelFilenames(doc));
		w.groups = readGroups(doc.child("world"), 0);
		AnimatedTranslation::worldUp = w.camera.up;

		return w;
	}
}

#endif