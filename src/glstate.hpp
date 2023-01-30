#ifndef GLSTATE_HPP
#define GLSTATE_HPP

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "gl_core_3_3.h"
#include "mesh.hpp"
#include "camera.hpp"
#include "scene.hpp"

// Manages OpenGL state, e.g. camera transform, objects, shaders
class GLState {
public:
	GLState();
	~GLState();
	// Disallow copy, move, & assignment
	GLState(const GLState& other) = delete;
	GLState& operator=(const GLState& other) = delete;
	GLState(GLState&& other) = delete;
	GLState& operator=(GLState&& other) = delete;

	// Callbacks
	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);

	// Set object to display
	void showScene();

	// Per-vertex attributes
	struct Vertex {
		glm::vec3 pos;		// Position
		glm::vec3 norm;		// Normal
	};

	// which camera is currently active
	inline CameraType getCamType() { return whichCam; }
	// get the camera instance
	inline Camera& getCamera(const CameraType camType = GROUND_VIEW) {
		return (camType == GROUND_VIEW) ? cam_ground : cam_overhead;
	}
	inline void switchCam() {  // switch between the two cameras
		whichCam = (whichCam == GROUND_VIEW) ? OVERHEAD_VIEW : GROUND_VIEW;
	}

	// camera control
	bool isCamRotating() const { return camRotating; }  // is the camera currently rotating (mouse dragging)
	void beginCameraRotate(glm::vec2 mousePos);
	void endCameraRotate();
	void rotateCamera(glm::vec2 mousePos);  // update view during camera rotation
	void offsetCamera(float offset);  // use the scroll wheel to move closer / farther
	inline float getMoveStep() { return moveStep; }

protected:
	// Initialization
	void initShaders();

	std::string meshFilename;		// Name of the obj file being shown
	std::unique_ptr<Mesh> mesh;		// Pointer to mesh object
	std::unique_ptr<Scene> scene;   // Pointer to the scene object

	// OpenGL state
	GLuint shader;		// GPU shader program
	GLuint xformLoc;	// Transformation matrix location
	GLuint vao;			// Vertex array object
	GLuint vbuf;		// Vertex buffer
	GLuint ibuf;		// Index buffer
	GLsizei vcount;		// Number of indices to draw

	// cameras:
	Camera cam_ground, cam_overhead;
	CameraType whichCam = GROUND_VIEW;  // which camera is active currently
	float moveStep = 0.1f;  // (translation step) moves the camera toward / away from the origin (scroll wheel)
	bool camRotating;		// Whether camera is currently rotating
	glm::vec2 initCamRot;	// Initial camera rotation on click
	glm::vec2 initMousePos;	// Initial mouse position on click
};

#endif
