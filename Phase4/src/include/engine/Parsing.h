#ifndef PARSING_H
#define PARSING_H

#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "Config.h"
#include <pugixml.hpp>

namespace modelFileManagement {

	using namespace std::filesystem;

	static path ModelsFolder() {
		path modelsFolder = current_path().parent_path() / "models";
		create_directory(modelsFolder);
		return modelsFolder;
	}

	static path TexturesFolder() {
		path texfolder = ModelsFolder() / "textures";
		create_directory(texfolder);
		return texfolder;
	}

	void exportOBJ(std::string filename, Model model) {
		path filepath = ModelsFolder() / filename;

		if (exists(filepath))
			remove(filepath);

		std::ofstream file(ModelsFolder() / filename);

		// Write vertices
		for (auto& v : model.vertices)
			file << std::format("v {:.6f} {:.6f} {:.6f}\n", v.x, v.y, v.z);

		file << "\n";

		// Write normals (with validation)
		for (auto& vn : model.normals) {
			if (!std::isfinite(vn.x)) vn.x = 0.0f;
			if (!std::isfinite(vn.y)) vn.y = 0.0f;
			if (!std::isfinite(vn.z)) vn.z = 0.0f;
			file << std::format("vn {:.6f} {:.6f} {:.6f}\n", vn.x, vn.y, vn.z);
		}

		file << "\n";

		// Write texture coordinates
		for (auto& vt : model.texcoords)
			file << std::format("vt {:.6f} {:.6f}\n", vt.s, vt.t);

		file << "\n";

		// Write faces (fixed indexing)
		for (size_t i = 0; i < model.vIndices.size(); i += 3) {
			if (i + 2 >= model.vIndices.size()) break; // safety check

			file << std::format("f {}/{}/{} {}/{}/{} {}/{}/{}\n",
				1 + model.vIndices[i],     1 + model.vtIndices[i],     1 + model.vnIndices[i],
				1 + model.vIndices[i + 1], 1 + model.vtIndices[i + 1], 1 + model.vnIndices[i + 1],
				1 + model.vIndices[i + 2], 1 + model.vtIndices[i + 2], 1 + model.vnIndices[i + 2]
			);
		}

		file.close();
	}
	
	Model importOBJ(const std::string& filename) {
		Model model;

		using namespace std::filesystem;
		using namespace modelFileManagement;

		auto filepath = ModelsFolder() / filename;
		std::ifstream file(filepath);
		if (!file) throw std::runtime_error("Failed to open file: " + filepath.string());

		std::string line;
		long lineno = -1;

		while (std::getline(file, line)) {
			lineno++;
			if (line.empty() || line[0] == '#') continue;


			std::istringstream iss(line);
			std::string prefix;
			iss >> prefix;

			// Parse vertex coords
			if (prefix == "v") {
				glm::vec3 v;
				if (!(iss >> v.x >> v.y >> v.z))
					throw std::runtime_error("Failed to parse vertex line: " + line);
				model.vertices.push_back(v);
			}
			// Parse normal coords
			else if (prefix == "vn") {
				//std::cout << std::format("OBJ {}, line {} - loading vn", filename, lineno) << std::endl;
				glm::vec3 vn;
				if (!(iss >> vn.x >> vn.y >> vn.z))
					throw std::runtime_error("Failed to parse normal line: " + line);
				model.normals.push_back(vn);
			}
			// Parse tex coords
			else if (prefix == "vt") {
				//std::cout << std::format("OBJ {}, line {} - loading vt", filename, lineno) << std::endl;
				glm::vec2 vt;
				if (!(iss >> vt.x >> vt.y))
					throw std::runtime_error("Failed to parse texcoords line: " + line);
				model.texcoords.push_back(vt);
			}
			// Parse tri face indices
			else if (prefix == "f") {

				//std::cout << std::format("OBJ {}, line {} - loading face", filename, lineno) << std::endl;

				std::vector<std::string> faceTokens;
				std::string token;
				while (iss >> token) {
					faceTokens.push_back(token);
				}

				if (faceTokens.size() != 3) {
					throw std::runtime_error("Only triangular faces are supported");
				}

				for (const auto& token : faceTokens) {
					std::istringstream viss(token);
					std::string vPart, vtPart, vnPart;

					// Parse vertex index
					std::getline(viss, vPart, '/');
					if (vPart.empty()) throw std::runtime_error("Vertex index is required");
					model.vIndices.push_back(std::stoul(vPart) - 1);  // OBJ indices are 1-based

					// Parse texture coordinate index if exists
					if (std::getline(viss, vtPart, '/') && !vtPart.empty()) {
						model.vtIndices.push_back(std::stoul(vtPart) - 1);
					}

					// Parse normal index if exists (after second slash)
					if (std::getline(viss, vnPart, '/') && !vnPart.empty()) {
						model.vnIndices.push_back(std::stoul(vnPart) - 1);
					}
				}
			}
		}

		// Validate that we have matching counts for indices
		if (!model.vtIndices.empty() && model.vtIndices.size() != model.vIndices.size()) {
			throw std::runtime_error("Vertex texture indices count doesn't match vertex indices count");
		}
		if (!model.vnIndices.empty() && model.vnIndices.size() != model.vIndices.size()) {
			throw std::runtime_error("Vertex normal indices count doesn't match vertex indices count");
		}

		return model;
	}
	
	Model exportThenImportOBJ(std::string filename, Model inputModel) {
		exportOBJ(filename, inputModel);
		return importOBJ(filename);
	}
	
	unsigned int importTexture(std::string filename) {

		ilInit();
		ilEnable(IL_ORIGIN_SET);
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT);

		ILuint imageID;
		ilGenImages(1, &imageID);
		ilBindImage(imageID);

		bool imageLoaded = ilLoadImage(const_cast<char*>((modelFileManagement::TexturesFolder() / filename).string().c_str()));
		if (!imageLoaded) {
			std::cout << "Failed to load texture: " << filename << std::endl;
			return false;
		}

		// IL

		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		unsigned int imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
		unsigned int imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
		void* imageTexels = ilGetData();

		// GLEW

		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		///////////////////////////////////////////////////////
		// filters using original texture
		// -> GL_LINEAR (aka bilinear)
		// -> GL_NEAREST
		// 
		// filters using nearest mipmap
		// -> GL_LINEAR_MIPMAP_NEAREST
		// -> GL_NEAREST_MIPMAP_NEAREST
		// 
		// filters using linear approx of adjacent mipmaps
		// -> GL_LINEAR_MIPMAP_LINEAR (aka trilinear)
		// -> GL_NEAREST_MIPMAP_LINEAR
		///////////////////////////////////////////////////////

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (Texture::minFilter==GL_NEAREST) ? GL_NEAREST : GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Texture::minFilter);

		const int LOD = 0;
		glTexImage2D(
			GL_TEXTURE_2D, LOD, GL_RGBA,
			imageWidth, imageHeight, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, imageTexels
		);

		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
		ilDeleteImages(1, &imageID);

		return textureID;
	}
};

bool saysTrue(const std::string& input) {
	if (input.length() != 4) return false;

	return (tolower(input[0]) == 't' &&
		    tolower(input[1]) == 'r' &&
		    tolower(input[2]) == 'u' &&
		    tolower(input[3]) == 'e'
		);
}

namespace configParser {
	using namespace std::filesystem;
	
	pugi::xml_document doc;

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

	std::vector<std::string> getUniqueTextureFilenames(const pugi::xml_document& doc) {
		std::unordered_set<std::string> filenames;

		// Find all texture nodes that are children of model nodes
		pugi::xpath_node_set textures = doc.select_nodes("//model/texture[@file]");

		for (pugi::xpath_node node : textures) {
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

	void readLights(const pugi::xml_node& lightsNode) {

		static const float DEFAULT_CUTOFF = 30.0f;

		for (pugi::xml_node lightNode : lightsNode.children("light")) {

			std::string type = lightNode.attribute("type").value();

			if (type == "point") {
				LightCaster::loadPoint({
					lightNode.attribute("posx").as_float(),
					lightNode.attribute("posy").as_float(),
					lightNode.attribute("posz").as_float()
					});
			}
			else if (type == "directional") {
				LightCaster::loadDirectional({
					lightNode.attribute("dirx").as_float(),
					lightNode.attribute("diry").as_float(),
					lightNode.attribute("dirz").as_float()
					});
			}
			else if (type == "spot" || type == "spotlight") {
				LightCaster::loadSpotlight(
					{
						lightNode.attribute("posx").as_float(),
						lightNode.attribute("posy").as_float(),
						lightNode.attribute("posz").as_float(),
					},
					{
						lightNode.attribute("dirx").as_float(),
						lightNode.attribute("diry").as_float(),
						lightNode.attribute("dirz").as_float(),
					},
					lightNode.attribute("cutoff").as_float(DEFAULT_CUTOFF)
					);
			}
		}
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

					//std::cout 
					//	<< std::format(
					//		"AnimatedTranslate:\n"
					//		"  Period: {:.2f}s\n"
					//		"  Aligned: {}\n"
					//		"  Control Points [{}]:",
					//		time,
					//		aligned ? "true" : "false",
					//		controlPoints.size()) 
					//	<< std::endl;

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

					//std::cout 
					//	<< std::format(
					//		"AnimatedRotate: period={:.2f}s axis=({:.2f}, {:.2f}, {:.2f})",
					//		time,
					//		axis.x, axis.y, axis.z) 
					//	<< std::endl;

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

		//for (auto& t : detectedTransforms) {
		//	std::cout << TransformToString(t) << std::endl;
		//}

		return detectedTransforms;
	}

	Material readMaterial(const pugi::xml_node& colorNode, int depth) {
	
		Material mtl;

		for (pugi::xml_node childNode : colorNode.children()) {

			// printIndent(depth + 1);

			if (std::strcmp(childNode.name(), "diffuse") == 0) {

				mtl.diffuse[0] = std::stof(childNode.attribute("R").value()) / 255.0f;
				mtl.diffuse[1] = std::stof(childNode.attribute("G").value()) / 255.0f;
				mtl.diffuse[2] = std::stof(childNode.attribute("B").value()) / 255.0f;
			}
			else if (std::strcmp(childNode.name(), "ambient") == 0) {

				mtl.ambient[0] = std::stof(childNode.attribute("R").value()) / 255.0f;
				mtl.ambient[1] = std::stof(childNode.attribute("G").value()) / 255.0f;
				mtl.ambient[2] = std::stof(childNode.attribute("B").value()) / 255.0f;
				
			}
			else if (std::strcmp(childNode.name(), "specular") == 0) {

				mtl.specular[0] = std::stof(childNode.attribute("R").value()) / 255.0f;
				mtl.specular[1] = std::stof(childNode.attribute("G").value()) / 255.0f;
				mtl.specular[2] = std::stof(childNode.attribute("B").value()) / 255.0f;
			}
			else if (std::strcmp(childNode.name(), "emissive") == 0) {

				mtl.emissive[0] = std::stof(childNode.attribute("R").value()) / 255.0f;
				mtl.emissive[1] = std::stof(childNode.attribute("G").value()) / 255.0f;
				mtl.emissive[2] = std::stof(childNode.attribute("B").value()) / 255.0f;
			}
			else if (std::strcmp(childNode.name(), "shininess") == 0) {

				mtl.shininess[0] = std::stof(childNode.attribute("value").value());
			}
		}

		return mtl;

	}
	
	std::vector<std::string> readModels(const pugi::xml_node& modelsNode, int depth) { 	// to be discontinued

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

	std::vector<Group::ModelReference> readModelReferences(const pugi::xml_node& modelsNode, int depth) {

		std::cout
			<< "Models:"
			<< std::endl;

		std::vector<Group::ModelReference> detectedModelRefs;

		for (pugi::xml_node modelNode : modelsNode.children("model")) {

			Group::ModelReference mref;
			mref.modelFilename = modelNode.attribute("file").value();
			
			printIndent(depth + 1);
			std::cout
				<< std::format(
					"Model \"{}\" ",
					mref.modelFilename);

			for (pugi::xml_node childNode : modelNode.children()) {

			
				std::string texFilename = "";
				Material mtl;

				if (std::strcmp(childNode.name(), "texture") == 0) {
					mref.textureFilename = childNode.attribute("file").value();
					
					std::cout
						<< std::format(
							"(tex: \"{}\") ",
							mref.textureFilename);
				}
				else if (std::strcmp(childNode.name(), "color") == 0) {
					mref.material = readMaterial(childNode, depth);
					std::cout
						<< std::format(
							"(mtl: diffuse({},{},{}) ambient({},{},{}) specular({},{},{}) emissive({},{},{}) shininess={})",
							mref.material.diffuse[0], mref.material.diffuse[1], mref.material.diffuse[2],
							mref.material.ambient[0], mref.material.ambient[1], mref.material.ambient[2],
							mref.material.specular[0], mref.material.specular[1], mref.material.specular[2],
							mref.material.emissive[0], mref.material.emissive[1], mref.material.emissive[2],
							mref.material.shininess[0]
							);
				}
				
			}

			std::cout << std::endl;
			detectedModelRefs.push_back(mref);
		}

		return detectedModelRefs;
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

		for (pugi::xml_node childNode : groupNode.children()) {

			//if (std::strcmp(childNode.name(), "colour") == 0) {
			//
			//	//printIndent(depth + 1);
			//
			//	group.colour = std::optional<glm::vec3>{
			//		readColour(childNode)
			//	};
			//}

			if (std::strcmp(childNode.name(), "transform") == 0) {

				printIndent(depth + 1);

				group.transforms = readTransforms(childNode, depth + 1);
			}
			else if (std::strcmp(childNode.name(), "models") == 0) {

				printIndent(depth + 1);
	
				group.modelReferences = readModelReferences(childNode, depth + 1);
				//group.modelFilenames = readModels(childNode, depth + 1);
			}
			else if (std::strcmp(childNode.name(), "group") == 0) {

				group.subgroups = readGroups(groupNode, depth + 1);
			}
		}

		//std::cout << std::string(group) << std::endl;

		return group;
	}

	void readCamera(const pugi::xml_node& cameraNode) {
		pugi::xml_node position = cameraNode.child("position");
		pugi::xml_node lookAt = cameraNode.child("lookAt");
		pugi::xml_node up = cameraNode.child("up");
		pugi::xml_node projection = cameraNode.child("projection");

		CameraController::currentPlacement =
		CameraController::initialPlacement = {
			.pos = {
				position.attribute("x").as_float(1.0f),
				position.attribute("y").as_float(1.0f),
				position.attribute("z").as_float(1.0f)
			},
			.target = {
				lookAt.attribute("x").as_float(0.0f),
				lookAt.attribute("y").as_float(0.0f),
				lookAt.attribute("z").as_float(0.0f)
			},
			.up = {
				up.attribute("x").as_float(0.0f),
				up.attribute("y").as_float(1.0f),
				up.attribute("z").as_float(0.0f)
			}
		};

		CameraController::currentProjection = 
		CameraController::initialProjection = {
			.fov  = projection.attribute("fov").as_double(60.0),
			.near = projection.attribute("near").as_double(1.0),
			.far  = projection.attribute("far").as_double(1000.0)
		};


	}

	void readWindow(const pugi::xml_node& windowNode) {

		static const int DEFAULT_SIZE = 512;

		WindowState::currentWidth =
		WindowState::initialWidth =
			windowNode.attribute("width").as_int(DEFAULT_SIZE);
		
		WindowState::currentHeight =
		WindowState::initialHeight =
			windowNode.attribute("height").as_int(DEFAULT_SIZE);
		
	}

	World loadWorld(std::string configFilename) {

		path configPath = ConfigFile(configFilename);
		
		if (!doc.load_file(configPath.string().c_str())) {
			std::cerr
				<< "Could not load XML file at: " << configPath
				<< std::endl;
		}

		pugi::xml_node lightsNode = doc.child("world").child("lights");
		if (lightsNode)
			readLights(lightsNode);

		World w;
		readWindow(doc.child("world").child("window"));
		readCamera(doc.child("world").child("camera"));
		w.groups = readGroups(doc.child("world"), 0);

		return w;
	}

	void importModels() {
		std::vector<std::string> modelFilenames = getUniqueModelFilenames(doc);

		std::cout << std::format("Requested Models ({}):\n", modelFilenames.size());
		for (auto& m : modelFilenames) std::cout << m << std::endl;
		std::cout << std::endl;

		for (auto& modelName : modelFilenames) ModelStorage::load(modelName, modelFileManagement::importOBJ(modelName));
		std::cout << std::format("Loaded Models ({}):\n", ModelStorage::models.size());
		for (auto& [modelName, model] : ModelStorage::models) std::cout << modelName << std::endl;
		std::cout << std::endl;
	}

	void importTextures() {
		std::vector<std::string> textureFilenames = getUniqueTextureFilenames(doc);

		std::cout << std::format("Requested Textures ({}):\n", textureFilenames.size());
		for (auto& t : textureFilenames) std::cout << t << std::endl;
		std::cout << std::endl;

		for (auto& texName : textureFilenames)
			Texture::load(texName, modelFileManagement::importTexture(texName));
		std::cout << std::format("Loaded Textures ({}):\n", Texture::textureIDs.size());
		for (auto& [filename, id] : Texture::textureIDs)
			std::cout << std::format("{} (id {})", filename, id) << std::endl;
		std::cout << std::endl;
	}
	
}

#endif