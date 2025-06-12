#ifndef CONFIG_H
#define CONFIG_H

#include <format>
#include <variant>
#include <unordered_map>
#include <unordered_set>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

	std::tuple<glm::mat4, glm::mat4, glm::mat4> Pxyz(std::vector<glm::vec3>& controlPoints) {

		glm::mat4 Px, Py, Pz;

		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
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
	
		void draw(int tessellation) {

			std::vector<std::vector<glm::vec3>> tessPoints;
			tessPoints.resize(tessellation + 1);

			// Draw normals + Precompute tessellated surface points
			glPushAttrib(GL_CURRENT_BIT);
			glColor3f(1, 0, 0);
			glBegin(GL_LINES);
			for (int i = 0; i <= tessellation; ++i) {
				
				float u = static_cast<float>(i) / tessellation;
				tessPoints[i].resize(tessellation + 1);

				for (int j = 0; j <= tessellation; ++j) {
				
					float v = static_cast<float>(j) / tessellation;
					auto [pos, normal] = this->evaluate(u, v);
					tessPoints[i][j] = pos;
					
					glVertex3fv(glm::value_ptr(pos));
					glVertex3fv(glm::value_ptr(pos + normal * 0.2f));
				}
			}
			glEnd();
			glPopAttrib();

			// Draw surface
			glBegin(GL_TRIANGLES);
			for (int i = 0; i < tessellation; ++i) {
				for (int j = 0; j < tessellation; ++j) {

					glVertex3fv(glm::value_ptr(tessPoints[i][j]));
					glVertex3fv(glm::value_ptr(tessPoints[i + 1][j]));
					glVertex3fv(glm::value_ptr(tessPoints[i][j + 1]));

					glVertex3fv(glm::value_ptr(tessPoints[i + 1][j]));
					glVertex3fv(glm::value_ptr(tessPoints[i + 1][j + 1]));
					glVertex3fv(glm::value_ptr(tessPoints[i][j + 1]));
				}
			}
			glEnd();
		}
	};
};

namespace catRom {

	const auto M = glm::mat4(
		-0.5f, 1.0f, -0.5f, 0.0f,
		1.5f, -2.5f, 0.0f, 1.0f,
		-1.5f, 2.0f, 0.5f, 0.0f,
		0.5f, -0.5f, 0.0f, 0.0f
	);

	glm::mat3x4 MP(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3) {
	
		const glm::mat3x4 P(
			p0.x, p1.x, p2.x, p3.x,
			p0.y, p1.y, p2.y, p3.y,
			p0.z, p1.z, p2.z, p3.z
		);

		return M * P;
	};

	std::pair<glm::vec3, glm::vec3> point(const glm::mat3x4& MP, float t) {
		glm::vec4 T(t * t * t, t * t, t, 1.0f);
		glm::vec4 dT(3 * t * t, 2 * t, 1, 0.0f);

		return { glm::vec3(T * MP), glm::vec3(dT * MP) };
	}

	void draw(int tessellationLevel, const glm::mat3x4& MP) {
	
		for (int i = 0; i < tessellationLevel+1; i++) {
			float t = i / (float)tessellationLevel;
			auto [pos, deriv] = point(MP, t);
			glVertex3f(pos.x, pos.y, pos.z);
		}
	}

	struct Spline {
		// one MP matrix per segment in the spline
		std::vector<glm::mat3x4> segmentMPs = {};

		std::pair<glm::vec3, glm::vec3> evaluate(float t) const {
			if (segmentMPs.empty()) return { glm::vec3(0), glm::vec3(0) };

			float tScaled  = t * segmentMPs.size();
			float tSegment = glm::fract(tScaled);
			int   iSegment = glm::min((int)tScaled, (int)segmentMPs.size() - 1);

			return point(segmentMPs[iSegment], tSegment);
		}

		void drawWhole(int tesselationLevel) {
			if (segmentMPs.empty()) return;

			glBegin(GL_LINE_STRIP);
			for (const auto& mp : segmentMPs) {
				draw(tesselationLevel, mp);
			}
			glEnd();
		}
	
		void transform(float t, float aligned, glm::vec3 worldUp) {
			const auto [p, dp] = evaluate(t);
			
			// translate to spline position

			glMultMatrixf(glm::value_ptr(glm::mat4(
				glm::vec4(1, 0, 0, 0),
				glm::vec4(0, 1, 0, 0),
				glm::vec4(0, 0, 1, 0),
				glm::vec4(p,       1)
			
			)));

			if (aligned) {
				auto front = glm::normalize(dp);
				auto right = glm::normalize(glm::cross(front,worldUp));
				auto up = glm::normalize(glm::cross(right, front));

				glMultMatrixf(glm::value_ptr(glm::mat4(
					glm::vec4(front,   0),
					glm::vec4(up,      0),
					glm::vec4(right,   0),
					glm::vec4(0, 0, 0, 1)
				)));

				/* alternative: Z-alignment...

				auto front = glm::normalize(dp);
				auto right = glm::normalize(glm::cross(worldUp,front));
				auto up = glm::normalize(glm::cross(front, right));

				glMultMatrixf(glm::value_ptr(glm::mat4(
					glm::vec4(right,   0),
					glm::vec4(up,      0),
					glm::vec4(front,   0),
					glm::vec4(0, 0, 0, 1)
				)));
				*/
			}
		}

		Spline() = default;

		Spline(std::vector<glm::vec3> controlPoints) {
			if (controlPoints.size() < 4) return;

			int nrSegments = controlPoints.size() - 2;
			for (int i = 0; i < nrSegments - 1; i++) {
				segmentMPs.push_back(MP(
					controlPoints[i],
					controlPoints[i + 1],
					controlPoints[i + 2],
					controlPoints[i + 3]
				));
			}
		}
	};
};

struct Window {
	int width = 512, height = 512;

	operator std::string() const {
		return std::format(
			"Window {{ width: {}, height: {} }}",
			width, height);
	}

	Window() = default;

	Window(int width, int height) :

		width(width),
		height(height)
	{}
};

struct Camera {
	glm::vec3 position = { 1, 1, 1 };
	glm::vec3 lookAt = { 0, 0, 0 };
	glm::vec3 up = { 0, 1, 0 };
	double fov = 60.0;
	double nearClip = 1.0;
	double farClip = 1000.0;

	operator std::string() const {
		return std::format(
			"Camera {{\n"
			"  position: ({:.2f}, {:.2f}, {:.2f})\n"
			"  lookAt: ({:.2f}, {:.2f}, {:.2f})\n"
			"  up: ({:.2f}, {:.2f}, {:.2f})\n"
			"  fov: {:.1f}\n"
			"  nearClip: {:.1f}\n"
			"  farClip: {:.1f}\n"
			"}}",
			position.x, position.y, position.z,
			lookAt.x, lookAt.y, lookAt.z,
			up.x, up.y, up.z,
			fov, nearClip, farClip
		);
	}
};

struct AnimatedTranslation {

	inline static const std::vector<int> tessellationLevels = { 1, 2, 4, 10 };
	inline static int currentTessIndex = tessellationLevels.size()-1;
	inline static std::string hudString = std::format("[5] Tess: {}", tessellationLevels[currentTessIndex]);
	inline static bool showPath = false;

	static inline glm::vec3 worldUp;
	std::vector<glm::vec3> controlPoints = {};
	catRom::Spline crPath;

	float t = 0.0f;
	float tPeriod = 0.0f;
	bool aligned = true;

	AnimatedTranslation(std::vector<glm::vec3> controlPoints, float tPeriod, bool isAligned) :
		tPeriod(tPeriod),
		aligned(isAligned)
	{
		this->controlPoints = controlPoints;

		// to close the loop
		(this->controlPoints).push_back(controlPoints[0]);
		(this->controlPoints).push_back(controlPoints[1]);
		(this->controlPoints).push_back(controlPoints[2]);
		
		crPath = catRom::Spline(this->controlPoints);
	}

	void drawControlPoints() {
		glPushAttrib(GL_POINT_BIT | GL_CURRENT_BIT);
		glPointSize(3.0f);
		glBegin(GL_POINTS);
		glColor3f(1, 1, 0);
		for (const auto& p : controlPoints)
			glVertex3f(p.x, p.y, p.z);
		glEnd();
		glPopAttrib();
	}

	static void nextTessellationLevel() {
		currentTessIndex = (currentTessIndex + 1) % tessellationLevels.size();
		hudString = std::format("[5] Tess: {}", tessellationLevels[currentTessIndex]);
	}

	void apply(float tDelta) {

		if (showPath) {
			drawControlPoints();
		}

		crPath.drawWhole(tessellationLevels[currentTessIndex]);
		crPath.transform(t, aligned, worldUp);

		auto tDeltaNormalised = tDelta / tPeriod;
		t = (t + tDeltaNormalised <= 1.0f) ? t + tDeltaNormalised : 0.0f;
	}

	std::string toString() const {
		std::string pointsStr;
		for (const auto& point : controlPoints) {
			pointsStr += std::format("({:.2f}, {:.2f}, {:.2f}) ", point.x, point.y, point.z);
		}
		if (!pointsStr.empty()) {
			pointsStr.pop_back();
		}

		return std::format(
			"AnimatedTranslation {{\n"
			"  Control Points: [{}]\n"
			"  World Up: ({:.2f}, {:.2f}, {:.2f})\n"
			"  Current t: {:.2f}\n"
			"  Period: {:.2f}\n"
			"  Aligned: {}\n"
			"  Show Path: {}\n"
			"}}",
			pointsStr,
			worldUp.x, worldUp.y, worldUp.z,
			t,
			tPeriod,
			aligned ? "true" : "false",
			showPath ? "true" : "false"
		);
	}
};

struct Translation {
	const float x, y, z;

	Translation(float x, float y, float z) : x(x), y(y), z(z) {}

	void apply() {
		glTranslatef(x, y, z);
	}

	std::string toString() const {
		return std::format("Translation({}, {}, {})", x, y, z);
	}
};

struct AnimatedRotation {

	glm::vec3 axis = {0.0f,1.0f,0.0f};
	float t = 0.0f;
	float tPeriod = 0.0f;

	AnimatedRotation(glm::vec3 axis, float tPeriod) : axis(axis), tPeriod(tPeriod) {}

	void apply(float tDelta) {

		glRotatef(360.0f * t, axis.x, axis.y, axis.z);

		auto tDeltaNormalised = tDelta / tPeriod;
		t = (t + tDeltaNormalised <= 1.0f) ? t + tDeltaNormalised : 0.0f;
		
	}
	
	std::string toString() const {
		return std::format(
			"AnimatedRotation {{\n"
			"  Axis: ({:.2f}, {:.2f}, {:.2f})\n"
			"  Period: {:.2f}\n"
			"}}",
			axis.x, axis.y, axis.z,
			tPeriod
		);
	}
};

struct Rotation {
	const float angle, x, y, z;

	Rotation(float angle, float x, float y, float z) : angle(angle), x(x), y(y), z(z) {}

	void apply() {
		glRotatef(angle, x, y, z);
	}

	std::string toString() const {
		return std::format("Rotation(angle: {}, axis: ({}, {}, {}))", angle, x, y, z);
	}
};

struct Scaling {
	const float x, y, z;

	Scaling(float x, float y, float z) : x(x), y(y), z(z) {}

	void apply() {
		glScalef(x, y, z);
	}

	std::string toString() const {
		return std::format("Scaling({}, {}, {})", x, y, z);
	}
};

using Transform = std::variant<
	AnimatedTranslation,
	Translation,
	AnimatedRotation,
	Rotation,
	Scaling
>;

std::string TransformToString(Transform transform) {

	struct TransformToString {
		std::string operator()(const Translation& t) const { return t.toString(); }
		std::string operator()(const Rotation& r) const { return r.toString(); }
		std::string operator()(const Scaling& s) const { return s.toString(); }
		std::string operator()(const AnimatedTranslation& at) const { return at.toString(); }
		std::string operator()(const AnimatedRotation& ar) const { return ar.toString(); }
	};

	return std::visit(TransformToString{}, transform);
}

struct Model {
	inline static bool showAxes = false;

	std::string filename;
	std::vector<float> vertices;
	unsigned int vertexCount = 0;
	unsigned int vboID = 0;

	static void toggleAxes() {
		Model::showAxes = !Model::showAxes;
	}

	void drawAxes() const  {

		/*
		// cartesian
		auto arrowedAxis = [](float length, bool x, bool y, bool z) -> void {
			glPushAttrib(GL_CURRENT_BIT);
			glColor3f(x, y, z);
			
			glBegin(GL_LINES);
			glVertex3f(0, 0, 0);
			glVertex3f(length * x, length * y, length * z);
			glEnd();

			glPushMatrix();
			glTranslatef(length * x, length * y, length * z);
			glRotatef(y * 90.0f, -1, 0, 0); //pitch
			glRotatef(x * 90.0f, 0, 1, 0); //yaw
			glutSolidCone(0.05, 0.3, 10, 1);
			glPopMatrix();

			glPopAttrib();
			};

		arrowedAxis(2.0, 1, 0, 0);
		arrowedAxis(2.0, 0, 1, 0);
		arrowedAxis(2.0, 0, 0, 1);
		*/

		glPushAttrib(GL_CURRENT_BIT);
		glBegin(GL_LINES);

		glColor3f(1.0f, 0.0f, 0.0f);  // Red
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(1.3f, 0.0f, 0.0f);

		glColor3f(0.0f, 1.0f, 0.0f);  // Green
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 1.3f, 0.0f);

		glColor3f(0.0f, 0.0f, 1.0f);  // Blue
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 1.3f);

		glEnd();
		glPopAttrib();
		
		// polars

		static const float radius = 1.3f;
		static const int slices = 20;
		static constexpr float step = glm::radians(360.0) / slices;
		
		glPushAttrib(GL_CURRENT_BIT);

		
		// for yaw (in XZ plane)
		glColor3f(1.0f, 0.0f, 1.0f);
		glBegin(GL_LINE_LOOP);

		for (int i = 0; i < slices; i++) {
			float yaw = i * step;
			float x = radius * cos(yaw);
			float z = radius * sin(yaw);
			glVertex3f(x, 0.0f, z);
		}
		glEnd();

		// for pitch (in ZY plane)
		glBegin(GL_LINE_LOOP);

		for (int i = 0; i < slices; i++) {
			float pitch = i * step;
			float y = radius * sin(pitch);
			float z = radius * cos(pitch);
			glVertex3f(0.0f, y, z);
		}

		glEnd();
		glPopAttrib();
	}

	void drawVBO() const {

		if (showAxes) drawAxes();

		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, vertexCount);

		glDisableClientState(GL_VERTEX_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

	}

	void drawImmediate() const {

		if (showAxes) drawAxes();

		glBegin(GL_TRIANGLES);

		for (size_t i = 0; i < vertices.size(); i += 3) {
			glVertex3f(
				vertices[i],
				vertices[i + 1],
				vertices[i + 2]
			);
		}

		glEnd();
	}

	void initializeVBO() {
		glGenBuffers(1, &vboID);
		glBindBuffer(GL_ARRAY_BUFFER, vboID);
		glBufferData(
			GL_ARRAY_BUFFER,
			vertices.size() * sizeof(float),
			vertices.data(),
			GL_STATIC_DRAW
		);
		vertexCount = vertices.size() / 3;
	}

	void deleteVBO() {
		if (vboID != 0) {
			glDeleteBuffers(1, &vboID);
			vboID = 0;
		}
	}

	Model(const std::string filename) : filename(filename) {

		vertices = modelFileManagement::loadModelVertices(filename);

	}

	std::string toString() const {
		return std::format(
			"Model: {}\n"
			"  Vertex count: {}\n"
			"  VBO ID: {}\n"
			"  Memory size: {:.2f} KB",
			filename,
			vertices.size() / 3,
			vboID,
			(vertices.size() * sizeof(float)) / 1024.0f);
	}

};

struct ModelStorage {

	std::unordered_map<std::string, Model> byFilename = {};

	const Model& get(const std::string& filename) {
		return byFilename.at(filename);
	}

	bool contains(const std::string& filename) const {
		return byFilename.find(filename) != byFilename.end();
	}

	void tryLoad(std::string& filename) {
		if (!contains(filename)) {
			byFilename.try_emplace(filename, std::move(Model(filename)));
		}
	}

	void drawAll(bool withVBO, std::vector<std::string> filenames) {
		
		if (withVBO) {
			for (auto& f : filenames) {
				if (contains(f)) {
					auto model = get(f);
					model.drawVBO();
				}
			}
		}
		else {
			for (auto& f : filenames) {
				if (contains(f)) {
					auto model = get(f);
					model.drawImmediate();
				}
			}
		}
	}

	void initializeAllVBOs() {
		for (auto& [filename, model] : byFilename) {
			model.initializeVBO();
		}
	}

	void cleanupAllVBOs() {
		for (auto& [filename, model] : byFilename) {
			model.deleteVBO();
		}
	}

	std::string toString() const {
		size_t totalVertices = 0;
		float totalMemory = 0.0f;

		for (const auto& [filename, model] : byFilename) {
			totalVertices += model.vertices.size() / 3;
			totalMemory += (model.vertices.size() * sizeof(float)) / 1024.0f;
		}

		std::string result = std::format(
			"ModelStorage contains {} models:\n"
			"  Total vertices: {}\n"
			"  Total memory: {:.2f} KB\n\n",
			byFilename.size(),
			totalVertices,
			totalMemory
		);

		for (const auto& [filename, model] : byFilename) {
			result += model.toString() + "\n\n";
		}

		return result;
	}

	ModelStorage() = default;

	ModelStorage(std::vector<std::string> modelFilenames) {
		for (auto& filename : modelFilenames) {
			tryLoad(filename);
		}
	}
};

struct Group {
	std::optional<glm::vec3> colour = std::nullopt;
	std::vector<Transform> transforms = {};
	std::vector<std::string> modelFilenames = {};
	std::vector<Group> subgroups = {};

	inline static std::vector<Transform> debugTransforms = {
		/*
		AnimatedRotation{
			glm::vec3(0,1,0),
			10.0f
		},*/

		AnimatedTranslation{
			{
				glm::vec3(0, 0, -1),
				glm::vec3(1, 0,  1),
				glm::vec3(2, 0, -1),
				glm::vec3(3, 0,  1),
				glm::vec3(4, 0, -1),
				glm::vec3(5, 0,  1),
				glm::vec3(6, 0, -1),
				glm::vec3(7, 0,  1)
			},
			10.0f, true
		}
		
	};

	void applyTransforms(float tDelta) {

		struct TransformMapping {
			float tDelta;

			TransformMapping(float delta) : tDelta(delta) {}

			void operator()(Translation& t)          { t.apply(); }
			void operator()(Rotation& r)             { r.apply(); }
			void operator()(Scaling& s)              { s.apply(); }
			void operator()(AnimatedTranslation& at) { at.apply(tDelta); }
			void operator()(AnimatedRotation& ar)    { ar.apply(tDelta); }
		};

		for (auto& t : transforms)
			std::visit(TransformMapping(tDelta), t);
	}

	void render(bool withVBO, ModelStorage& models, float tDelta) {

		glPushMatrix();
		glPushAttrib(GL_CURRENT_BIT);

		applyTransforms(tDelta);

		if (colour.has_value())
			glColor3f(colour.value().r, colour.value().g, colour.value().b);

		models.drawAll(withVBO, modelFilenames);

		for (auto& subgroup : subgroups)
			subgroup.render(withVBO, models, tDelta);

		glPopAttrib();
		glPopMatrix();
	}

	operator std::string() const {
		std::string result = "Group {\n";

		// Colour information
		if (colour.has_value()) {
			const auto& c = colour.value();
			result += std::format("  Colour: ({:.2f}, {:.2f}, {:.2f})\n", c.r, c.g, c.b);
		}
		else {
			result += "  Colour: none\n";
		}

		// Transforms
		result += "  Transforms: [\n";
		for (const auto& transform : transforms) {
			result += "    " + TransformToString(transform) + "\n";
		}
		result += "  ]\n";

		// Model filenames
		result += "  Models: [\n";
		for (const auto& filename : modelFilenames) {
			result += "    " + filename + "\n";
		}
		result += "  ]\n";

		// Subgroups
		result += std::format("  Subgroups: {} [\n", subgroups.size());
		for (const auto& subgroup : subgroups) {
			// Indent each line of the subgroup's string representation
			std::string subgroupStr = static_cast<std::string>(subgroup);
			size_t pos = 0;
			while ((pos = subgroupStr.find('\n', pos)) != std::string::npos) {
				subgroupStr.insert(pos + 1, "    ");
				pos += 5;
			}
			result += "    " + subgroupStr + "\n";
		}
		result += "  ]\n";

		result += "}";
		return result;
	}

};

struct World {

	Window window;
	Camera camera;
	ModelStorage modelStorage;
	std::vector<Group> groups = {};

	void renderGroups(bool withVBO, float tDelta) {

		for (auto& g : groups)
			g.render(withVBO, modelStorage, tDelta);
	}

	operator std::string() const {
		std::string result = "World {\n";

		// Window information
		result += "  Window: " + static_cast<std::string>(window) + "\n\n";

		// Camera information
		result += "  Camera: " + static_cast<std::string>(camera) + "\n\n";

		// Model storage information
		result += "  " + modelStorage.toString() + "\n";

		// Groups information
		result += std::format("  Groups: {} [\n", groups.size());
		for (const auto& group : groups) {
			// Indent each line of the group's string representation
			std::string groupStr = static_cast<std::string>(group);
			size_t pos = 0;
			while ((pos = groupStr.find('\n', pos)) != std::string::npos) {
				groupStr.insert(pos + 1, "    ");
				pos += 5;
			}
			result += "    " + groupStr + "\n\n";
		}
		result += "  ]\n";

		result += "}";
		return result;
	}
};

#endif