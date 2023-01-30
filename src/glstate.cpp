#define NOMINMAX
#include <iostream>
#include "glstate.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "util.hpp"

// Constructor
GLState::GLState() :  // initialize all variables
	shader(0),
	xformLoc(0),
	vao(0),
	vbuf(0),
	ibuf(0),
	vcount(0),
	cam_ground(
		glm::vec3(0.0, 1.5, 20.0),	// eye (position of the camera)
		glm::vec3(0.0, 0.0, 0.0),	// center (the point the camera is looking at)
		glm::vec3(0.0, 1.0, 0.0),	// up vector
		GROUND_VIEW),  // the ground view camera
	cam_overhead(
		glm::vec3(0.0, 20.0, 25.0),	// camera spherical coordinates
		glm::vec3(0.0, 0.0, 0.0),	// not used
		glm::vec3(0.0, 0.0, 1.0),	// not used
		OVERHEAD_VIEW)  // the overhead camera
	{}

// Destructor
GLState::~GLState() {
	// Release OpenGL resources
	if (shader)	glDeleteProgram(shader);
	if (vao)	glDeleteVertexArrays(1, &vao);
	if (vbuf)	glDeleteBuffers(1, &vbuf);
	if (ibuf)	glDeleteBuffers(1, &ibuf);
}

// Called when OpenGL context is created (some time after construction)
void GLState::initializeGL() {
	// General settings
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);

	// Initialize OpenGL state
	initShaders();
	showScene();  // start
}

// Called when window requests a screen redraw
void GLState::paintGL() {
	// Clear the color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set shader to draw with
	glUseProgram(shader);

	// Construct a transformation matrix for the camera
	glm::mat4 xform(1.0f), proj, view;

	auto objects = scene->getSceneObjects();  // get all objects to render in the scene
	for (auto& meshObj : objects) {
		glm::mat4 modelMat = meshObj->getModelMat();
		proj = (whichCam == GROUND_VIEW) ? cam_ground.getProj() : cam_overhead.getProj();
		view = (whichCam == GROUND_VIEW) ? cam_ground.getView() : cam_overhead.getView();
		xform = proj * view * modelMat;  // opengl does matrix multiplication from right to left

		glm::mat4 viewProjMat(1.0f);
		if (getCamType() == OVERHEAD_VIEW) {  // only the overhead view supports the trackball feature
			// Perspective projection
			float aspect = (float)(cam_overhead.getW()) / (float)(cam_overhead.getH());  // aspect ratio
			glm::mat4 proj = glm::perspective(glm::radians(cam_overhead.getFovy()), aspect, 0.1f, 100.0f);  // projection matrix
			// Camera viewpoint
			glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -cam_overhead.getCoords().z));  // camera view matrix
			view = glm::rotate(view, glm::radians(cam_overhead.getCoords().y), glm::vec3(1.0f, 0.0f, 0.0f));
			view = glm::rotate(view, glm::radians(cam_overhead.getCoords().x), glm::vec3(0.0f, 1.0f, 0.0f));
			// Combine transformations
			viewProjMat = proj * view;
			xform = viewProjMat * modelMat;
		}

		glUniformMatrix4fv(xformLoc, 1, GL_FALSE, glm::value_ptr(xform));
		// Draw the mesh
		meshObj->draw();
	}

	glUseProgram(0);
}

// Called when window is resized
void GLState::resizeGL(int w, int h) {
	// Tell OpenGL the new dimensions of the window
	cam_ground.setWH(w, h);
	glViewport(0, 0, w, h);
}

void GLState::showScene() {
	scene = std::unique_ptr<Scene>(new Scene());
	scene->parseScene();  // read the scene file and load the meshes of the objects
}

// Create shaders and associated state
void GLState::initShaders() {
	// Compile and link shader files
	std::vector<GLuint> shaders;
	shaders.push_back(compileShader(GL_VERTEX_SHADER, "shaders/v.glsl"));
	shaders.push_back(compileShader(GL_FRAGMENT_SHADER, "shaders/f.glsl"));
	shader = linkProgram(shaders);
	// Cleanup extra state
	for (auto s : shaders)
		glDeleteShader(s);
	shaders.clear();

	// Get uniform locations
	xformLoc = glGetUniformLocation(shader, "xform");
}

// Start rotating the camera (click + drag)
void GLState::beginCameraRotate(glm::vec2 mousePos) {
	if (getCamType() == OVERHEAD_VIEW) {  // only the overhead camera supports tractball feature
		camRotating = true;
		initCamRot = glm::vec2(cam_overhead.getCoords());
		initMousePos = mousePos;
	}
}

// Stop rotating the camera (mouse button is released)
void GLState::endCameraRotate() {
	if (getCamType() == OVERHEAD_VIEW) {
		camRotating = false;
	}
}

// Use mouse delta to determine new camera rotation
void GLState::rotateCamera(glm::vec2 mousePos) {
	if (camRotating && getCamType() == OVERHEAD_VIEW) {
		float rotScale = glm::min(cam_overhead.getW() * 1000 / 450.0f, cam_overhead.getH() * 1000 / 270.0f);
		glm::vec2 mouseDelta = mousePos - initMousePos;
		glm::vec2 newAngle = initCamRot + mouseDelta / rotScale;
		newAngle.y = glm::clamp(newAngle.y, -90.0f, 90.0f);
		while (newAngle.x > 180.0f) newAngle.x -= 360.0f;
		while (newAngle.x < -180.0f) newAngle.x += 360.0f;
		if (glm::length(newAngle - glm::vec2(cam_overhead.getCoords())) > FLT_EPSILON) {
			cam_overhead.setCoords(newAngle.x, newAngle.y, cam_overhead.getCoords().z);
		}
	}
}

// Moves the camera toward / away from the origin (scroll wheel)
void GLState::offsetCamera(float offset) {
	if (getCamType() == OVERHEAD_VIEW) {
		// ################### TODO8 ###################
		// Task: implement this function so that when using the scroll wheel, the camera will move closer / farther to the scene.
		// Step1: update only z value of "camCoords" of the overhead camera "cam_overhead".
		//        remember to set two cutoff values (near and far), so the camera cannot be moved too close / too far.
	}
}
