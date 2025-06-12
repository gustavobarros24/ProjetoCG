#ifndef CONFIG_H
#define CONFIG_H

#include <format>
#include <variant>
#include <unordered_map>
#include <unordered_set>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>



struct CameraController {
	enum class Behaviour { FREEROAM, ORBITAL };

	struct Placement {
		glm::vec3 pos = { 1, 1, 1 };
		glm::vec3 target = { 0, 0, 0 };
		glm::vec3 up = { 0, 1, 0 };

		glm::vec3 front() const {
			return glm::normalize(target - pos);
		}

		std::pair<float, float> yaw_pitch() const {
			glm::vec3 f = front();
			float yaw = atan2(f.x, f.z);
			float pitch = asin(f.y);
			return { yaw, pitch };
		}

		Placement translate(glm::vec3 v) const {
			return { pos + v, target + v, up };
		}

		Placement rotate(float yawDegs, float pitchDegs, glm::vec3 worldUp) const {

			float dYaw = glm::radians(yawDegs);
			float dPitch = glm::radians(pitchDegs);
			auto [yaw, pitch] = yaw_pitch();

			yaw = yaw + dYaw;
			pitch = glm::clamp(
				pitch + dPitch,
				glm::radians(-89.99f),
				glm::radians(89.99f)
			);

			// new front, right, up
			auto f = glm::normalize(glm::vec3(
				cos(pitch) * sin(yaw),
				sin(pitch),
				cos(pitch) * cos(yaw)
			));
			auto r = glm::normalize(glm::cross(f, worldUp));
			auto u = glm::normalize(glm::cross(r, f));

			return { pos, pos + f, u };
		}

		Placement revolve(float yawDegs, float pitchDegs, glm::vec3 center, glm::vec3 worldUp) const {

			glm::vec3 toCamera = pos - center;
			float radius = std::max(1.0f, glm::length(toCamera));

			float dYaw = glm::radians(yawDegs);
			float dPitch = glm::radians(pitchDegs);
			auto [yaw, pitch] = yaw_pitch();

			yaw = yaw + dYaw;
			pitch = glm::clamp(
				pitch + dPitch,
				glm::radians(-89.99f),
				glm::radians(89.99f)
			);

			auto f = glm::normalize(glm::vec3(
				cos(pitch) * sin(yaw),
				sin(pitch),
				cos(pitch) * cos(yaw)
			));

			// center-f because f directs from pos to center
			return { center - f * radius, center, worldUp };
		}

		Placement zoomIn(float units, glm::vec3 center, glm::vec3 worldUp) const {

			glm::vec3 toCamera = pos - center;
			float radius = std::max(1.0f, glm::length(toCamera) - units);

			auto f = -glm::normalize(toCamera);

			return { center - f * radius, center, worldUp };
		}
	};

	struct Projection {
		double fov = 60.0;
		double near = 1.0;
		double far = 1000.0;
	};


	static inline Placement initialPlacement = {};
	static inline Projection initialProjection = {};

	static inline Placement currentPlacement = initialPlacement;
	static inline Projection currentProjection = initialProjection;
	static inline Behaviour currentBehaviour = Behaviour::FREEROAM;


	static bool inFreeroam() {
		return currentBehaviour == Behaviour::FREEROAM;
	}

	struct Freeroam {

		static inline float FR_rotSpeed = 200.0f; // degrees per second
		static inline float FR_movSpeed = 100.0f;

		static Placement rotate(float yawDegs, float pitchDegs, float deltaTime) {
			//Placement old = currentPlacement;
			return currentPlacement.rotate(
				yawDegs * FR_rotSpeed * deltaTime,
				pitchDegs * FR_rotSpeed * deltaTime,
				initialPlacement.up
			);
		}

		static Placement moveForward(float deltaTime) {
			return currentPlacement.translate(currentPlacement.front() * FR_movSpeed * deltaTime);
		}

		static Placement moveBackward(float deltaTime) {
			return currentPlacement.translate(-currentPlacement.front() * FR_movSpeed * deltaTime);
		}

		static Placement moveRight(float deltaTime) {
			auto right = right_up(currentPlacement.front(), initialPlacement.up).first;
			return currentPlacement.translate(right * FR_movSpeed * deltaTime);
		}

		static Placement moveLeft(float deltaTime) {
			auto right = right_up(currentPlacement.front(), initialPlacement.up).first;
			return currentPlacement.translate(-right * FR_movSpeed * deltaTime);
		}

		static Placement moveUp(float deltaTime) {
			return currentPlacement.translate(initialPlacement.up * FR_movSpeed * deltaTime);
		}

		static Placement moveDown(float deltaTime) {
			return currentPlacement.translate(-initialPlacement.up * FR_movSpeed * deltaTime);
		}
	};

	struct Orbital {

		static inline float OB_rotSpeed = 200.0f; // degrees per second
		static inline float OB_zoomSpeed = 100.0f;

		static Placement zoomIn(float deltaTime) {
			return currentPlacement.zoomIn(OB_zoomSpeed * deltaTime, initialPlacement.target, initialPlacement.up);
		}

		static Placement zoomOut(float deltaTime) {
			return currentPlacement.zoomIn(-OB_zoomSpeed * deltaTime, initialPlacement.target, initialPlacement.up);
		}

		static Placement rotateAzimuth(float direction, float deltaTime) {
			float angle = direction * OB_rotSpeed * deltaTime;
			return currentPlacement.revolve(angle, 0.0f, initialPlacement.target, initialPlacement.up);
		}

		static Placement rotateElevation(float direction, float deltaTime) {
			float angle = direction * OB_rotSpeed * deltaTime;
			return currentPlacement.revolve(0.0f, angle, initialPlacement.target, initialPlacement.up);
		}
	};

	static void toggleMode() {
		if (currentBehaviour == Behaviour::FREEROAM) {
			currentBehaviour = Behaviour::ORBITAL;
			currentPlacement = initialPlacement;
		}
		else {
			currentBehaviour = Behaviour::FREEROAM;
		}
	}

	static void handleKey(unsigned char key, float deltaTime) {
		if (inFreeroam()) {
			switch (key) {
			case 'w': currentPlacement = Freeroam::moveForward(deltaTime); break;
			case 's': currentPlacement = Freeroam::moveBackward(deltaTime); break;
			case 'a': currentPlacement = Freeroam::moveLeft(deltaTime); break;
			case 'd': currentPlacement = Freeroam::moveRight(deltaTime); break;
			case 'q': currentPlacement = Freeroam::moveUp(deltaTime); break;
			case 'e': currentPlacement = Freeroam::moveDown(deltaTime); break;
			}
		}
		else {
			switch (key) {
			case 'w': currentPlacement = Orbital::zoomIn(deltaTime); break;
			case 's': currentPlacement = Orbital::zoomOut(deltaTime); break;
			}
		}
	}

	static void handleKeySpecial(int key_code, float deltaTime) {
		if (inFreeroam()) {
			float yawDir = 0.0f, pitchDir = 0.0f;
			switch (key_code) {
			case GLUT_KEY_RIGHT: yawDir = -1.0f; break;
			case GLUT_KEY_LEFT:  yawDir = 1.0f; break;
			case GLUT_KEY_UP:    pitchDir = 1.0f; break;
			case GLUT_KEY_DOWN:  pitchDir = -1.0f; break;
			}
			if (yawDir != 0.0f || pitchDir != 0.0f) {
				currentPlacement = Freeroam::rotate(yawDir, pitchDir, deltaTime);
			}
		}
		else {
			float azimuthDir = 0.0f, elevationDir = 0.0f;
			switch (key_code) {
			case GLUT_KEY_RIGHT: azimuthDir = 1.0f; break;
			case GLUT_KEY_LEFT:  azimuthDir = -1.0f; break;
			case GLUT_KEY_UP:    elevationDir = 1.0f; break;
			case GLUT_KEY_DOWN:  elevationDir = -1.0f; break;
			}
			if (azimuthDir != 0.0f) {
				currentPlacement = Orbital::rotateAzimuth(azimuthDir, deltaTime);
			}
			if (elevationDir != 0.0f) {
				currentPlacement = Orbital::rotateElevation(elevationDir, deltaTime);
			}
		}
	}

	static void lookAt() {
		gluLookAt(
			currentPlacement.pos.x, currentPlacement.pos.y, currentPlacement.pos.z,
			currentPlacement.target.x, currentPlacement.target.y, currentPlacement.target.z,
			currentPlacement.up.x, currentPlacement.up.y, currentPlacement.up.z
		);
	}

	static void perspective(float aspectRatio) {
		gluPerspective(
			currentProjection.fov,
			aspectRatio,
			currentProjection.near,
			currentProjection.far
		);
	}

	static glm::vec3 cartesian(float yawRad, float pitchRad) {
		return {
			cos(pitchRad) * sin(yawRad),
			sin(pitchRad),
			cos(pitchRad) * cos(yawRad)
		};
	}

	static std::pair<glm::vec3, glm::vec3> right_up(glm::vec3 front, glm::vec3 worldUp) {
		auto right = glm::normalize(glm::cross(front, worldUp));
		auto up = glm::normalize(glm::cross(right, front));
		return { right, up };
	}

	static glm::vec3 worldUp() { return initialPlacement.up; }

};

struct WindowState {
	inline static int fullscreen = false;

	inline static int initialWidth = 512;
	inline static int initialHeight = 512;

	inline static int currentWidth = initialWidth;
	inline static int currentHeight = initialHeight;

	static void changeSize(int w, int h) {
		if (h == 0)
			h = 1;

		float aspectRatio = w * 1.0f / h;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(0, 0, w, h);
		CameraController::perspective(aspectRatio);
		glMatrixMode(GL_MODELVIEW);
	}

	static void toggleFullscreen() {
		if (!fullscreen) {
			fullscreen = true;
			glutFullScreen();
		}
		else {
			glutPositionWindow(100, 100);
			glutReshapeWindow(initialWidth, initialHeight);
			fullscreen = false;
		}
	}
};

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

			glm::vec3 normal = glm::normalize(glm::cross(du,dv));

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

					auto p00 = glm::value_ptr(tessPoints[i][j]);
					auto p10 = glm::value_ptr(tessPoints[i+1][j]);
					auto p11 = glm::value_ptr(tessPoints[i+1][j+1]);
					auto p01 = glm::value_ptr(tessPoints[i][j+1]);

					glVertex3fv(p00);
					glVertex3fv(p10);
					glVertex3fv(p11);

					glVertex3fv(p00);
					glVertex3fv(p11);
					glVertex3fv(p01);
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

struct LightCaster {

	inline static std::vector<LightCaster> lights = {};
	inline static bool locationVisible = false;
	inline static float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; 

	enum class Type { POINT, DIRECTIONAL, SPOTLIGHT };

	Type type = Type::POINT;

	float pos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };  // point & spot
	float dir[4] = { 0.0f, 0.0f, -1.0f, 0.0f }; // directional & spot


	// spot
	float cutoffDegs = 30.0f;
	float exponent = 0.0f;

	// point & spot
	float constantAttenuation = 1.0f;
	float linearAttenuation = 1.0f;
	float quadraticAttenuation = 1.0f;

	void drawLocation() {
		if (type == Type::DIRECTIONAL)
			return;
		
		glPushMatrix();
		glPushAttrib(GL_POINT_BIT | GL_CURRENT_BIT | GL_LINE_BIT | GL_LIGHTING_BIT);
		glDisable(GL_LIGHTING);
		
		glTranslatef(pos[0], pos[1], pos[2]);
		glColor3f(1,1,0);

		// Locate the light source
		glPointSize(8.0f);
		glBegin(GL_POINTS);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glEnd();

		if (type == Type::SPOTLIGHT) {
			glLineWidth(2.0f);
			glBegin(GL_LINES);
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(dir[0] * 2.0f, dir[1] * 2.0f, dir[2] * 2.0f);
			glEnd();
		}

		glPopMatrix();
		glPopAttrib();
	}
	
	static void applyAll() {

		static int nLights = lights.size();

		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightCaster::white);
		for (int i = 0; i < std::min(nLights, 8); ++i) {

			GLenum lightID = GL_LIGHT0 + i;
			auto light = lights[i];

			if (locationVisible)
				light.drawLocation();

			glLightfv(lightID, GL_DIFFUSE, LightCaster::white);
			glLightfv(lightID, GL_SPECULAR, LightCaster::white);

			if (light.type == Type::DIRECTIONAL) {
				glLightfv(lightID, GL_POSITION, light.dir);
			}
			else {
				glLightfv(lightID, GL_POSITION, light.pos);
				//glLightf(lightID, GL_CONSTANT_ATTENUATION, light.constantAttenuation);
				//glLightf(lightID, GL_LINEAR_ATTENUATION, light.linearAttenuation);
				//glLightf(lightID, GL_QUADRATIC_ATTENUATION, light.quadraticAttenuation);

				if (light.type == Type::SPOTLIGHT) {
					glLightfv(lightID, GL_SPOT_DIRECTION, light.dir);
					glLightf(lightID, GL_SPOT_CUTOFF, light.cutoffDegs);
					glLightf(lightID, GL_SPOT_EXPONENT, light.exponent);
				}
			}

			glEnable(lightID);

		}

		
	}
	
	static void loadPoint(glm::vec3 position) {
		if (lights.size() > 8) return;
		LightCaster l;
		l.type = Type::POINT;
		l.pos[0] = position.x;
		l.pos[1] = position.y;
		l.pos[2] = position.z;
		lights.push_back(l);
	}

	static void loadDirectional(glm::vec3 direction) {
		LightCaster l;
		l.type = Type::DIRECTIONAL;
		l.dir[0] = direction.x;
		l.dir[1] = direction.y;
		l.dir[2] = direction.z;
		lights.push_back(l);
	}

	static void loadSpotlight(glm::vec3 position, glm::vec3 direction, float cutoff = 30.0f) {
		LightCaster l;
		l.type = Type::SPOTLIGHT;
		l.pos[0] = position.x;
		l.pos[1] = position.y;
		l.pos[2] = position.z;
		l.dir[0] = direction.x;
		l.dir[1] = direction.y;
		l.dir[2] = direction.z;
		l.cutoffDegs = cutoff;
		lights.push_back(l);
	}

	static void print() {
		std::cout << "===== LightCaster Report =====" << std::endl;
		std::cout << "Total lights: " << lights.size() << std::endl;
		std::cout << "Show location: " << (locationVisible ? "true" : "false") << std::endl;
		std::cout << "-----------------------------" << std::endl;

		for (size_t i = 0; i < lights.size(); ++i) {
			const auto& light = lights[i];
			std::cout << "Light #" << i << ":" << std::endl;
			std::cout << "  Type: ";
			switch (light.type) {
			case Type::POINT: std::cout << "POINT"; break;
			case Type::DIRECTIONAL: std::cout << "DIRECTIONAL"; break;
			case Type::SPOTLIGHT: std::cout << "SPOTLIGHT"; break;
			}
			std::cout << std::endl;
			
			if (light.type != Type::DIRECTIONAL) {
				std::cout << "  Position: ["
					<< light.pos[0] << ", "
					<< light.pos[1] << ", "
					<< light.pos[2] << ", "
					<< light.pos[3] << "]" << std::endl;
			}

			if (light.type != Type::POINT) {
				std::cout << "  Direction: ["
					<< light.dir[0] << ", "
					<< light.dir[1] << ", "
					<< light.dir[2] << "]" << std::endl;
			}

			//std::cout << "  Ambient: ["
			//	<< light.ambient[0] << ", "
			//	<< light.ambient[1] << ", "
			//	<< light.ambient[2] << ", "
			//	<< light.ambient[3] << "]" << std::endl;
			//std::cout << "  Diffuse: ["
			//	<< light.diffuse[0] << ", "
			//	<< light.diffuse[1] << ", "
			//	<< light.diffuse[2] << ", "
			//	<< light.diffuse[3] << "]" << std::endl;
			//std::cout << "  Specular: ["
			//	<< light.specular[0] << ", "
			//	<< light.specular[1] << ", "
			//	<< light.specular[2] << ", "
			//	<< light.specular[3] << "]" << std::endl;
			
			if (light.type == Type::SPOTLIGHT) {
				std::cout << "  Cutoff (degrees): " << light.cutoffDegs << std::endl;
				std::cout << "  Exponent: " << light.exponent << std::endl;
			}

			if (light.type != Type::DIRECTIONAL) {
				std::cout << "  Attenuation: "
					<< "Constant=" << light.constantAttenuation << ", "
					<< "Linear=" << light.linearAttenuation << ", "
					<< "Quadratic=" << light.quadraticAttenuation << std::endl;
			}

			std::cout << "-----------------------------" << std::endl;
		}
	}

	static void toggleLocations() {
		locationVisible = !locationVisible;
	}
};

struct AnimatedTranslation {

	inline static const std::vector<int> tessellationLevels = { 10 }; //{ 1, 2, 4, 10 };
	inline static int currentTessIndex = tessellationLevels.size()-1;
	inline static bool showPath = false;

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
	}

	void apply(float tDelta) {

		if (AnimatedTranslation::showPath == true) {
			drawControlPoints();
		    crPath.drawWhole(tessellationLevels[currentTessIndex]);
		}

		crPath.transform(t, aligned, CameraController::initialPlacement.up);

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
			"  Current t: {:.2f}\n"
			"  Period: {:.2f}\n"
			"  Aligned: {}\n"
			"  Show Path: {}\n"
			"}}",
			pointsStr,
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

struct Texture {

	inline static std::unordered_map<std::string, unsigned int> textureIDs = {};
	inline static int minFilter = GL_NEAREST;
	inline static bool anisotropy = false;

	static unsigned int id(std::string filename) {
		return (textureIDs.contains(filename)) ? textureIDs[filename] : 0;
	}

	static void setFilter(int newFilter) {
		minFilter = newFilter;
		updateFiltering();
	}

	static void setAnisotropy(bool enabled) {
		anisotropy = enabled;
		updateFiltering();
	}

	static void updateFiltering(std::string filename = "") {
		
		auto updateTexture = [](GLuint texID) {
			glBindTexture(GL_TEXTURE_2D, texID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (minFilter == GL_NEAREST)?GL_NEAREST:GL_LINEAR);
			if (anisotropy) {
				GLfloat maxAnisotropy;
				glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
				glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
			}
			else glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 0.0f);
			};

		if (!filename.empty())
			updateTexture(id(filename));
		else
			for (auto& [_, texID] : textureIDs)
				updateTexture(texID);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	static void load(std::string filename, unsigned int id) {
		if (!textureIDs.contains(filename)) {
			textureIDs.insert(std::make_pair(filename, id));
			updateFiltering(filename);
		}
	}

	static void print() {
		for (auto& [filename, texID] : textureIDs) {
			std::cout << std::format("Texture {} (ID: {})", filename, texID) << std::endl;
		}
	}
};

struct Material {
	GLfloat diffuse[3] = { 200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f };
	GLfloat ambient[3] = { 50.0f / 255.0f, 50.0f / 255.0f, 50.0f / 255.0f };
	GLfloat specular[3] = { 0.0f, 0.0f, 0.0f };
	GLfloat emissive[3] = { 0.0f, 0.0f, 0.0f };
	GLfloat shininess[1] = { 0.0f };
};

struct Model {

	inline static bool showAxes = false;
	inline static bool showTexture = true;

	std::vector<glm::vec3> vertices = {};
	std::vector<glm::vec3> normals = {};
	std::vector<glm::vec2> texcoords = {};

	std::vector<unsigned int> vIndices = {};
	std::vector<unsigned int> vnIndices = {};
	std::vector<unsigned int> vtIndices = {};

	GLuint vertexBufferID = 0;
	GLuint indexBufferID = 0;
	bool buffersInitialised = false;

	std::pair<std::vector<float>, std::vector<unsigned int>> interleavedData() {

		// Insert vertex attributes (position + normal + texcoord) according to indices
		// this pmuch negates the whole purpose of indices in the first place
		// too bad
		std::vector<float> interleavedAttribs;
		interleavedAttribs.reserve(vIndices.size() * 8); // 3 pos + 3 normal + 2 texcoord per vertex

		for (size_t i = 0; i < vIndices.size(); i++) {
			// Position (must exist)
			glm::vec3 vertex = vertices[vIndices[i]];
			interleavedAttribs.push_back(vertex.x);
			interleavedAttribs.push_back(vertex.y);
			interleavedAttribs.push_back(vertex.z);

			// Normal (default if missing)
			glm::vec3 normal(0, 1, 0);
			if (i < vnIndices.size() && vnIndices[i] < normals.size()) {
				normal = normals[vnIndices[i]];
			}
			interleavedAttribs.push_back(normal.x);
			interleavedAttribs.push_back(normal.y);
			interleavedAttribs.push_back(normal.z);

			// Texture coordinate (default if missing)
			glm::vec2 texcoord(0, 0);
			if (i < vtIndices.size() && vtIndices[i] < texcoords.size()) {
				texcoord = texcoords[vtIndices[i]];
			}
			interleavedAttribs.push_back(texcoord.s);
			interleavedAttribs.push_back(texcoord.t);
		}

		// Create element indices (sequential since we've deferred the vertex data)
		std::vector<unsigned int> elementIndices(vIndices.size());
		for (unsigned int i = 0; i < vIndices.size(); i++) {
			elementIndices[i] = i;
		}

		return { interleavedAttribs, elementIndices };
	}

	static void toggleAxes() {
		Model::showAxes = !Model::showAxes;
	}

	static void toggleTextures() {
		Model::showTexture = !Model::showTexture;
	}
	
	void drawAxes() const {

		glPushAttrib(GL_LIGHTING_BIT);
		glDisable(GL_LIGHTING);

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

		// polars
		static const float radius = 1.3f;
		static const int slices = 20;
		static constexpr float step = glm::radians(360.0) / slices;

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
		

		// Draw normals from GPU buffer
		if (buffersInitialised) {
			glColor3f(1.0f, 0.5f, 0.0f); // Orange

			// Bind the vertex buffer
			glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

			// Get the size of the buffer
			GLint bufferSize;
			glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bufferSize);

			// Map the buffer to client memory
			float* bufferData = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
			const size_t stride = 8; // 3 pos + 3 normal + 2 texcoord

			if (bufferData) {
				glBegin(GL_LINES);

				for (size_t i = 0; i < vIndices.size(); i++) {
					// Get vertex position (first 3 floats)
					float x = bufferData[i * stride + 0];
					float y = bufferData[i * stride + 1];
					float z = bufferData[i * stride + 2];

					// Get normal (next 3 floats)
					float nx = bufferData[i * stride + 3];
					float ny = bufferData[i * stride + 4];
					float nz = bufferData[i * stride + 5];

					// Draw line from vertex to vertex + normal*0.2
					glVertex3f(x, y, z);
					glVertex3f(x + nx * 0.2f, y + ny * 0.2f, z + nz * 0.2f);
				}
				glEnd();

				glUnmapBuffer(GL_ARRAY_BUFFER);
			}

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		glPopAttrib(); // GL_CURRENT_BIT
		glPopAttrib(); // GL_LIGHTING_BIT

	}

	void initBuffers() {
		if (buffersInitialised) return;

		// Generate buffers
		glGenBuffers(1, &vertexBufferID);
		glGenBuffers(1, &indexBufferID);

		auto [interleavedVertexAttribs, elementIndices] = interleavedData();

		// Upload interleaved vertex data
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
		glBufferData(GL_ARRAY_BUFFER,
			interleavedVertexAttribs.size() * sizeof(float),
			interleavedVertexAttribs.data(),
			GL_STATIC_DRAW);

		// Upload index data
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER,
			elementIndices.size() * sizeof(unsigned int),
			elementIndices.data(),
			GL_STATIC_DRAW);

		buffersInitialised = true;
	}

	void cleanupBuffers() {
		if (vertexBufferID) glDeleteBuffers(1, &vertexBufferID);
		if (indexBufferID) glDeleteBuffers(1, &indexBufferID);
		vertexBufferID = indexBufferID = 0;
		buffersInitialised = false;
	}

	void draw(unsigned int textureID = 0, Material material = Material()) {

		if (buffersInitialised == false) initBuffers();
		if (showAxes) drawAxes();

		const size_t stride = 8 * sizeof(float);

		glPushAttrib(GL_LIGHTING_BIT);
		
		glMaterialfv(GL_FRONT, GL_DIFFUSE, material.diffuse);
		glMaterialfv(GL_FRONT, GL_AMBIENT, material.ambient);
		glMaterialfv(GL_FRONT, GL_SPECULAR, material.specular);
		glMaterialfv(GL_FRONT, GL_SHININESS, material.shininess);
		glMaterialfv(GL_FRONT, GL_EMISSION, material.emissive);

		// Bind the VBO and set up pointers
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

		// Position (offset 0)
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, stride, 0);

		// Normal (offset 3 floats)
		glEnableClientState(GL_NORMAL_ARRAY);
		glNormalPointer(GL_FLOAT, stride, (void*)(3 * sizeof(float)));

		// Texture coordinate (offset 6 floats)
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, (showTexture) ? textureID : 0);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, stride, (void*)(6 * sizeof(float)));

		// Draw elements
		int vertexCount = vIndices.size();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
		glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);

		// Clean up
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);

		glPopAttrib();
	}
};


struct ModelStorage {

	inline static std::unordered_map<std::string, Model> models;

	static void draw(std::string modelFilename, unsigned int textureID = 0, Material material = Material()) {
		if (models.contains(modelFilename))
			models[modelFilename].draw(textureID, material);
	}

	static void initBuffers() {
		for (auto& [filename, model] : models) {
			model.initBuffers();
		}
	}

	static void cleanupBuffers() {
		for (auto& [filename, model] : models) {
			model.cleanupBuffers();
		}
	}

	static void load(std::string modelFilename, Model model) {
		if (!models.contains(modelFilename))
			models.emplace(modelFilename, model);
	}

};

struct Group {

	struct ModelReference {

		std::string modelFilename;
		std::string textureFilename = "";
		Material material = Material();

	};

	std::vector<Transform> transforms = {};
	std::vector<Group> subgroups = {};

	std::vector<ModelReference> modelReferences;

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

		glPushAttrib(GL_LIGHTING_BIT);
		glDisable(GL_LIGHTING);

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

		glPopAttrib();
	}

	void render(float tDelta) {

		glPushMatrix();
		glPushAttrib(GL_CURRENT_BIT);

		applyTransforms(tDelta);

		for (auto mref : modelReferences) {

			std::string mtlString = std::format(
				"Material( diff({},{},{}) amb({},{},{}) spc({},{},{}) ems({},{},{}) shine({}) )",
				mref.material.diffuse[0],  mref.material.diffuse[1],  mref.material.diffuse[2],
				mref.material.ambient[0],  mref.material.ambient[1],  mref.material.ambient[2],
				mref.material.specular[0], mref.material.specular[1], mref.material.specular[2],
				mref.material.emissive[0], mref.material.emissive[1], mref.material.emissive[2],
				mref.material.shininess[0]
			);

			ModelStorage::draw(mref.modelFilename, Texture::id(mref.textureFilename), mref.material);
		}

		for (auto& subgroup : subgroups)
			subgroup.render(tDelta);

		glPopAttrib();
		glPopMatrix();
	}
};

struct World {

	std::vector<Group> groups = {};

	void renderGroups(float tDelta) {

		for (auto& g : groups)
			g.render(tDelta);
	}

};


#endif