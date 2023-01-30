#include <iostream>
#include <memory>
#include <filesystem>
#include <algorithm>
#include "glstate.hpp"
#include <GL/freeglut.h>
namespace fs = std::filesystem;

// Menu identifiers
const int MENU_EXIT = 1;					// Exit application
std::vector<std::string> meshFilenames;		// Paths to .obj files to load

// OpenGL state
std::unique_ptr<GLState> glState;

// Initialization functions
void initGLUT(int* argc, char** argv);
void initMenu();
void findObjFiles();

// Callback functions
void display();
void reshape(GLint width, GLint height);
void keyRelease(unsigned char key, int x, int y);
void keyPress(unsigned char key, int x, int y);
void mouseBtn(int button, int state, int x, int y);
void mouseMove(int x, int y);
void idle();
void menu(int cmd);
void cleanup();

// Program entry point
int main(int argc, char** argv) {
	try {
		// Create the window and menu
		initGLUT(&argc, argv);
		initMenu();
		// Initialize OpenGL (buffers, shaders, etc.)
		glState = std::unique_ptr<GLState>(new GLState());
		glState->initializeGL();

	} catch (const std::exception& e) {
		// Handle any errors
		std::cerr << "Fatal error: " << e.what() << std::endl;
		cleanup();
		return -1;
	}

	std::cout << "Mouse controls (can only do this when using the overhead camera):" << std::endl;
	std::cout << "  Left click + drag to rotate camera" << std::endl;
	std::cout << "  Scroll wheel to zoom in/out" << std::endl;
	std::cout << "Keyboard controls (can only do this when using the ground camera):" << std::endl;
	std::cout << "  A:  Turn left" << std::endl;
	std::cout << "  D:  Turn right" << std::endl;
	std::cout << "  W:  Move forward" << std::endl;
	std::cout << "  X:  Move backward" << std::endl;
	std::cout << "  Z:  Move down" << std::endl;
	std::cout << "  C:  Move up" << std::endl;
	std::cout << "  S:  Switch between the two cameras (a ground camera and an overhead camera)" << std::endl;
	std::cout << std::endl;

	// Execute main loop
	glutMainLoop();

	return 0;
}

// Setup window and callbacks
void initGLUT(int* argc, char** argv) {
	// Set window and context settings
	int width = 800; int height = 800;
	glutInit(argc, argv);
	glutInitWindowSize(width, height);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	// Create the window
	glutCreateWindow("FreeGLUT Window");

	// Create a menu

	// GLUT callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardUpFunc(keyRelease);
	glutKeyboardFunc(keyPress);
	glutMouseFunc(mouseBtn);
	glutMotionFunc(mouseMove);
	glutIdleFunc(idle);
	glutCloseFunc(cleanup);
}

void initMenu() {
	// Create a submenu with all the objects you can view
	findObjFiles();
	int objmenu = glutCreateMenu(menu);

	// Create the main menu, adding the objects menu as a submenu
	glutCreateMenu(menu);
	glutAddMenuEntry("Exit", MENU_EXIT);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

}

void findObjFiles() {
	// Search the models/ directory for any file ending in .obj
	fs::path modelsDir = "models";
	for (auto& di : fs::directory_iterator(modelsDir)) {
		if (di.is_regular_file() && di.path().extension() == ".obj")
			meshFilenames.push_back(di.path().string());
	}
	std::sort(meshFilenames.begin(), meshFilenames.end());
}

// Called whenever a screen redraw is requested
void display() {
	// Tell the GLState to render the scene
	glState->paintGL();

	// Scene is rendered to the back buffer, so swap the buffers to display it
	glutSwapBuffers();
}

// Called when the window is resized
void reshape(GLint width, GLint height) {
	// Tell OpenGL the new window size
	glState->resizeGL(width, height);
}

// Called when a key is released
void keyRelease(unsigned char key, int x, int y) {
	switch (key) {
	case 27:	// Escape key
		menu(MENU_EXIT);
		break;
	}
}

void keyPress(unsigned char key, int x, int y) {
	switch (key) {
	case 's':  // switch camera
		glState->switchCam();
		glutPostRedisplay();	// Request redraw
		break;
	case 'a':  // turn left
		glState->getCamera(glState->getCamType()).turnLeft();
		glutPostRedisplay();	// Request redraw
		break;
	case 'd':  // turn right
		glState->getCamera(glState->getCamType()).turnRight();
		glutPostRedisplay();	// Request redraw
		break;
	case 'w':  // move forward
		glState->getCamera(glState->getCamType()).moveForward();
		glutPostRedisplay();
		break;
	case 'x':  // move backward
		glState->getCamera(glState->getCamType()).moveBackward();
		glutPostRedisplay();
		break;
	case 'c':  // move up
		glState->getCamera(glState->getCamType()).moveUp();
		glutPostRedisplay();
		break;
	case 'z':  // move down
		glState->getCamera(glState->getCamType()).moveDown();
		glutPostRedisplay();
		break;
	}
}

// Called when a mouse button is pressed or released
void mouseBtn(int button, int state, int x, int y) {
	// Press left mouse button
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		glState->beginCameraRotate(glm::vec2(x, y));
	}
	// Release left mouse button
	if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
		// Stop camera rotation
		glState->endCameraRotate();
	}
	// Scroll wheel up
	if (button == 3) {
		// "Zoom in" otherwise
		glState->offsetCamera(-0.1f);
		glutPostRedisplay();
	}
	// Scroll wheel down
	if (button == 4) {
		// "Zoom out" otherwise
		glState->offsetCamera(0.1f);
		glutPostRedisplay();
	}
}

// Called when the mouse moves
void mouseMove(int x, int y) {
	if (glState->isCamRotating()) {
		// Rotate the camera if currently rotating
		glState->rotateCamera(glm::vec2(x, y));
		glutPostRedisplay();	// Request redraw
	}
}

// Called when there are no events to process
void idle() {
	// TODO: anything that happens every frame (e.g. movement) should be done here
	// Be sure to call glutPostRedisplay() if the screen needs to update as well
}

// Called when a menu button is pressed
void menu(int cmd) {
	switch (cmd) {
	// End the program
	case MENU_EXIT:
		glutLeaveMainLoop();
		break;
	}
}

// Called when the window is closed or the event loop is otherwise exited
void cleanup() {
	// Delete the GLState object, calling its destructor,
	// which releases the OpenGL objects
	glState.reset(nullptr);
}
