#include <array>
#include <cmath>

#include <glm/glm.hpp>

#include "ConfigParser.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

World world = ConfigParser::loadFromFile("config.xml");

namespace window {
	void changeSize(int w, int h) {
		if (h == 0)
			h = 1;

		float ratio = w * 1.0f / h;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(0, 0, w, h);
		gluPerspective(
			world.camera.fov,
			ratio,
			world.camera.nearClip,
			world.camera.farClip
		);
		glMatrixMode(GL_MODELVIEW);
	}
};

namespace camera {
	bool inFreeroam = true;
	const glm::vec3 worldUp = glm::vec3(
		world.camera.up[0],
		world.camera.up[1],
		world.camera.up[2]
	);

	namespace freeroam {

		glm::vec3 pos = glm::vec3(
			world.camera.position[0],
			world.camera.position[1],
			world.camera.position[2]
		);

		glm::vec3 front = glm::normalize(
			glm::vec3(
			world.camera.lookAt[0],
			world.camera.lookAt[1],
			world.camera.lookAt[2]) - pos
		);
		glm::vec3 up = glm::normalize(worldUp);

		float yaw = glm::degrees(atan2(front.x, front.z));
		float pitch = glm::degrees(asin(front.y));
		
		const float movementSpeed = 0.5f;
		const float sensitivity = 0.5f;

		void updateVectors() {
			// Calculate new front vector
			front.x = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			front.y = sin(glm::radians(pitch));
			front.z = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			front = glm::normalize(front);

			// Re-calculate right and up vectors
			glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
			up = glm::normalize(glm::cross(right, front));
		}

		void rotateYaw(float direction) {
			yaw += direction * sensitivity;
			updateVectors();
		}

		void rotatePitch(float direction) {
			pitch += direction * sensitivity;

			// Constrain pitch to prevent over-rotation
			if (pitch > 89.0f) pitch = 89.0f;
			if (pitch < -89.0f) pitch = -89.0f;

			updateVectors();
		}

		void moveForward() {
			pos += front * movementSpeed;
		}

		void moveBackward() {
			pos -= front * movementSpeed;
		}

		void moveRight() {
			glm::vec3 right = glm::normalize(glm::cross(-front, worldUp));
			pos -= right * movementSpeed;
		}

		void moveLeft() {
			glm::vec3 right = glm::normalize(glm::cross(-front, worldUp));
			pos += right * movementSpeed;
		}

		void moveUp() {
			pos += worldUp * movementSpeed;
		}

		void moveDown() {
			pos -= worldUp * movementSpeed;
		}

	};

	namespace orbital {
		glm::vec3 position;
		glm::vec3 lookAt = glm::vec3(
			world.camera.lookAt[0],
			world.camera.lookAt[1],
			world.camera.lookAt[2]
		);

		float radius;
		
		float azimuth = 0.0f;    // Horizontal angle
		float elevation = 0.0f; // Vertical angle
		
		const float sensitivity = 2.0f;
		const float zoomSpeed = 1.0f;

		void updatePosition() {
			// Constrain elevation to prevent over-rotation
			elevation = glm::clamp(elevation, -89.0f, 89.0f);

			// Calculate spherical coordinates to cartesian
			float x = radius * cos(glm::radians(elevation)) * sin(glm::radians(azimuth));
			float y = radius * sin(glm::radians(elevation));
			float z = radius * cos(glm::radians(elevation)) * cos(glm::radians(azimuth));

			position = lookAt + glm::vec3(x, y, z);
		}

		void rotateAzimuth(float direction) {
			azimuth += direction * sensitivity;
			updatePosition();
		}

		void rotateElevation(float direction) {
			elevation += direction * sensitivity;
			updatePosition();
		}

		void zoomIn() {
			radius = std::max(1.0f, radius - zoomSpeed);
			updatePosition();
		}

		void zoomOut() {
			radius += zoomSpeed;
			updatePosition();
		}

		void initialize() {
			// Initialize from world.camera

			position = glm::vec3(
				world.camera.position[0],
				world.camera.position[1],
				world.camera.position[2]
			);

			// Calculate initial spherical coordinates
			glm::vec3 toCamera = position - lookAt;
			radius = glm::length(toCamera);
			if (radius > 0.001f) {
				toCamera = glm::normalize(toCamera);
				elevation = glm::degrees(asinf(toCamera.y));
				azimuth = glm::degrees(atan2f(toCamera.x, toCamera.z));
			}

			updatePosition();
		}
	};

	void toggleMode() {
		if (inFreeroam) {
			orbital::initialize();
		}
		else {
			freeroam::updateVectors();
		}
		inFreeroam = !inFreeroam;
	}

	void handleKey(unsigned char key) {
		if (key == 'c' || key == 'C') {
			toggleMode();
			return;
		}

		if (inFreeroam) {
			switch (key) {
			case 'w': freeroam::moveForward(); break;
			case 's': freeroam::moveBackward(); break;
			case 'a': freeroam::moveLeft(); break;
			case 'd': freeroam::moveRight(); break;
			case 'q': freeroam::moveUp(); break;
			case 'e': freeroam::moveDown(); break;
			}
		}
		else {
			switch (key) {
			case 'w': orbital::rotateElevation(1.0f); break;
			case 's': orbital::rotateElevation(-1.0f); break;
			case 'a': orbital::rotateAzimuth(1.0f); break;
			case 'd': orbital::rotateAzimuth(-1.0f); break;
			case 'q': orbital::zoomIn(); break;
			case 'e': orbital::zoomOut(); break;
			}
		}
	}

	void handleKeySpecial(int key_code) {
		if (inFreeroam) {
			switch (key_code) {
			case GLUT_KEY_RIGHT: freeroam::rotateYaw(-1.0f); break;
			case GLUT_KEY_LEFT:  freeroam::rotateYaw(1.0f); break;
			case GLUT_KEY_UP:    freeroam::rotatePitch(1.0f); break;
			case GLUT_KEY_DOWN:  freeroam::rotatePitch(-1.0f); break;
			}
		}
	}

	void set() {
		glLoadIdentity();

		if (inFreeroam) {
			gluLookAt(
				freeroam::pos.x,
				freeroam::pos.y,
				freeroam::pos.z,

				freeroam::pos.x + freeroam::front.x,
				freeroam::pos.y + freeroam::front.y,
				freeroam::pos.z + freeroam::front.z,

				freeroam::up.x,
				freeroam::up.y,
				freeroam::up.z
			);
		}
		else { // for orbital
			gluLookAt(
				orbital::position.x,
				orbital::position.y,
				orbital::position.z,
				
				world.camera.lookAt[0],
				world.camera.lookAt[1],
				world.camera.lookAt[2],
				
				world.camera.up[0],
				world.camera.up[1],
				world.camera.up[2]
			);
		}
	}
};

namespace render {

	namespace polygonMode {

		const GLenum modes[] = { GL_FILL, GL_LINE, GL_POINT };
		int currentMode = 0;

		void next() {
			currentMode = (currentMode + 1) % 3;
			glPolygonMode(GL_FRONT_AND_BACK, modes[currentMode]);
		}
	}

	namespace axes {
		
		bool visible = false;

		void toggle() { visible = !visible; }

		void cartesian() {

			glPushAttrib(GL_CURRENT_BIT);
			glBegin(GL_LINES);

			glm::vec3 origin = { 0.0f, 0.0f, 0.0f };

			std::array<glm::vec3, 3> axisHeads = {
				glm::vec3(100.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 100.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, 100.0f),
			};

			std::array<glm::vec3, 3> axisColours = {
				glm::vec3(1.0f, 0.0f, 0.0f),
				glm::vec3(0.0f, 1.0f, 0.0f),
				glm::vec3(0.0f, 0.0f, 1.0f)
			};

			for (int i = 0; i < 3; i++) {
				glColor3f(axisColours[i].x, axisColours[i].y, axisColours[i].z);
				glVertex3f(origin.x, origin.y, origin.z);
				glVertex3f(axisHeads[i].x, axisHeads[i].y, axisHeads[i].z);
			}

			glEnd();
			glPopAttrib();
		}

		void polar() {

			const int segments = 100; // Number of segments to approximate the circle
			constexpr float step = glm::radians(360.0) / segments;

			for (int radius = 10; radius <= 500; radius += 10) {
				glBegin(GL_LINE_LOOP);

				for (int i = 0; i < segments; i++) {
					float angle = i * step;
					float x = radius * cos(angle);
					float z = radius * sin(angle);
					glVertex3f(x, 0.0f, z); // Y is 0 for XZ plane
				}

				glEnd();
			}

		}
	};

	namespace cullMode {
		enum Mode {
			NONE,
			BACK,
			FRONT
		};

		Mode currentMode = NONE;

		void next() {
			currentMode = static_cast<Mode>((currentMode + 1) % 3);

			if (currentMode == NONE) {
				glDisable(GL_CULL_FACE);
			}
			else {
				glEnable(GL_CULL_FACE);
				glCullFace(currentMode == BACK ? GL_BACK : GL_FRONT);
			}
		}
	};

	void applyTransform(const Transform& transform) {

		struct TransformMapping {
			void operator()(const Translation& t) { glTranslatef(t.x, t.y, t.z);       }
			void operator()(const Rotation& r)    { glRotatef(r.angle, r.x, r.y, r.z); }
			void operator()(const Scaling& s)     { glScalef(s.x, s.y, s.z);           }
		};

		std::visit(TransformMapping{}, transform);
	}

	void renderGroup(const Group& group) {
		glPushMatrix();
		glPushAttrib(GL_CURRENT_BIT);

		// Apply colours
		glColor3f(group.colour.r, group.colour.g, group.colour.b);

		// Apply transforms
		for (const auto& transform : group.transforms) {
			applyTransform(transform);
		}
		
		// Draw this group's models
		glBegin(GL_TRIANGLES);
		for (const auto& model : group.models) {
			for (size_t i = 0; i < model.vertices.size(); i += 3) {
				glVertex3f(
					model.vertices[i],
					model.vertices[i + 1],
					model.vertices[i + 2]
				);
			}
		}
		glEnd();

		// Recursively render subgroups
		for (const auto& subgroup : group.subgroups) {
			renderGroup(subgroup);
		}

		
		glPopAttrib();
		glPopMatrix();
	}

	void renderScene(void) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera::set();
		if (axes::visible) axes::cartesian();
		if (axes::visible) axes::polar();

		for (const auto& group : world.groups) {
			render::renderGroup(group);
		}
		
		glutSwapBuffers();
	}
};

namespace keybinds {

	void keyboardSpecial(int key_code, int x, int y) {
		camera::handleKeySpecial(key_code);
	}

	void keyboard(unsigned char key, int x, int y) {
		// camera navigation
		camera::handleKey(key);

		// misc
		switch (key) {
		case '1': render::polygonMode::next(); break;
		case '2': render::axes::toggle(); break;
		case '3': render::cullMode::next(); break;
		}
	}
};

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(world.window.width, world.window.height);
	glutCreateWindow("Engine");

	glutReshapeFunc(window::changeSize);
	glutIdleFunc(render::renderScene);
	glutDisplayFunc(render::renderScene);
	glutKeyboardFunc(keybinds::keyboard);
	glutSpecialFunc(keybinds::keyboardSpecial);

	glEnable(GL_DEPTH_TEST);
	render::cullMode::next();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glutMainLoop();
	return 1;
}