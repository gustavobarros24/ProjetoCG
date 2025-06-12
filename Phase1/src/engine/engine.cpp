
#include "Model.h"
#include "WorldConfig.h"
#include "tinyXML2/tinyxml2.h"

#include <array>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

const GLenum polygonModes[] = { GL_FILL, GL_LINE, GL_POINT };
int currentPolygonMode = 0;

float currentPosX = 0.0f;
float currentPosY = 0.0f;
float currentPosZ = 0.0f;
float stepPos = 0.1f;

float currentAngle = 0.0f;
float stepAngle = 1.0f;

WorldConfig config("config.xml");

void changeSize(int w, int h)
{
	// Prevent a divide by zero, when window is too short
	// (you can’t make a window with zero width).
	if (h == 0)
		h = 1;
	
	// compute window's aspect ratio
	float ratio = w * 1.0f / h;
	
	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	
	// Load the identity matrix
	glLoadIdentity();
	
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);
	
	// Set the perspective
	gluPerspective(config.cameraFOV, ratio, config.cameraNearClip, config.cameraFarClip);
	
	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}

void drawAxes() {

	Vec3 origin = { 0.0f, 0.0f, 0.0f };

	std::array<Vec3, 3> axisHeads = {
		Vec3(100.0f, 0.0f, 0.0f), // x
		Vec3(0.0f, 100.0f, 0.0f), // y
		Vec3(0.0f, 0.0f, 100.0f), // z
	};

	std::array<Vec3, 3> axisColours = {
		Vec3(1.0f, 0.0f, 0.0f), // x in red
		Vec3(0.0f, 1.0f, 0.0f), // y in green
		Vec3(0.0f, 0.0f, 1.0f)  // z in blue
	};

	glPushAttrib(GL_CURRENT_BIT);
	glBegin(GL_LINES);

	for (int i = 0; i < 3; i++) {
		glColor3f(axisColours[i].x, axisColours[i].y, axisColours[i].z);
		glVertex3f(origin.x, origin.y, origin.z);
		glVertex3f(axisHeads[i].x, axisHeads[i].y, axisHeads[i].z);
	}

	glEnd();
	glPopAttrib();
}

void drawModels() {
	glPushMatrix();
	glTranslatef(currentPosX, currentPosY, currentPosZ);
	glRotatef(currentAngle, 0.0f, 0.0f, 1.0f);

	glBegin(GL_TRIANGLES);
	for (const auto& group : config.groups) {
		for (const auto& model : group.models) {
			for (const auto& vertex : model.vertices) {
				glVertex3f(vertex.x, vertex.y, vertex.z);
			}
		}
	}

	glEnd();

	glPopMatrix();
}

void renderScene(void)
{
	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set camera
	glLoadIdentity();
	gluLookAt(
		config.cameraPos[0],
		config.cameraPos[1],
		config.cameraPos[2],
		
		config.cameraLookAt[0],
		config.cameraLookAt[1],
		config.cameraLookAt[2],
		
		config.cameraUp[0],
		config.cameraUp[1],
		config.cameraUp[2]
	);

	drawAxes();
	drawModels();

	glutSwapBuffers();
}

void processKeyboardSpecial(int key_code, int x, int y) {

	switch (key_code) {
	case GLUT_KEY_LEFT:
		currentPosX -= stepPos;
		break;
	case GLUT_KEY_RIGHT:
		currentPosX += stepPos;
		break;
	case GLUT_KEY_UP:
		currentPosZ -= stepPos;
		break;
	case GLUT_KEY_DOWN:
		currentPosZ += stepPos;
		break;
	}

	glutPostRedisplay();
}

void processKeyboard(unsigned char key, int x, int y) {

	switch (key) {

		// rotation
	case 'q':
	case 'Q':
		currentAngle += stepAngle;
		break;
	case 'e':
	case 'E':
		currentAngle -= stepAngle;
		break;

		// cycle through polygon modes
	case ' ':
		currentPolygonMode = (currentPolygonMode + 1) % 3;
		glPolygonMode(GL_FRONT_AND_BACK, polygonModes[currentPolygonMode]);
		break;
	}

	glutPostRedisplay();
}

void printXML() {
	try {

		// Access parsed data
		std::cout << "Window Width: " << config.windowWidth << std::endl;
		std::cout << "Window Height: " << config.windowHeight << std::endl;
		std::cout << "Camera Position: (" << config.cameraPos[0] << ", " << config.cameraPos[1] << ", " << config.cameraPos[2] << ")" << std::endl;
		std::cout << "Camera LookAt: (" << config.cameraLookAt[0] << ", " << config.cameraLookAt[1] << ", " << config.cameraLookAt[2] << ")" << std::endl;
		std::cout << "Camera Up: (" << config.cameraUp[0] << ", " << config.cameraUp[1] << ", " << config.cameraUp[2] << ")" << std::endl;
		std::cout << "Camera FOV: " << config.cameraFOV << std::endl;
		std::cout << "Camera Near Clip: " << config.cameraNearClip << std::endl;
		std::cout << "Camera Far Clip: " << config.cameraFarClip << std::endl;

		// Access groups and models
		for (const auto& group : config.groups) {
			for (const auto& model : group.models) {
				std::cout << "Model has " << model.vertices.size() << " vertices." << std::endl;
			}
		}
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

int main(int argc, char** argv)
{
	printXML();

	// put GLUT’s init here
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(config.windowWidth, config.windowHeight);
	glutCreateWindow("Engine");
	
	// put callback registry here
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene);
	glutDisplayFunc(renderScene);
	glutSpecialFunc(processKeyboardSpecial);
	glutKeyboardFunc(processKeyboard);
	
	// some OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	// enter GLUT’s main cycle
	glutMainLoop();
	
	return 1;
}