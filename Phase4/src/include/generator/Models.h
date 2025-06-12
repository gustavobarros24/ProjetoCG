#ifndef MODELS_H
#define MODELS_H

#define _USE_MATH_DEFINES
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

struct ModelData {

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texcoords;

	std::vector<unsigned int> vIndices;
	std::vector<unsigned int> vnIndices;
	std::vector<unsigned int> vtIndices;
};

namespace fileManagement {

	using namespace std::filesystem;

	path ModelsFolder() {
		path modelsFolder = current_path().parent_path() / "models";
		create_directory(modelsFolder);
		return modelsFolder;
	}

	void exportToOBJ(const std::vector<float>& vertices, const std::string& filename) {

		path filepath = ModelsFolder() / filename;

		if (exists(filepath))
			remove(filepath);

		std::ofstream file(ModelsFolder() / filename);

		// Write vertices
		for (size_t i = 0; i < vertices.size(); i += 3) {
			file << "v " << vertices[i] << " " << vertices[i + 1] << " " << vertices[i + 2] << "\n";
		}

		file.close();
	}

	void exportOBJ(ModelData model, std::string filename) {

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


	void exportToOBJOld(ModelData model, std::string filename) {

		path filepath = ModelsFolder() / filename;

		if (exists(filepath))
			remove(filepath);

		std::ofstream file(ModelsFolder() / filename);

		for (auto& v : model.vertices) 
			file << std::format("v {:.6f} {:.6f} {:.6f} \n", v.x, v.y, v.z);

		file << "\n\n\n";

		for (auto& vn : model.normals)
			file << std::format("vn {:.6f} {:.6f} {:.6f} \n", vn.x, vn.y, vn.z);

		file << "\n\n\n";

		for (auto& vt : model.texcoords)
			file << std::format("vt {:.6f} {:.6f} \n", vt.s, vt.t);

		for (int i = 0; i < model.vIndices.size() / 3; i += 3)

			// obj tri face syntax: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
			// (indexing starts on 1)
			
			file << std::format("f {}/{}/{} {}/{}/{} {}/{}/{} \n",
				std::to_string(1 + model.vIndices[i]),
				std::to_string(1 + model.vtIndices[i]),
				std::to_string(1 + model.vnIndices[i]),

				std::to_string(1 + model.vIndices[i + 1]),
				std::to_string(1 + model.vtIndices[i + 1]),
				std::to_string(1 + model.vnIndices[i + 1]),

				std::to_string(1 + model.vIndices[i + 2]),
				std::to_string(1 + model.vtIndices[i + 2]),
				std::to_string(1 + model.vnIndices[i + 2])
				);

		file.close();
	}
}

namespace bezier2 {

	const glm::mat4 M(
		-1, 3, -3, 1,
		3, -6, 3, 0,
		-3, 3, 0, 0,
		1, 0, 0, 0
	);

	glm::mat4 MPMt(const glm::mat4& P) {
		return (M * P) * glm::transpose(M);
	}

	// Evaluates a point on a Bézier patch at parameters (u, v)
	std::pair<glm::vec3, glm::vec3> point(
		const glm::mat4& MPMt_x,
		const glm::mat4& MPMt_y,
		const glm::mat4& MPMt_z,
		float u, float v) {

		// Create parameter vectors
		glm::vec4 U(u * u * u, u * u, u, 1);
		glm::vec4 V(v * v * v, v * v, v, 1);
		glm::vec4 dU(3 * u * u, 2 * u, 1, 0);
		glm::vec4 dV(3 * v * v, 2 * v, 1, 0);

		glm::vec3 position = {
			glm::dot(U * MPMt_x, V),
			glm::dot(U * MPMt_y, V),
			glm::dot(U * MPMt_z, V),
		};

		glm::vec3 du = {
			glm::dot(dU * MPMt_x, V),
			glm::dot(dU * MPMt_y, V),
			glm::dot(dU * MPMt_z, V)
		};

		glm::vec3 dv = {
			glm::dot(U * MPMt_x, dV),
			glm::dot(U * MPMt_y, dV),
			glm::dot(U * MPMt_z, dV)
		};

		glm::vec3 normal = glm::normalize(glm::cross(du, dv));

		return { position, normal };
	}

	void renderPatch(const glm::mat4& Px, const glm::mat4& Py, const glm::mat4& Pz, int tessellation) {

		glm::mat4 MPMt_x = bezier2::MPMt(Px);
		glm::mat4 MPMt_y = bezier2::MPMt(Py);
		glm::mat4 MPMt_z = bezier2::MPMt(Pz);

		// Precompute all surface points
		std::vector<std::vector<glm::vec3>> surfacePoints;
		surfacePoints.resize(tessellation + 1);

		for (int i = 0; i <= tessellation; ++i) {

			float u = static_cast<float>(i) / tessellation;
			surfacePoints[i].resize(tessellation + 1);

			for (int j = 0; j <= tessellation; ++j) {

				float v = static_cast<float>(j) / tessellation;
				auto [pos, _] = point(MPMt_x, MPMt_y, MPMt_z, u, v);
				surfacePoints[i][j] = pos;
			}
		}
		/*
		// draw the surface
		glBegin(GL_TRIANGLES);
		for (int i = 0; i < tessellation; ++i) {
			for (int j = 0; j < tessellation; ++j) {
				// First triangle of the quad
				glVertex3fv(glm::value_ptr(surfacePoints[i][j]));
				glVertex3fv(glm::value_ptr(surfacePoints[i + 1][j]));
				glVertex3fv(glm::value_ptr(surfacePoints[i][j + 1]));

				// Second triangle of the quad
				glVertex3fv(glm::value_ptr(surfacePoints[i + 1][j]));
				glVertex3fv(glm::value_ptr(surfacePoints[i + 1][j + 1]));
				glVertex3fv(glm::value_ptr(surfacePoints[i][j + 1]));
			}
		}
		glEnd();

		// draw the control point normals
		glPushAttrib(GL_CURRENT_BIT);
		glColor3f(1, 0, 0);
		glBegin(GL_LINES);
		for (int i = 0; i <= tessellation; ++i) {
			float u = static_cast<float>(i) / tessellation;

			for (int j = 0; j <= tessellation; ++j) {

				float v = static_cast<float>(j) / tessellation;
				auto [pos, normal] = patchpoint(MPMt_x, MPMt_y, MPMt_z, u, v);

				glVertex3fv(glm::value_ptr(pos));
				glVertex3fv(glm::value_ptr(pos + normal * 0.2f));
			}
		}
		glEnd();
		glPopAttrib();
		*/
	}

	std::tuple<glm::mat4, glm::mat4, glm::mat4> Pxyz(std::vector<glm::vec3>& controlPoints) {
	
		glm::mat4 Px;
		glm::mat4 Py;
		glm::mat4 Pz;

		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				// The patchPoints are assumed to be in row-major order
				const glm::vec3& point = controlPoints[i * 4 + j];
				Px[j][i] = point.x;
				Py[j][i] = point.y;
				Pz[j][i] = point.z;
			}
		}

		return { Px, Py, Pz };
	}

	struct Patch {

		glm::mat4 MPMt_x;
		glm::mat4 MPMt_y;
		glm::mat4 MPMt_z;

		Patch(std::vector<glm::vec3>& controlPoints) {
		
			auto [Px, Py, Pz] = Pxyz(controlPoints);

			this->MPMt_x = bezier2::MPMt(Px);
			this->MPMt_y = bezier2::MPMt(Py);
			this->MPMt_z = bezier2::MPMt(Pz);
		
		}

		std::pair<glm::vec3, glm::vec3> evaluate(float u, float v) {
		
			// Create parameter vectors
			const glm::vec4 U(u * u * u, u * u, u, 1);
			const glm::vec4 V(v * v * v, v * v, v, 1);
			const glm::vec4 dU(3 * u * u, 2 * u, 1, 0);
			const glm::vec4 dV(3 * v * v, 2 * v, 1, 0);

			glm::vec3 position = {
				glm::dot(U * MPMt_x, V),
				glm::dot(U * MPMt_y, V),
				glm::dot(U * MPMt_z, V),
			};

			glm::vec3 du = {
				glm::dot(dU * MPMt_x, V),
				glm::dot(dU * MPMt_y, V),
				glm::dot(dU * MPMt_z, V)
			};

			glm::vec3 dv = {
				glm::dot(U * MPMt_x, dV),
				glm::dot(U * MPMt_y, dV),
				glm::dot(U * MPMt_z, dV)
			};

			glm::vec3 normal = glm::normalize(glm::cross(du, dv));

			return { position, normal };
		}
	};
};

namespace generateVertices {
	
	glm::vec3 vNormal(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, bool ccw = true) {
		return (ccw) ?
			glm::normalize(glm::cross(
				v1 - v0,
				v2 - v0
			)) :
			glm::normalize(glm::cross(
				v2 - v0,
				v1 - v0
			));
	}

	glm::vec3 cartesian(float pitchDegs, float yawDegs, float radius = 1.0f) {
		return radius * glm::vec3(
			cos(pitchDegs) * sin(yawDegs),
			sin(pitchDegs),
			cos(pitchDegs) * cos(yawDegs)
		);
	}

	std::pair<glm::vec3, glm::vec3> bilinear(float u, float v, const glm::vec3& v00, const glm::vec3& v10, const glm::vec3& v11, const glm::vec3& v01) {

		auto a00 = (1 - u) * (1 - v);
		auto a10 = (1 - u) * v;
		auto a11 = u * v;
		auto a01 = u * (1 - v);

		glm::vec3 pos = a00 * v00
			+ a10 * v10
			+ a11 * v11
			+ a01 * v01;

		return { pos, vNormal(v00, v10, v11) };
	}
	
	ModelData planeAux(int divisions,
		glm::vec3 bl, glm::vec3 br, glm::vec3 tr, glm::vec3 tl) {

		std::vector<glm::vec3> vertices = {};
		std::vector<glm::vec3> normals = { vNormal(bl, br, tr) };
		std::vector<glm::vec2> texcoords = {};
		std::vector<unsigned int> vIndices = {};
		std::vector<unsigned int> vnIndices = {};
		std::vector<unsigned int> vtIndices = {};

		glm::vec3 normal = vNormal(bl, br, tr);
		float length = glm::distance(bl, br);
		float height = glm::distance(bl, tl);


		for (int row = 0; row <= divisions; row++) {
			for (int col = 0; col <= divisions; col++) {

				float v = float(row) / divisions;
				float u = float(col) / divisions;
				auto [pos, normal] = bilinear(u, v, bl, br, tr, tl);

				vertices.push_back(pos);
				texcoords.push_back({ v,u });

			}
		}

		for (int row = 0; row < divisions; row++) {
			for (int col = 0; col < divisions; col++) {

				auto r = glm::uvec2(row, row + 1) * glm::uvec2(divisions + 1);
				auto c = glm::uvec2(col, col + 1);

				auto v00 = r[0] + c[0], v01 = r[0] + c[1],
					 v10 = r[1] + c[0], v11 = r[1] + c[1];

				auto _tl = v00, _tr = v01,
					_bl = v10, _br = v11;

				vIndices.push_back(_tl); vnIndices.push_back(0); vtIndices.push_back(_tl);
				vIndices.push_back(_bl); vnIndices.push_back(0); vtIndices.push_back(_bl);
				vIndices.push_back(_br); vnIndices.push_back(0); vtIndices.push_back(_br);

				vIndices.push_back(_br); vnIndices.push_back(0); vtIndices.push_back(_br);
				vIndices.push_back(_tr); vnIndices.push_back(0); vtIndices.push_back(_tr);
				vIndices.push_back(_tl); vnIndices.push_back(0); vtIndices.push_back(_tl);
			}
		}

		ModelData model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}

	ModelData plane(float length, int divisions) {

		float hl = length / 2;
		glm::vec3 offset = { -hl, 0.0f, -hl };

		auto tl = offset + glm::vec3(0.0f, 0.0f, 0.0f), tr = offset + glm::vec3(length, 0.0f, 0.0f);
		auto bl = offset + glm::vec3(0.0f, 0.0f, length), br = offset + glm::vec3(length, 0.0f, length);

		return planeAux(divisions, bl, br, tr, tl);
	}

	ModelData box(float length, int divisions) {

		std::vector<glm::vec3> vertices = {};
		std::vector<glm::vec3> normals = {};
		std::vector<glm::vec2> texcoords = {};
		std::vector<unsigned int> vIndices = {};
		std::vector<unsigned int> vnIndices = {};
		std::vector<unsigned int> vtIndices = {};

		float l = length;
		float hl = length / 2;

		glm::vec3 offsetAlongX = glm::vec3(0, -hl, -hl), offsetAwayX = glm::vec3(hl, 0, 0);
		glm::vec3 offsetAlongY = glm::vec3(-hl, 0, -hl), offsetAwayY = glm::vec3(0, hl, 0);
		glm::vec3 offsetAlongZ = glm::vec3(-hl, -hl, 0), offsetAwayZ = glm::vec3(0, 0, hl);

		glm::vec3 bl, br, tr, tl;

		std::vector<ModelData> faces;

		// X faces
		bl = glm::vec3(0, 0, l);
		br = glm::vec3(0, 0, 0);
		tr = glm::vec3(0, l, 0);
		tl = glm::vec3(0, l, l);
		faces.push_back(planeAux(divisions,
			bl + offsetAlongX + offsetAwayX,
			br + offsetAlongX + offsetAwayX,
			tr + offsetAlongX + offsetAwayX,
			tl + offsetAlongX + offsetAwayX
		)); // positive (ccw)
		faces.push_back(planeAux(divisions,
			br + offsetAlongX - offsetAwayX,
			bl + offsetAlongX - offsetAwayX,
			tl + offsetAlongX - offsetAwayX,
			tr + offsetAlongX - offsetAwayX
		)); // negative (cw)


		// Y faces
		bl = glm::vec3(0, 0, l);
		br = glm::vec3(l, 0, l);
		tr = glm::vec3(l, 0, 0);
		tl = glm::vec3(0, 0, 0);
		faces.push_back(planeAux(divisions,
			bl + offsetAlongY + offsetAwayY,
			br + offsetAlongY + offsetAwayY,
			tr + offsetAlongY + offsetAwayY,
			tl + offsetAlongY + offsetAwayY
		)); // positive (ccw)
		faces.push_back(planeAux(divisions,
			br + offsetAlongY - offsetAwayY,
			bl + offsetAlongY - offsetAwayY,
			tl + offsetAlongY - offsetAwayY,
			tr + offsetAlongY - offsetAwayY
		)); // negative (cw)

		// Z faces
		bl = glm::vec3(0, 0, 0);
		br = glm::vec3(l, 0, 0);
		tr = glm::vec3(l, l, 0);
		tl = glm::vec3(0, l, 0);
		faces.push_back(planeAux(divisions,
			bl + offsetAlongZ + offsetAwayZ,
			br + offsetAlongZ + offsetAwayZ,
			tr + offsetAlongZ + offsetAwayZ,
			tl + offsetAlongZ + offsetAwayZ
		)); // positive (ccw)
		faces.push_back(planeAux(divisions,
			br + offsetAlongZ - offsetAwayZ,
			bl + offsetAlongZ - offsetAwayZ,
			tl + offsetAlongZ - offsetAwayZ,
			tr + offsetAlongZ - offsetAwayZ
		)); // negative (cw)

		size_t vOffset = 0;
		size_t nOffset = 0;

		for (const auto& face : faces) {

			// append face data into main buffers
			vertices.insert(vertices.end(), face.vertices.begin(), face.vertices.end());
			normals.insert(normals.end(), face.normals.begin(), face.normals.end());
			texcoords.insert(texcoords.end(), face.texcoords.begin(), face.texcoords.end());

			for (auto index : face.vIndices) {
				vIndices.push_back(vOffset + index);
				vnIndices.push_back(nOffset);
				vtIndices.push_back(vOffset + index);
			}

			vOffset += face.vertices.size();
			nOffset += face.normals.size();
		}

		ModelData model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}

	ModelData skybox(float length, int divisions) {

		std::vector<glm::vec3> vertices = {};
		std::vector<glm::vec3> normals = {};
		std::vector<glm::vec2> texcoords = {};
		std::vector<unsigned int> vIndices = {};
		std::vector<unsigned int> vnIndices = {};
		std::vector<unsigned int> vtIndices = {};

		float l = length;
		float hl = length / 2;

		glm::vec3 offsetAlongX = glm::vec3(0, -hl, -hl), offsetAwayX = glm::vec3(hl, 0, 0);
		glm::vec3 offsetAlongY = glm::vec3(-hl, 0, -hl), offsetAwayY = glm::vec3(0, hl, 0);
		glm::vec3 offsetAlongZ = glm::vec3(-hl, -hl, 0), offsetAwayZ = glm::vec3(0, 0, hl);

		glm::vec3 bl, br, tr, tl;

		std::vector<ModelData> faces;

		// X faces
		bl = glm::vec3(0, 0, l);
		br = glm::vec3(0, 0, 0);
		tr = glm::vec3(0, l, 0);
		tl = glm::vec3(0, l, l);
		faces.push_back(planeAux(divisions,
			bl + offsetAlongX - offsetAwayX,
			br + offsetAlongX - offsetAwayX,
			tr + offsetAlongX - offsetAwayX,
			tl + offsetAlongX - offsetAwayX
		)); // positive (ccw)
		faces.push_back(planeAux(divisions,
			br + offsetAlongX + offsetAwayX,
			bl + offsetAlongX + offsetAwayX,
			tl + offsetAlongX + offsetAwayX,
			tr + offsetAlongX + offsetAwayX
		)); // negative (cw)


		// Y faces
		bl = glm::vec3(0, 0, l);
		br = glm::vec3(l, 0, l);
		tr = glm::vec3(l, 0, 0);
		tl = glm::vec3(0, 0, 0);
		faces.push_back(planeAux(divisions,
			bl + offsetAlongY - offsetAwayY,
			br + offsetAlongY - offsetAwayY,
			tr + offsetAlongY - offsetAwayY,
			tl + offsetAlongY - offsetAwayY
		)); // positive (ccw)
		faces.push_back(planeAux(divisions,
			br + offsetAlongY + offsetAwayY,
			bl + offsetAlongY + offsetAwayY,
			tl + offsetAlongY + offsetAwayY,
			tr + offsetAlongY + offsetAwayY
		)); // negative (cw)

		// Z faces
		bl = glm::vec3(0, 0, 0);
		br = glm::vec3(l, 0, 0);
		tr = glm::vec3(l, l, 0);
		tl = glm::vec3(0, l, 0);
		faces.push_back(planeAux(divisions,
			bl + offsetAlongZ - offsetAwayZ,
			br + offsetAlongZ - offsetAwayZ,
			tr + offsetAlongZ - offsetAwayZ,
			tl + offsetAlongZ - offsetAwayZ
		)); // positive (ccw)
		faces.push_back(planeAux(divisions,
			br + offsetAlongZ + offsetAwayZ,
			bl + offsetAlongZ + offsetAwayZ,
			tl + offsetAlongZ + offsetAwayZ,
			tr + offsetAlongZ + offsetAwayZ
		)); // negative (cw)

		size_t vOffset = 0;
		size_t nOffset = 0;

		for (const auto& face : faces) {

			// append face data into main buffers
			vertices.insert(vertices.end(), face.vertices.begin(), face.vertices.end());
			normals.insert(normals.end(), face.normals.begin(), face.normals.end());
			texcoords.insert(texcoords.end(), face.texcoords.begin(), face.texcoords.end());

			for (auto index : face.vIndices) {
				vIndices.push_back(vOffset + index);
				vnIndices.push_back(nOffset);
				vtIndices.push_back(vOffset + index);
			}

			vOffset += face.vertices.size();
			nOffset += face.normals.size();
		}

		ModelData model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}
	
	ModelData sphere(float radius, int stacks, int slices) {

		std::vector<glm::vec3> vertices = {
			{0.0f, -radius, 0.0f}, // bottom
			{0.0f, radius, 0.0f} // top
		};
		std::vector<glm::vec3> normals = {
			{0.0f, -1.0f, 0.0f}, // bottom
			{0.0f, 1.0f, 0.0f} // top
		};
		std::vector<glm::vec2> texcoords = {
			{0.5f, 0.0f}, // bottom
			{0.5f, 1.0f}  // top
		};
		std::vector<unsigned int> vIndices = {};
		std::vector<unsigned int> vnIndices = {};
		std::vector<unsigned int> vtIndices = {};

		const float pitchStep = 180.0f / stacks;
		const float yawStep = 360.0f / slices;

		for (int slice = 0; slice <= slices; slice++) {
			for (int stack = 1; stack < stacks; stack++) {

				float pitch = glm::radians(-90.0f + stack * pitchStep);
				float yaw = glm::radians(slice * yawStep);

				float v = float(stack) / stacks;
				float u = float(slice) / slices;

				vertices.push_back(generateVertices::cartesian(pitch, yaw, radius));
				normals.push_back(generateVertices::cartesian(pitch, yaw, 1.0f));
				texcoords.push_back(glm::vec2(u, v));
			}
		}

		unsigned int bottom = 0;
		unsigned int top = 1;

		for (int slice = 0; slice < slices; slice++) {

			unsigned int c0 = slice * (stacks - 1); // current
			unsigned int c1 = (slice + 1) * (stacks - 1); // next

			vIndices.insert(vIndices.end(), { bottom, 2 + c1, 2 + c0 });
			vnIndices.insert(vnIndices.end(), { bottom, 2 + c1, 2 + c0 });
			vtIndices.insert(vtIndices.end(), { bottom, 2 + c1, 2 + c0 });

			for (int stack = 0; stack < stacks - 1; stack++) {

				unsigned int r0 = stack;     // ...
				unsigned int r1 = stack + 1; // ...+1

				unsigned int r0c0 = 2 + r0 + c0; //current
				unsigned int r0c1 = 2 + r0 + c1; //next
				unsigned int r1c0 = 2 + r1 + c0; //current+1
				unsigned int r1c1 = 2 + r1 + c1; //next+1

				if (stack == stacks - 2) { // Top tri
					vIndices.insert(vIndices.end(), { r0c0, r0c1, top });
					vnIndices.insert(vnIndices.end(), { r0c0, r0c1, top });
					vtIndices.insert(vtIndices.end(), { r0c0, r0c1, top });
					break;
				}

				// Two triangles per quad
				vIndices.insert(vIndices.end(), { r0c0, r0c1, r1c0,   r1c0, r0c1, r1c1 });
				vnIndices.insert(vnIndices.end(), { r0c0, r0c1, r1c0,   r1c0, r0c1, r1c1 });
				vtIndices.insert(vtIndices.end(), { r0c0, r0c1, r1c0,   r1c0, r0c1, r1c1 });
			}
		}

		ModelData model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}

	ModelData cone(float radius, float height, int slices, int stacks) {
		std::vector<glm::vec3> vertices = {
			{0.0f, 0.0f, 0.0f}, // bottom 
			{0.0f, height, 0.0f} // top
		};
		std::vector<glm::vec3> normals = {
			{0.0f, -1.0, 0.0f}, // bottom 
			{0.0f, 1.0f, 0.0f} // top
		};
		std::vector<glm::vec2> texcoords = {
			{0.5f, 0.0f}, // bottom
			{0.5f, 1.0f}  // top
		};

		std::vector<unsigned int> vIndices = {};
		std::vector<unsigned int> vnIndices = {};
		std::vector<unsigned int> vtIndices = {};

		float slope = radius / height;

		for (int slice = 0; slice <= slices; slice++) {
			for (int stack = 0; stack < stacks; stack++) {

				float v = float(stack) / stacks;
				float u = float(slice) / slices;

				float yaw = glm::radians(360.0f * u);
				float r = radius * (1.0 - v);

				vertices.push_back(glm::vec3(
					r * sinf(yaw),
					v * height,
					r * cosf(yaw)
				));
				normals.push_back(glm::normalize(glm::vec3(
					sinf(yaw),
					slope,
					cosf(yaw)
				)));
				texcoords.push_back(glm::vec2(u, v));
			}
		}

		unsigned int bottom = 0;
		unsigned int top = 1;

		// Generate side face indices
		for (int slice = 0; slice < slices; slice++) {

			unsigned int c0 = slice * stacks;
			unsigned int c1 = (slice + 1) * stacks; // next

			vIndices.insert(vIndices.end(), { bottom, 2 + c1, 2 + c0 });
			vnIndices.insert(vnIndices.end(), { bottom, bottom, bottom });
			vtIndices.insert(vtIndices.end(), { top, 2 + c1, 2 + c0 });

			for (int stack = 0; stack < stacks; stack++) {

				unsigned int r0 = stack;     // ...
				unsigned int r1 = stack + 1; // ...+1

				unsigned int r0c0 = 2 + r0 + c0; //current
				unsigned int r0c1 = 2 + r0 + c1; //next
				unsigned int r1c0 = 2 + r1 + c0; //current+1
				unsigned int r1c1 = 2 + r1 + c1; //next+1

				if (stack == stacks - 1) { // top tri

					vIndices.insert(vIndices.end(), { r0c0, r0c1, top });
					vnIndices.insert(vnIndices.end(), { r0c0, r0c1, top });
					vtIndices.insert(vtIndices.end(), { r0c0, r0c1, top });

					break;
				}

				vIndices.insert(vIndices.end(), { r0c0, r0c1, r1c0,   r1c0, r0c1, r1c1 });
				vnIndices.insert(vnIndices.end(), { r0c0, r0c1, r1c0,   r1c0, r0c1, r1c1 });
				vtIndices.insert(vtIndices.end(), { r0c0, r0c1, r1c0,   r1c0, r0c1, r1c1 });
			}
		}

		ModelData model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}

	ModelData tube(float iradius, float oradius, float height, int slices) {
		std::vector<glm::vec3> vertices = {};
		std::vector<glm::vec3> normals = {};
		std::vector<glm::vec2> texcoords = {};
		std::vector<unsigned int> vIndices = {};
		std::vector<unsigned int> vnIndices = {};
		std::vector<unsigned int> vtIndices = {};

		const float h2 = height / 2;
		const float yawStep = glm::radians(360.0f / slices);

		// Generate vertices + normals + texcoords
		for (int slice = 0; slice <= slices; slice++) {
			const float u = float(slice) / slices;
			const float yaw = slice * yawStep;
			const float sinYaw = sinf(yaw);
			const float cosYaw = cosf(yaw);

			// Outer wall
			vertices.emplace_back(oradius * sinYaw, -h2, oradius * cosYaw);
			vertices.emplace_back(oradius * sinYaw, h2, oradius * cosYaw);
			normals.emplace_back(sinYaw, 0.0f, cosYaw);
			texcoords.emplace_back(u, 0.0f);
			texcoords.emplace_back(u, 1.0f);

			// Inner wall
			vertices.emplace_back(iradius * sinYaw, -h2, iradius * cosYaw);
			vertices.emplace_back(iradius * sinYaw, h2, iradius * cosYaw);
			normals.emplace_back(-sinYaw, 0.0f, -cosYaw);
			texcoords.emplace_back(1.0f - u, 0.0f);
			texcoords.emplace_back(1.0f - u, 1.0f);

			// Top cap
			vertices.emplace_back(oradius * sinYaw, h2, oradius * cosYaw);
			vertices.emplace_back(iradius * sinYaw, h2, iradius * cosYaw);
			normals.emplace_back(0.0f, 1.0f, 0.0f);
			texcoords.emplace_back(u, 0.0f);
			texcoords.emplace_back(u, 1.0f);

			// Bottom cap
			vertices.emplace_back(oradius * sinYaw, -h2, oradius * cosYaw);
			vertices.emplace_back(iradius * sinYaw, -h2, iradius * cosYaw);
			normals.emplace_back(0.0f, -1.0f, 0.0f);
			texcoords.emplace_back(1.0f - u, 0.0f);
			texcoords.emplace_back(1.0f - u, 1.0f);
		}

		// Generate indices
		const int verticesPerSlice = 8;
		const int normalsPerSlice = 4;
		for (unsigned int slice = 0; slice < slices; slice++) {
			const unsigned int nextSlice = slice + 1;

			// Outer wall
			unsigned int ov00 = slice * verticesPerSlice;
			unsigned int ov10 = slice * verticesPerSlice + 1;
			unsigned int ov01 = (slice + 1) * verticesPerSlice;
			unsigned int ov11 = (slice + 1) * verticesPerSlice + 1;
			unsigned int ovn0 = slice * normalsPerSlice;
			unsigned int ovn1 = (slice + 1) * normalsPerSlice;

			vIndices.insert(vIndices.end(), { ov00, ov01, ov10, ov10, ov01, ov11 });
			vnIndices.insert(vnIndices.end(), { ovn0, ovn1, ovn0, ovn0, ovn1, ovn1 });
			vtIndices.insert(vtIndices.end(), { ov00, ov01, ov10, ov10, ov01, ov11 });

			// Inner wall
			unsigned int iv00 = 2 + ov00;
			unsigned int iv10 = 2 + ov10;
			unsigned int iv01 = 2 + ov01;
			unsigned int iv11 = 2 + ov11;
			unsigned int ivn0 = 1 + ovn0;
			unsigned int ivn1 = 1 + ovn1;

			vIndices.insert(vIndices.end(), { iv00, iv10, iv01, iv01, iv10, iv11 });
			vnIndices.insert(vnIndices.end(), { ivn0, ivn0, ivn1, ivn1, ivn0, ivn1 });
			vtIndices.insert(vtIndices.end(), { iv00, iv10, iv01, iv01, iv10, iv11 });

			// Top cap
			unsigned int tv00 = 2 + iv00;
			unsigned int tv10 = 2 + iv10;
			unsigned int tv01 = 2 + iv01;
			unsigned int tv11 = 2 + iv11;
			unsigned int tvn = 1 + ivn0;

			vIndices.insert(vIndices.end(), { tv00, tv01, tv10, tv10, tv01, tv11 });
			vnIndices.insert(vnIndices.end(), { tvn,  tvn,  tvn,  tvn,  tvn,  tvn });
			vtIndices.insert(vtIndices.end(), { tv00, tv01, tv10, tv10, tv01, tv11 });

			// Bottom cap
			unsigned int bv00 = 2 + tv00;
			unsigned int bv10 = 2 + tv10;
			unsigned int bv01 = 2 + tv01;
			unsigned int bv11 = 2 + tv11;
			unsigned int bvn = 1 + tvn;

			vIndices.insert(vIndices.end(), { bv00, bv10, bv01, bv01, bv10, bv11 });
			vnIndices.insert(vnIndices.end(), { bvn,  bvn,  bvn,  bvn,  bvn,  bvn });
			vtIndices.insert(vtIndices.end(), { bv00, bv10, bv01, bv01, bv10, bv11 });
		}

		ModelData model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}

	std::tuple<std::vector<std::vector<size_t>>, std::vector<glm::vec3>
	> readBezierFile(const std::string& filename, bool transpose = true) {

		std::ifstream file(fileManagement::ModelsFolder() / filename);
		if (!file.is_open()) {
			std::cerr << "Error opening file: " << filename << std::endl;
			return {};
		}

		size_t nPatches;
		file >> nPatches;

		std::vector<std::vector<size_t>> patches(nPatches, std::vector<size_t>(16));

		for (size_t i = 0; i < nPatches; ++i) {
			for (size_t j = 0; j < 16; ++j) {
				size_t index;
				file >> index;
				file.ignore();

				if (transpose) {
					size_t row = j / 4;
					size_t col = j % 4;
					size_t transposed_j = col * 4 + row;
					patches[i][transposed_j] = index;
				}
				else patches[i][j] = index;
			}
		}

		std::vector<glm::vec3> controlPoints;
		size_t nControlPoints;
		file >> nControlPoints;

		for (size_t i = 0; i < nControlPoints; ++i) {
			float x, y, z;
			file >> x; file.ignore();
			file >> y; file.ignore();
			file >> z; file.ignore();
			controlPoints.emplace_back(x, y, z);
		}

		file.close();
		return { patches, controlPoints };
	}

	ModelData bezier(const std::string& filename, const int tessellation, bool smooth = true) {

		std::vector<glm::vec3> vertices = {};
		std::vector<glm::vec3> normals = {};
		std::vector<glm::vec2> texcoords = {};
		std::vector<unsigned int> vIndices = {};
		std::vector<unsigned int> vnIndices = {};
		std::vector<unsigned int> vtIndices = {};

		auto [patches, controlPoints] = readBezierFile(filename);

		for (size_t patchIndex = 0; patchIndex < patches.size(); ++patchIndex) {

			// Load control points
			std::vector<glm::vec3> patchControlPoints;
			for (size_t cpIndex = 0; cpIndex < 16; ++cpIndex) {
				patchControlPoints.push_back(controlPoints[patches[patchIndex][cpIndex]]);
			}
			bezier2::Patch patch(patchControlPoints);

			// Generate vertices + normals
			for (int u = 0; u <= tessellation; ++u) {
				float uTess = float(u) / tessellation;

				for (int v = 0; v <= tessellation; ++v) {
					float vTess = float(v) / tessellation;

					auto [point, normal] = patch.evaluate(uTess, vTess);
					vertices.insert(vertices.end(), point);
					texcoords.insert(texcoords.end(), { 1.0f - uTess, 1.0f - vTess }); // fixes mirrored texture
					if (smooth) normals.insert(normals.end(), normal);
				}
			}

			// Generate indices
			size_t verticesPerPatch = (tessellation + 1) * (tessellation + 1);
			size_t patchOffset = vertices.size() - verticesPerPatch;

			for (int u = 0; u < tessellation; ++u) {
				glm::uvec2 uTess = glm::uvec2(u, u + 1) * glm::uvec2(tessellation + 1);
				for (int v = 0; v < tessellation; ++v) {
					glm::uvec2 vTess = glm::uvec2(v, v + 1);

					unsigned int v00 = patchOffset + uTess[0] + vTess[0], v01 = patchOffset + uTess[0] + vTess[1],
						v10 = patchOffset + uTess[1] + vTess[0], v11 = patchOffset + uTess[1] + vTess[1];

					vIndices.insert(vIndices.end(), { v00,v10,v11 });
					vIndices.insert(vIndices.end(), { v00,v11,v01 });
					vtIndices.insert(vtIndices.end(), { v00,v10,v11 });
					vtIndices.insert(vtIndices.end(), { v00,v11,v01 });

					if (smooth) {
						// one normal per vertex
						// v&vn added at same time -> same index
						vnIndices.insert(vnIndices.end(), { v00,v10,v11 });
						vnIndices.insert(vnIndices.end(), { v00,v11,v01 });

					}
					else {
						// flat shading (one normal per face)
						// find normal from cross product of verts in the face
						normals.insert(normals.end(), vNormal(
							vertices[v00],
							vertices[v10],
							vertices[v11]
						));
						vnIndices.insert(vnIndices.end(), 6, normals.size() - 1);
					}
				}
			}
		}

		ModelData model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}

};

#endif
