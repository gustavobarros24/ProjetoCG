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

	void addQuad(
		std::vector<float>& vertices,
		const glm::vec3& v1, const glm::vec3& v2,
		const glm::vec3& v3, const glm::vec3& v4,
		bool facingFront=true) {

		/********

		v1-----v4
		|  \\ 2 |
		| 1 \\  |
		v2-----v3

		********/

		if (facingFront) {
			vertices.insert(vertices.end(), {
				// First triangle (ccw)
				v1.x, v1.y, v1.z,
				v2.x, v2.y, v2.z,
				v3.x, v3.y, v3.z,
				// Second triangle (ccw)
				v1.x, v1.y, v1.z,
				v3.x, v3.y, v3.z,
				v4.x, v4.y, v4.z
				});
		}
		else {
			vertices.insert(vertices.end(), {
				// First triangle (cw)
				v1.x, v1.y, v1.z,
				v3.x, v3.y, v3.z,
				v2.x, v2.y, v2.z,
				// Second triangle (cw)
				v1.x, v1.y, v1.z,
				v4.x, v4.y, v4.z,
				v3.x, v3.y, v3.z
				});
		}
	}

	std::vector<float> plane(float length, int divisions) {
		std::vector<float> vertices;

		const float halfLength = length / 2.0f;
		const float step = length / divisions;

		for (int row = 0; row < divisions; row++) {
			float r1 = -halfLength + row * step;
			float r2 = -halfLength + (row + 1) * step;
			for (int col = 0; col < divisions; col++) {
				float c1 = -halfLength + col * step;
				float c2 = -halfLength + (col + 1) * step;
				addQuad(
					vertices,
					{ r1, 0.0f, c1 },
					{ r1, 0.0f, c2 },
					{ r2, 0.0f, c2 },
					{ r2, 0.0f, c1 },
					true
				);
			}
		}
		return vertices;
	}

	std::vector<float> box(float length, int divisions) {
		std::vector<float> vertices;
		const float halfLength = length / 2.0f;
		const float step = length / divisions;

		// Generate each face separately
		// Right face (X = +halfLength)
		for (int row = 0; row < divisions; row++) {
			float r1 = -halfLength + row * step;
			float r2 = r1 + step;
			for (int col = 0; col < divisions; col++) {
				float c1 = -halfLength + col * step;
				float c2 = c1 + step;
				addQuad(
					vertices,
					{ halfLength, r1, c1 },
					{ halfLength, r2, c1 },
					{ halfLength, r2, c2 },
					{ halfLength, r1, c2 },
					true
				);
			}
		}

		// Left face (X = -halfLength)
		for (int row = 0; row < divisions; row++) {
			float r1 = -halfLength + row * step;
			float r2 = r1 + step;
			for (int col = 0; col < divisions; col++) {
				float c1 = -halfLength + col * step;
				float c2 = c1 + step;
				addQuad(
					vertices,
					{ -halfLength, r1, c1 },
					{ -halfLength, r2, c1 },
					{ -halfLength, r2, c2 },
					{ -halfLength, r1, c2 },
					false
				);
			}
		}

		// Top face (Y = +halfLength)
		for (int row = 0; row < divisions; row++) {
			float r1 = -halfLength + row * step;
			float r2 = r1 + step;
			for (int col = 0; col < divisions; col++) {
				float c1 = -halfLength + col * step;
				float c2 = c1 + step;
				addQuad(
					vertices,
					{ r1, halfLength, c1 },
					{ r1, halfLength, c2 },
					{ r2, halfLength, c2 },
					{ r2, halfLength, c1 },
					true
				);
			}
		}

		// Bottom face (Y = -halfLength)
		for (int row = 0; row < divisions; row++) {
			float r1 = -halfLength + row * step;
			float r2 = r1 + step;
			for (int col = 0; col < divisions; col++) {
				float c1 = -halfLength + col * step;
				float c2 = c1 + step;
				addQuad(
					vertices,
					{ r1, -halfLength, c1 },
					{ r1, -halfLength, c2 },
					{ r2, -halfLength, c2 },
					{ r2, -halfLength, c1 },
					false
				);
			}
		}

		// Front face (Z = +halfLength)
		for (int row = 0; row < divisions; row++) {
			float r1 = -halfLength + row * step;
			float r2 = r1 + step;
			for (int col = 0; col < divisions; col++) {
				float c1 = -halfLength + col * step;
				float c2 = c1 + step;
				addQuad(
					vertices,
					{ r1, c1, halfLength },
					{ r2, c1, halfLength },
					{ r2, c2, halfLength },
					{ r1, c2, halfLength },
					true
				);
			}
		}

		// Back face (Z = -halfLength)
		for (int row = 0; row < divisions; row++) {
			float r1 = -halfLength + row * step;
			float r2 = r1 + step;
			for (int col = 0; col < divisions; col++) {
				float c1 = -halfLength + col * step;
				float c2 = c1 + step;
				addQuad(
					vertices,
					{ r1, c1, -halfLength },
					{ r2, c1, -halfLength },
					{ r2, c2, -halfLength },
					{ r1, c2, -halfLength },
					false
				);
			}
		}

		return vertices;
	}

	std::vector<float> sphere(float radius, int stacks, int slices) {
		std::vector<float> vertices;

		for (int stack = 0; stack < stacks; ++stack) {
			float pitch1 = M_PI / 2 - static_cast<float>(stack) / stacks * M_PI;
			float pitch2 = M_PI / 2 - static_cast<float>(stack + 1) / stacks * M_PI;

			for (int slice = 0; slice < slices; ++slice) {
				float yaw1 = static_cast<float>(slice) / slices * 2 * M_PI;
				float yaw2 = static_cast<float>(slice + 1) / slices * 2 * M_PI;

				// Compute the 4 vertices of the quad (on the sphere) using the new conversion
				glm::vec3 t1 = radius * glm::vec3(cos(pitch1) * sin(yaw1), sin(pitch1), cos(pitch1) * cos(yaw1));
				glm::vec3 t2 = radius * glm::vec3(cos(pitch1) * sin(yaw2), sin(pitch1), cos(pitch1) * cos(yaw2));
				glm::vec3 b1 = radius * glm::vec3(cos(pitch2) * sin(yaw1), sin(pitch2), cos(pitch2) * cos(yaw1));
				glm::vec3 b2 = radius * glm::vec3(cos(pitch2) * sin(yaw2), sin(pitch2), cos(pitch2) * cos(yaw2));

				if (stack != 0) {
					addQuad(
						vertices,
						{ t1.x, t1.y, t1.z },
						{ b1.x, b1.y, b1.z },
						{ b2.x, b2.y, b2.z },
						{ t2.x, t2.y, t2.z },
						true
					);

				}
				else {
					// lower tri
					vertices.insert(vertices.end(), {
						t1.x, t1.y, t1.z,
						b1.x, b1.y, b1.z,
						b2.x, b2.y, b2.z,
						});
				}
			}
		}

		return vertices;
	}

	std::vector<float> cone(float radius, float height, int slices, int stacks) {
		std::vector<float> vertices;

		// Helper function to add a quad to vertices
		auto addQuad = [&vertices](const glm::vec3& v1,
			const glm::vec3& v2,
			const glm::vec3& v3,
			const glm::vec3& v4,
			bool ccw
			) {
				if (ccw) {
					vertices.insert(vertices.end(), {
						// First triangle (ccw)
						v1.x, v1.y, v1.z,
						v2.x, v2.y, v2.z,
						v3.x, v3.y, v3.z,
						// Second triangle (ccw)
						v1.x, v1.y, v1.z,
						v3.x, v3.y, v3.z,
						v4.x, v4.y, v4.z
						});
				}
				else {
					vertices.insert(vertices.end(), {
						// First triangle (cw)
						v1.x, v1.y, v1.z,
						v3.x, v3.y, v3.z,
						v2.x, v2.y, v2.z,
						// Second triangle (cw)
						v1.x, v1.y, v1.z,
						v4.x, v4.y, v4.z,
						v3.x, v3.y, v3.z
						});
				}
			};

		const float stackHeight = height / stacks;

		for (int stack = 0; stack < stacks; stack++) {
			// bottom/top stack ycoords
			float yb = stack * stackHeight;
			float yt = (stack + 1) * stackHeight;

			// bottom/top stack radii
			float rb = radius * (1.0f - (float)stack / stacks);
			float rt = radius * (1.0f - (float)(stack + 1) / stacks);

			for (int slice = 0; slice < slices; slice++) {
				float yaw1 = static_cast<float>(slice) / slices * 2 * M_PI;
				float yaw2 = static_cast<float>(slice + 1) / slices * 2 * M_PI;

				glm::vec3 t1(rt * sinf(yaw1), yt, rt * cosf(yaw1));
				glm::vec3 t2(rt * sinf(yaw2), yt, rt * cosf(yaw2));
				glm::vec3 b1(rb * sinf(yaw1), yb, rb * cosf(yaw1));
				glm::vec3 b2(rb * sinf(yaw2), yb, rb * cosf(yaw2));

				if (stack != stacks - 1) {
					addQuad(
						{ t1.x, t1.y, t1.z },
						{ b1.x, b1.y, b1.z },
						{ b2.x, b2.y, b2.z },
						{ t2.x, t2.y, t2.z },
						true
					);
				}
				else {

					// lateral lower tri
					vertices.insert(vertices.end(), {
						b1.x, b1.y, b1.z,
						b2.x, b2.y, b2.z,
						t1.x, t1.y, t1.z,
						});

					// (cw) base slice -> achievable in a single stack
					glm::vec3 c1(radius * sinf(yaw1), 0.0f, radius * cosf(yaw1));
					glm::vec3 c2(radius * sinf(yaw2), 0.0f, radius * cosf(yaw2));

					vertices.insert(vertices.end(), {
						c1.x, c1.y, c1.z,
						0.0f, 0.0f, 0.0f,
						c2.x, c2.y, c2.z,
						});
				}
			}
		}
		return vertices;
	}

	std::vector<float> tube(float iradius, float oradius, float height, int slices) {

		std::vector<float> vertices;

		const float halfHeight = height * 0.5f;
		const float yawStep = 2.0f * M_PI / slices;

		// Generate top and bottom caps
		for (int slice = 0; slice < slices; slice++) {
			float yaw1 = slice * yawStep;
			float yaw2 = yaw1 + yawStep;

			// outer/inner top/bottom 1/2
			glm::vec3 ot1(oradius * sinf(yaw1), halfHeight, oradius * cosf(yaw1));
			glm::vec3 ot2(oradius * sinf(yaw2), halfHeight, oradius * cosf(yaw2));

			glm::vec3 it1(iradius * sinf(yaw1), halfHeight, iradius * cosf(yaw1));
			glm::vec3 it2(iradius * sinf(yaw2), halfHeight, iradius * cosf(yaw2));

			glm::vec3 ob1(oradius * sinf(yaw1), -halfHeight, oradius * cosf(yaw1));
			glm::vec3 ob2(oradius * sinf(yaw2), -halfHeight, oradius * cosf(yaw2));

			glm::vec3 ib1(iradius * sinf(yaw1), -halfHeight, iradius * cosf(yaw1));
			glm::vec3 ib2(iradius * sinf(yaw2), -halfHeight, iradius * cosf(yaw2));

			//// (ccw) top
			addQuad(
				vertices,
				{ it1.x, it1.y, it1.z },
				{ ot1.x, ot1.y, ot1.z },
				{ ot2.x, ot2.y, ot2.z },
				{ it2.x, it2.y, it2.z },
				true
			);

			//// (cw) bottom
			addQuad(
				vertices,
				{ ib1.x, ib1.y, ib1.z },
				{ ob1.x, ob1.y, ob1.z },
				{ ob2.x, ob2.y, ob2.z },
				{ ib2.x, ib2.y, ib2.z },
				false
			);

			//// (ccw) outer lateral
			addQuad(
				vertices,
				{ ot1.x, ot1.y, ot1.z },
				{ ob1.x, ob1.y, ob1.z },
				{ ob2.x, ob2.y, ob2.z },
				{ ot2.x, ot2.y, ot2.z },
				true
			);

			//// (cw) inner lateral
			addQuad(
				vertices,
				{ it1.x, it1.y, it1.z },
				{ ib1.x, ib1.y, ib1.z },
				{ ib2.x, ib2.y, ib2.z },
				{ it2.x, it2.y, it2.z },
				false
			);
		}
		return vertices;
	}

	std::vector<float> bezier(const std::string& filename, const int tessellation) {

		std::ifstream file(fileManagement::ModelsFolder() / filename);
		if (!file.is_open()) {
			std::cerr << "Error opening file: " << filename << std::endl;
			return {};
		}

		size_t nPatches;
		file >> nPatches;

		std::vector<std::vector<size_t>> patches( nPatches, std::vector<size_t>(16));

		for (size_t i = 0; i < nPatches; ++i) {
			for (size_t j = 0; j < 16; ++j) {
				size_t index;
				file >> index;
				file.ignore();
				patches[i][j] = index;
			}
		}

		std::vector<glm::vec3> controlPoints = {};
		size_t nControlPoints;
		file >> nControlPoints;

		for (size_t i = 0; i < nControlPoints; ++i) {
			float x, y, z;
			file >> x; file.ignore();
			file >> y; file.ignore();
			file >> z; file.ignore();
			controlPoints.push_back(glm::vec3(x, y, z));
		}

		file.close();

		std::vector<float> vertices = {};

		for (size_t i = 0; i < nPatches; ++i) {

			std::vector<glm::vec3> patchControlPoints;
			
			for (size_t j = 0; j < 16; ++j) {
				patchControlPoints.push_back(controlPoints[patches[i][j]]);
			}

			bezier2::Patch patch(patchControlPoints);

			for (int u = 0; u < tessellation; ++u) {
					
				float u0 = static_cast<float>(u) / tessellation;
				float u1 = static_cast<float>(u + 1) / tessellation;
	
					for (int v = 0; v < tessellation; ++v) {
	
						float v0 = static_cast<float>(v) / tessellation;
						float v1 = static_cast<float>(v + 1) / tessellation;

						glm::vec3 p00 = patch.evaluate(u0, v0).first;
						glm::vec3 p10 = patch.evaluate(u1, v0).first;
						glm::vec3 p01 = patch.evaluate(u0, v1).first;
						glm::vec3 p11 = patch.evaluate(u1, v1).first;
						
						addQuad(
							vertices,
							{ p00.x, p00.y, p00.z },
							{ p01.x, p01.y, p01.z },
							{ p11.x, p11.y, p11.z },
							{ p10.x, p10.y, p10.z }
							);
				}
			}
		}

		return vertices;
	}
};

#endif
