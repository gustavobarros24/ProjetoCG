#include <array>
#include <cmath>

#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../engine/Parsing.h"

World world;

namespace window {
	bool inFullscreen = false;

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

	void toggleFullscreen() {
		static int wPrev = world.window.width;
		static int hPrev = world.window.height;
		static int xPrev = 100;
		static int yPrev = 100;

		if (inFullscreen == false) {
			// save current window position/size and go fullscreen
			wPrev = glutGet(GLUT_WINDOW_WIDTH);
			hPrev = glutGet(GLUT_WINDOW_HEIGHT);
			xPrev = glutGet(GLUT_WINDOW_X);
			yPrev = glutGet(GLUT_WINDOW_Y);
			inFullscreen = true;
			glutFullScreen();
		}
		else {
			// restore last window position/size
			glutPositionWindow(xPrev, yPrev);
			glutReshapeWindow(wPrev, hPrev);
			inFullscreen = false;
		}
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

		const float sensitivity = 200.0f;
		const float movementSpeed = 100.0f;

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

		void moveForward(float deltaTime) {
			pos += front * movementSpeed * deltaTime;
		}

		void moveBackward(float deltaTime) {
			pos -= front * movementSpeed * deltaTime;
		}

		void moveRight(float deltaTime) {
			glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
			pos += right * movementSpeed * deltaTime;
		}

		void moveLeft(float deltaTime) {
			glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
			pos -= right * movementSpeed * deltaTime;
		}

		void moveUp(float deltaTime) {
			pos += worldUp * movementSpeed * deltaTime;
		}

		void moveDown(float deltaTime) {
			pos -= worldUp * movementSpeed * deltaTime;
		}

		void rotateYaw(float direction, float deltaTime) {
			yaw += direction * sensitivity * deltaTime;
			updateVectors();
		}

		void rotatePitch(float direction, float deltaTime) {
			pitch += direction * sensitivity * deltaTime;
			if (pitch > 89.99f) pitch = 89.99f;
			if (pitch < -89.99f) pitch = -89.99f;
			updateVectors();
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
		
		const float sensitivity = 300.0f;
		const float zoomSpeed = 150.0f;

		void updatePosition() {
			// Constrain elevation to prevent over-rotation
			elevation = glm::clamp(elevation, -89.99f, 89.99f);

			float x = radius * cos(glm::radians(elevation)) * sin(glm::radians(azimuth));
			float y = radius * sin(glm::radians(elevation));
			float z = radius * cos(glm::radians(elevation)) * cos(glm::radians(azimuth));

			position = lookAt + glm::vec3(x, y, z);
		}

		void rotateAzimuth(float direction, float deltaTime) {
			azimuth += direction * sensitivity * deltaTime;
			updatePosition();
		}

		void rotateElevation(float direction, float deltaTime) {
			elevation += direction * sensitivity * deltaTime;
			updatePosition();
		}

		void zoomIn(float deltaTime) {
			radius = std::max(1.0f, radius - zoomSpeed * deltaTime);
			updatePosition();
		}

		void zoomOut(float deltaTime) {
			radius += zoomSpeed * deltaTime;
			updatePosition();
		}
	
		void initialize() {
			position = glm::vec3(
				world.camera.position[0],
				world.camera.position[1],
				world.camera.position[2]
			);

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

	void handleKey(unsigned char key, float deltaTime) {
		if (key == 'c' || key == 'C') {
			toggleMode();
			return;
		}

		if (inFreeroam) {
			switch (key) {
			case 'w': freeroam::moveForward(deltaTime); break;
			case 's': freeroam::moveBackward(deltaTime); break;
			case 'a': freeroam::moveLeft(deltaTime); break;
			case 'd': freeroam::moveRight(deltaTime); break;
			case 'q': freeroam::moveUp(deltaTime); break;
			case 'e': freeroam::moveDown(deltaTime); break;
			}
		}
		else {
			switch (key) {
			case 'w': orbital::zoomIn(deltaTime); break;
			case 's': orbital::zoomOut(deltaTime); break;
			}
		}
	}

	void handleKeySpecial(int key_code, float deltaTime) {
		if (inFreeroam) {
			switch (key_code) {
			case GLUT_KEY_RIGHT: freeroam::rotateYaw(-1.0f, deltaTime); break;
			case GLUT_KEY_LEFT:  freeroam::rotateYaw(1.0f, deltaTime); break;
			case GLUT_KEY_UP:    freeroam::rotatePitch(1.0f, deltaTime); break;
			case GLUT_KEY_DOWN:  freeroam::rotatePitch(-1.0f, deltaTime); break;
			}
		}
		else {
			switch (key_code) {
			case GLUT_KEY_RIGHT: orbital::rotateAzimuth(1.0f, deltaTime); break;
			case GLUT_KEY_LEFT:  orbital::rotateAzimuth(-1.0f, deltaTime); break;
			case GLUT_KEY_UP:    orbital::rotateElevation(1.0f, deltaTime); break;
			case GLUT_KEY_DOWN:  orbital::rotateElevation(-1.0f, deltaTime); break;
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

namespace keybinds {

	void keyboardSpecialUp(int key_code, int x, int y);

	void keyboardUp(unsigned char key, int x, int y);

	void keyboardSpecial(int key_code, int x, int y);

	void keyboard(unsigned char key, int x, int y);

	void update(float deltaTime);

};

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

namespace generateVertices {

	void addQuad(
		std::vector<float>& vertices,
		const glm::vec3& v1, const glm::vec3& v2,
		const glm::vec3& v3, const glm::vec3& v4,
		bool facingFront = true) {

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
};

auto myplane = generateVertices::plane(1, 10);

namespace render { 
	
	namespace polygonMode {

		const GLenum modes[] = { GL_FILL, GL_LINE, GL_POINT };
		const std::string strModes[] = { "Fill", "Line", "Point" };
		int currentMode = 0;

		std::string hudString = std::format("[1] Polygons: {}", strModes[currentMode]);

		void next() {
			currentMode = (currentMode + 1) % 3;
			glPolygonMode(GL_FRONT_AND_BACK, modes[currentMode]);
			hudString = std::format("[1] Polygons: {}", strModes[currentMode]);
		}
	}

	namespace axes {

		bool visible = false;
		const std::string strModes[] = { "Off", "On" };

		std::string hudString = std::format("[2] Axes: {}", strModes[visible]);

		void cartesian() {

			/*
			auto arrowedAxis = [](float length, bool x, bool y, bool z) -> void {
					glPushAttrib(GL_CURRENT_BIT);
					glPushMatrix();

					glColor3f(1*x, 1*y, 1*z);


					glBegin(GL_LINES);
					glVertex3f(0,0,0);
					glVertex3f(length*x,length*y,length*z);
					glEnd();

					glTranslatef(length*x, length*y, length*z);

					//pitch
					glRotatef(y * 90.0f, -1, 0, 0);
					//yaw
					glRotatef(x * 90.0f, 0, 1, 0);

					glutSolidCone(0.05, 0.3, 10, 1);

					glPopMatrix();
					glPopAttrib();
				};

			arrowedAxis(1000.0f, 1, 0, 0);
			arrowedAxis(1000.0f, 0, 1, 0);
			arrowedAxis(1000.0f, 0, 0, 1);
			*/

			glPushAttrib(GL_CURRENT_BIT);
			glBegin(GL_LINES);

			glColor3f(1.0f, 0.0f, 0.0f);  // Red
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(1000.0f, 0.0f, 0.0f);

			glColor3f(0.0f, 1.0f, 0.0f);  // Green
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 1000.0f, 0.0f);

			glColor3f(0.0f, 0.0f, 1.0f);  // Blue
			glVertex3f(0.0f, 0.0f, 0.0f);
			glVertex3f(0.0f, 0.0f, 1000.0f);

			glEnd();
			glPopAttrib();

		}

		void toggle() {
			visible = !visible;
			AnimatedTranslation::showPath = false;
			hudString = std::format("[2] Axes: {}", strModes[visible]);
		}

		void show() {
			cartesian();
			AnimatedTranslation::showPath = true;
		}

	};

	namespace cullMode {
		enum Mode { NONE, BACK, FRONT };
		const std::string strModes[] = { "None", "Back", "Front" };
		int currentMode = NONE;

		std::string hudString = std::format("[3] Culling: {}", strModes[currentMode]);

		void next() {
			currentMode = (currentMode + 1) % 3;

			switch (currentMode) {
			case NONE:
				glDisable(GL_CULL_FACE);
				break;

			case BACK:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_BACK);
				break;

			case FRONT:
				glEnable(GL_CULL_FACE);
				glCullFace(GL_FRONT);
				break;
			}

			hudString = std::format("[3] Culling: {}", strModes[currentMode]);
		}
	};

	namespace vboMode {

		bool withVBOs = true;

		const std::string strModes[] = { "Off", "On" };
		std::string hudString = std::format("[4] VBOs: {}", strModes[withVBOs]);

		void toggle() {
			withVBOs = !withVBOs;
			hudString = std::format("[4] VBOs: {}", strModes[withVBOs]);
		}
	};

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
		
			glPushAttrib(GL_CURRENT_BIT | GL_ENABLE_BIT);
			//glDisable(GL_LIGHTING);
			//glDisable(GL_DEPTH_TEST);

			double windowWidth = glutGet(GLUT_WINDOW_WIDTH);
			double windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(0, windowWidth, 0, windowHeight);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();

			std::vector<std::string> lines = {
				framesPerSecond::hudString,
				clock::hudString,
				polygonMode::hudString,
				axes::hudString,
				cullMode::hudString,
				vboMode::hudString,
				AnimatedTranslation::hudString
			};

			glColor3f(1.0f, 1.0f, 1.0f);

			float y = windowHeight - 25.0f;
			for (const auto& line : lines) {
				glRasterPos2f(10, y);
				for (char c : line) {
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
				}
				y -= 15.0f;
			}

			glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
			glPopMatrix();

			glPopAttrib();
		}
	};

	void renderScene(void) {
		clock::update();

		keybinds::update(clock::deltaTime);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera::set();

		if (axes::visible) axes::show();
		
		//world.renderGroups(vboMode::withVBOs, clock::deltaTime);
		//debugPatch.draw(20);

		glBegin(GL_TRIANGLES);

		for (size_t i = 0; i < myplane.size(); i += 3) {
			glVertex3f(
				myplane[i],
				myplane[i + 1],
				myplane[i + 2]
			);
		}

		glEnd();

		framesPerSecond::update(clock::currentTime, 100.0f);
		hud::show();

		glutSwapBuffers();
	}
};

namespace keybinds {

	std::unordered_set<unsigned char> keysPressed;
	std::unordered_set<int>           specialKeysPressed;

	std::unordered_set<unsigned char> toggleKeys = {
		'1','2','3','4','5','0',
		'c','C'
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
			break;
		case '3':
			render::cullMode::next();
			break;
		case '4':
			render::vboMode::toggle();
			break;
		case '5':
			AnimatedTranslation::nextTessellationLevel();
			break;
		case '0':
			window::toggleFullscreen();
			break;
		
		case 'c':
		case 'C':
			camera::toggleMode();
			break;
		}
	}

	void update(float deltaTime) {
		// Handle continuous key presses
		for (unsigned char key : keysPressed) {
			camera::handleKey(key, deltaTime);
		}

		for (int key_code : specialKeysPressed) {
			camera::handleKeySpecial(key_code, deltaTime);
		}
	}
};


int main(int argc, char** argv) {	

	world = configParser::loadWorld("config.xml");
	std::cout << std::string(world) << std::endl;

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(world.window.width, world.window.height);
	glutCreateWindow("Engine");
	glutReshapeFunc(window::changeSize);

	// Initialize GLEW
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		return 1;
	}

	world.modelStorage.initializeAllVBOs();
	atexit([]() { world.modelStorage.cleanupAllVBOs(); });

	glutIdleFunc(render::renderScene);
	glutDisplayFunc(render::renderScene);

	glutKeyboardFunc(keybinds::keyboard);
	glutKeyboardUpFunc(keybinds::keyboardUp);
	glutSpecialFunc(keybinds::keyboardSpecial);
	glutSpecialUpFunc(keybinds::keyboardSpecialUp);

	glEnable(GL_DEPTH_TEST);
	render::cullMode::next();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glutMainLoop();

	return 1;
}