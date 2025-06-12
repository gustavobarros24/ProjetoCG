#ifndef GRAPHICSUTILS_H
#define GRAPHICSUTILS_H

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "Config.h"
#include <array>
#include <functional>
#include <glm/glm.hpp>

World world = ConfigParser::loadFromFile("solarsystem.xml");

namespace polygonMode {

	const GLenum modes[] = { GL_FILL, GL_LINE, GL_POINT };
	int currentMode = 0;

	void next() {
		currentMode = (currentMode + 1) % 3;
		glPolygonMode(GL_FRONT_AND_BACK, modes[currentMode]);
	}

}

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
	const glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	bool inFreeroam = true;
	
	glm::vec3 pos = glm::vec3(0.0f, 2.0f, 0.0f);
	glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

	float yaw = -90.0f;   // Initial yaw (looking along -Z)
	float pitch = 0.0f;
	const float movementSpeed = 0.5f;
	const float sensitivity = 0.5f;

	void updateVectors() {
		// Calculate new front vector
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
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
		glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
		pos += right * movementSpeed;
	}

	void moveLeft() {
		glm::vec3 right = glm::normalize(glm::cross(front, worldUp));
		pos -= right * movementSpeed;
	}
};

namespace keybinds {

	void keyboardSpecial(int key_code, int x, int y) {
		//cameraControl.handleSpecialKey(key_code);

		switch (key_code) {
		case GLUT_KEY_RIGHT: camera::rotateYaw(1.0f); break;
		case GLUT_KEY_LEFT: camera::rotateYaw(-1.0f); break;
		case GLUT_KEY_UP: camera::rotatePitch(1.0f); break;
		case GLUT_KEY_DOWN: camera::rotatePitch(-1.0f); break;
		}
	}

	void keyboard(unsigned char key, int x, int y) {
		//cameraControl.handleKey(key);
		switch (key) {

		// camera navigation
		case 'w': camera::moveForward(); break;
		case 's': camera::moveBackward(); break;
		case 'a': camera::moveLeft(); break;
		case 'd': camera::moveRight(); break;
		
		// misc
		case ' ': polygonMode::next(); break;
		}
	}
};

namespace render {

	void pushPop(const std::function<void()>& codeBlock) {
		glPushMatrix();
		codeBlock();
		glPopMatrix();
	}

	void axes() {

		glBegin(GL_LINES);
		glPushAttrib(GL_CURRENT_BIT);

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

		glPopAttrib();
		glEnd();
	}

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

		// Apply all transforms for this group
		for (const auto& transform : group.transforms) {
			applyTransform(transform);
		}

		// Render models in this group
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

		glPopMatrix();
	}

	void setCamera() {
		glLoadIdentity();
		
		gluLookAt(
			camera::pos.x,
			camera::pos.y,
			camera::pos.z,

			camera::pos.x + camera::front.x,
			camera::pos.y + camera::front.y,
			camera::pos.z + camera::front.z,

			camera::up.x,
			camera::up.y,
			camera::up.z
		);

		/*
		gluLookAt(
			world.camera.position[0],
			world.camera.position[1],
			world.camera.position[2],

			world.camera.lookAt[0],
			world.camera.lookAt[1],
			world.camera.lookAt[2],

			world.camera.up[0],
			world.camera.up[1],
			world.camera.up[2]
		);
		*/
	}

	void renderScene(void) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		render::setCamera();
		render::axes();

		for (const auto& group : world.groups) {
			render::renderGroup(group);
		}

		glutSwapBuffers();
	}
}

#endif