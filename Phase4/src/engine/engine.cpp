#include <array>
#include <cmath>

#include <GL/glew.h>
#include <IL/il.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../engine/Parsing.h"

World world;


std::vector<glm::vec3> debugControlGrid = {
	// Row 0
	glm::vec3(-1.5f, 0.0f, -1.5f),
	glm::vec3(-0.5f, 0.5f, -1.5f),
	glm::vec3(0.5f, 0.5f, -1.5f),
	glm::vec3(1.5f, 0.0f, -1.5f),

	// Row 1
	glm::vec3(-1.5f, 0.5f, -0.5f),
	glm::vec3(-0.5f, 1.0f, -0.5f),
	glm::vec3(0.5f, 1.0f, -0.5f),
	glm::vec3(1.5f, 0.5f, -0.5f),

	// Row 2
	glm::vec3(-1.5f, 0.5f, 0.5f),
	glm::vec3(-0.5f, 1.0f, 0.5f),
	glm::vec3(0.5f, 1.0f, 0.5f),
	glm::vec3(1.5f, 0.5f, 0.5f),

	// Row 3
	glm::vec3(-1.5f, 0.0f, 1.5f),
	glm::vec3(-0.5f, 0.5f, 1.5f),
	glm::vec3(0.5f, 0.5f, 1.5f),
	glm::vec3(1.5f, 0.0f, 1.5f)
};

auto debugPatch = bezier2::Patch(debugControlGrid);

namespace genVerts {

	/*
	glm::vec3 vNormal(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, bool ccw=true) {
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
	*/

	glm::vec3 vNormal(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, bool ccw = true) {
		glm::vec3 cross = (ccw) ? glm::cross(v1 - v0, v2 - v0) : glm::cross(v2 - v0, v1 - v0);
		
		float length = glm::length(cross);

		// threshold for zero-length vectors
		if (length < 1e-6f) 
			return glm::vec3(0.0f, 1.0f, 0.0f);
		
		return cross / length;
	}

	glm::vec3 cartesian(float pitchDegs, float yawDegs, float radius = 1.0f) {
		return radius * glm::vec3(
			cos(pitchDegs) * sin(yawDegs),
			sin(pitchDegs),
			cos(pitchDegs) * cos(yawDegs)
		);
	}

	std::pair<glm::vec3, glm::vec3> bilinear(float u, float v,
		const glm::vec3& v00, const glm::vec3& v10,
		const glm::vec3& v11, const glm::vec3& v01) {

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

	Model planeAux(int divisions,
		glm::vec3 bl, glm::vec3 br, glm::vec3 tr, glm::vec3 tl,
		bool ccw = true) {

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

				float u = float(col) / divisions;
				float v = float(row) / divisions;
				auto [pos, normal] = bilinear(u, v, bl, br, tr, tl);

				vertices.push_back(pos);
				texcoords.push_back({ v,u });
				//texcoords.push_back({ u,v });

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

		Model model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}

	Model plane(float length, int divisions) {

		float hl = length / 2;
		glm::vec3 offset = { -hl, 0.0f, -hl };

		auto tl = offset + glm::vec3(0.0f, 0.0f, 0.0f);
		auto tr = offset + glm::vec3(length, 0.0f, 0.0f);
		auto bl = offset + glm::vec3(0.0f, 0.0f, length);
		auto br = offset + glm::vec3(length, 0.0f, length);

		return planeAux(divisions, bl, br, tr, tl);
	}

	Model box(float length, int divisions) {

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

		std::vector<Model> faces;

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

		Model model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}

	Model skybox(float length, int divisions) {

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

		std::vector<Model> faces;

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

		Model model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}

	
	Model sphere(float radius, int stacks, int slices) {

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

				vertices.push_back(cartesian(pitch, yaw, radius));
				normals.push_back(cartesian(pitch, yaw, 1.0f));
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

		Model model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}

	Model cone(float radius, float height, int slices, int stacks) {
		std::vector<glm::vec3> vertices = {
			{0.0f, 0.0f, 0.0f}, // bottom 
			{0.0f, height, 0.0f} // top
		};
		std::vector<glm::vec3> normals = {
			{0.0f, -1.0, 0.0f}, // bottom 
			{0.0f, 1.0f, 0.0f} // top
		};
		std::vector<glm::vec2> texcoords = {
			{0.5f, 1.0f}, // bottom
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

			vIndices.insert(vIndices.end(),   { bottom, 2 + c1, 2 + c0 });
			vnIndices.insert(vnIndices.end(), { bottom, bottom, bottom });
			vtIndices.insert(vtIndices.end(), { bottom, 2 + c1, 2 + c0 });

			for (int stack = 0; stack < stacks; stack++) {

				unsigned int r0 = stack;     // ...
				unsigned int r1 = stack + 1; // ...+1

				unsigned int r0c0 = 2 + r0 + c0; //current
				unsigned int r0c1 = 2 + r0 + c1; //next
				unsigned int r1c0 = 2 + r1 + c0; //current+1
				unsigned int r1c1 = 2 + r1 + c1; //next+1

				
				if (stack == stacks - 1) { // top tri
					vIndices.insert(vIndices.end(),   { r0c0, r0c1, top });
					vnIndices.insert(vnIndices.end(), { r0c0, r0c1, top });
					vtIndices.insert(vtIndices.end(), { r0c0, r0c1, top });

					break;
				}

				vIndices.insert(vIndices.end(),   { r0c0, r0c1, r1c0,   r1c0, r0c1, r1c1 });
				vnIndices.insert(vnIndices.end(), { r0c0, r0c1, r1c0,   r1c0, r0c1, r1c1 });
				vtIndices.insert(vtIndices.end(), { r0c0, r0c1, r1c0,   r1c0, r0c1, r1c1 });
			}
		}

		Model model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}

	Model tube(float iradius, float oradius, float height, int slices) {
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

		Model model;
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

		std::ifstream file(modelFileManagement::ModelsFolder() / filename);
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

	Model bezier(const std::string& filename, const int tessellation, bool smooth = true) {

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

		Model model;
		model.vertices = vertices;
		model.normals = normals;
		model.texcoords = texcoords;
		model.vIndices = vIndices;
		model.vnIndices = vnIndices;
		model.vtIndices = vtIndices;
		return model;
	}

};

//auto teapot = modelFileManagement::importOBJ("myteapot.3d");
//auto sphere = modelFileManagement::importOBJ("mysphere.3d");
//auto cone =   modelFileManagement::importOBJ("mycone.3d");
//auto tube =   modelFileManagement::importOBJ("mytube.3d");
//auto plane =  modelFileManagement::importOBJ("myplane.3d");
//auto box =    modelFileManagement::importOBJ("mybox.3d");

//auto plane = genVerts::plane(3,3);
//auto box = genVerts::box(3, 3);
//auto skybox = genVerts::skybox(700, 3);
//auto cone = genVerts::cone(3,3,30,30);



namespace keybinds { void update(float deltaTime); };

namespace render {

	namespace polygonMode {

		const GLenum modes[] = { GL_FILL, GL_LINE, GL_POINT };
		const std::string str[] = { "Fill", "Line", "Point" };
		int current = 0;

		void next() {
			current = (current + 1) % 3;
			glPolygonMode(GL_FRONT_AND_BACK, modes[current]);
		}
	}
	
	namespace axes {

		bool enabled = false;

		void toggle() {
			enabled = !enabled;
			AnimatedTranslation::showPath = false;
		}

		void show() {
			glPushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT);
			glDisable(GL_LIGHTING);
			glBegin(GL_LINES);

			glColor3f(1.0f, 0.0f, 0.0f);  // Red (X+)
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(1000.0f, 0.0f, 0.0f);

			glColor3f(0.2f, 0.0f, 0.0f);  // Red (X-)
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(-1000.0f, 0.0f, 0.0f);

			glColor3f(0.0f, 1.0f, 0.0f);  // Green (Y+)
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 1000.0f, 0.0f);

			glColor3f(0.0f, 0.2f, 0.0f);  // Green (Y-)
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, -1000.0f, 0.0f);

			glColor3f(0.0f, 0.0f, 1.0f);  // Blue (Z+)
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, 1000.0f);

			glColor3f(0.0f, 0.0f, 0.2f);  // Blue (Z-)
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, -1000.0f);

			glEnd();
			glPopAttrib();

			AnimatedTranslation::showPath = true;
		}

	};

	namespace faceCull {
		const std::string str[] = { "None", "Back", "Front" };
		int current = 0;

		void next() {
			current = (current + 1) % 3;

			switch (current) {
			case 0:
				glDisable(GL_CULL_FACE);
				break;

			case 1:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;

			case 2:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
			}
		}
	};

	namespace textureFilter {
		const GLenum mode[] = { GL_NEAREST, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR };
		const std::string str[] = { "Nearest", "Bilinear", "Trilinear", "Trilinear (Anisotropic)" };
		int current = 0;

		void next() {
			current = (current + 1) % 4;
			Texture::setFilter(mode[current]);
			Texture::setAnisotropy(current == 3);
		}
	}

	namespace lighting {
		bool enabled = false;

		void toggle() {
			enabled = !enabled;
			if (enabled) glEnable(GL_LIGHTING); else glDisable(GL_LIGHTING);
		}
	}
	
	namespace clock {
	
		float currentTime = 0.0f;
		float lastTime = 0.0f;
		float deltaTime = 0.0f; // time between frames

		std::string hudString = std::format("Elapsed: {:.1f}s", currentTime/1000.0f);

		void update() {
			lastTime = currentTime;
			currentTime = glutGet(GLUT_ELAPSED_TIME);
			deltaTime = (currentTime - lastTime) / 1000.0f;
			hudString = std::format("Elapsed: {:.1f}s", currentTime / 1000.0f);
		}
	}

	namespace framesPerSecond {

		float lastSampleTime = 0.0f;
		float frameCount = 0.0f;
		float fps;

		std::string hudString = "Frames/s: 0";

		void update(float currentTime, float samplingPeriod = 100.0f) {

			frameCount++;
			if (currentTime - lastSampleTime > samplingPeriod) {
				fps = frameCount * 1000.0f / (currentTime - lastSampleTime);
				lastSampleTime = currentTime;
				frameCount = 0;
				hudString = std::format("Frames/s: {}", static_cast<int>(fps));
				//glutSetWindowTitle(hudString.c_str());
			}
		}
	};
	
	namespace hud {

		void show() {

			const double windowWidth = glutGet(GLUT_WINDOW_WIDTH);
			const double windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

			glPushAttrib(GL_CURRENT_BIT | GL_LIGHTING_BIT | GL_DEPTH_BUFFER_BIT);
			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			
			glMatrixMode(GL_PROJECTION); glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(0, windowWidth, 0, windowHeight);

			glMatrixMode(GL_MODELVIEW); glPushMatrix();
			glLoadIdentity();
			

			const std::vector<std::string> stats = {
				framesPerSecond::hudString,
				clock::hudString
			};

			const std::vector<std::string> settings = {
				std::format("[1] Polygons: {}",  polygonMode::str[polygonMode::current]),
				std::format("[2] Axes: {}",      (axes::enabled) ? "On" : "Off"),
				std::format("[3] Face Cull: {}", faceCull::str[faceCull::current]),
				std::format("[4] Lighting: {}",  (lighting::enabled) ? "On" : "Off"),
				std::format("[5] Filtering: {}", textureFilter::str[textureFilter::current]),
				std::format("[0] Fullscreen")
			};

			float y = windowHeight - 20.0f;

			glColor3f(1.0f, 1.0f, 1.0f);
			for (const auto& line : stats) {
				glRasterPos2f(10, y);
				for (char c : line) {
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
				}
				y -= 15.0f;
			}
			y -= 10.0f;

			glColor3f(1.0f, 1.0f, 0.0f);
			for (const auto& line : settings) {
				glRasterPos2f(10, y);
				for (char c : line) {
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
				}
				y -= 15.0f;
			}

			glMatrixMode(GL_PROJECTION); glPopMatrix();
			glMatrixMode(GL_MODELVIEW); glPopMatrix();

			glPopAttrib();
		}
	};

	void renderScene(void) {
		clock::update();
		keybinds::update(clock::deltaTime);
		framesPerSecond::update(clock::currentTime, 100.0f);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();
		
		CameraController::lookAt();
		if (axes::enabled) axes::show();
		LightCaster::applyAll();

		glColor3f(1.0f, 1.0f, 1.0f);
		world.renderGroups(clock::deltaTime);
		//glTranslatef(10.0f, 0, 0);
		//debugPatch.draw(20);

		//glPushMatrix();
		//teapot.draw(Texture::id("earth.jpg"));glTranslatef(10.0f, 0, 0);
		//tube  .draw(Texture::id("earth.jpg"));glTranslatef(10.0f, 0, 0);
		//sphere.draw(Texture::id("earth.jpg"));glTranslatef(10.0f, 0, 0);
		//cone  .draw(Texture::id("earth.jpg"));glTranslatef(10.0f, 0, 0);
		//plane .draw(Texture::id("earth.jpg"));glTranslatef(10.0f, 0, 0);
		//skybox   .draw(Texture::id("earth.jpg"));
		//glPopMatrix();

		hud::show();
		glutSwapBuffers();
	}
};

namespace keybinds {

	std::unordered_set<unsigned char> keysPressed;
	std::unordered_set<int> specialKeysPressed;

	std::unordered_set<unsigned char> toggleKeys = {
		'1','2','3','4','5','0',
		
		'c','C',
	};

	void keyboardSpecialUp(int key_code, int x, int y) {
		specialKeysPressed.erase(key_code);
	}

	void keyboardUp(unsigned char key, int x, int y) {
		keysPressed.erase(key);
	}

	void keyboardSpecial(int key_code, int x, int y) {
		specialKeysPressed.insert(key_code);
	}

	void keyboard(unsigned char key, int x, int y) {
		if (!toggleKeys.contains(key))
			keysPressed.insert(key);

		switch (key) {
		
		case '1': 
			render::polygonMode::next();
			break;
		case '2': 
			render::axes::toggle();
			Model::toggleAxes();
			LightCaster::toggleLocations();
			AnimatedTranslation::showPath = !AnimatedTranslation::showPath;
			break;
		case '3':
			render::faceCull::next();
			break;
		case '4':
			render::lighting::toggle();
			break;
		case '5':
			render::textureFilter::next();
			break;
		case '0':
			WindowState::toggleFullscreen();
			break;
		case 'c':
		case 'C':
			CameraController::toggleMode();
			break;
		}
	}

	void update(float deltaTime) {
		for (unsigned char key : keysPressed) {
			CameraController::handleKey(key, deltaTime);
		}
		for (int key_code : specialKeysPressed) {
			CameraController::handleKeySpecial(key_code, deltaTime);
		}
	}
};

int main(int argc, char** argv) {	

	world = configParser::loadWorld("config.xml");
	
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WindowState::initialWidth, WindowState::initialHeight);
	glutCreateWindow("Engine");
	glutReshapeFunc(WindowState::changeSize);

	// Initialize GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	configParser::importModels();
	configParser::importTextures();

	Texture::print();
	
	atexit([]() { ModelStorage::cleanupBuffers(); });

	glutIdleFunc(render::renderScene);
	glutDisplayFunc(render::renderScene);

	glutKeyboardFunc(keybinds::keyboard);
	glutKeyboardUpFunc(keybinds::keyboardUp);
	glutSpecialFunc(keybinds::keyboardSpecial);
	glutSpecialUpFunc(keybinds::keyboardSpecialUp);

	glEnable(GL_RESCALE_NORMAL);
	glEnable(GL_DEPTH_TEST);
	
	// render settings initialisation
	render::faceCull::next();
	render::lighting::toggle();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glutMainLoop();

	return 1;
}