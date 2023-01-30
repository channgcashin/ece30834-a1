#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "gl_core_3_3.h"

enum CameraType {
	GROUND_VIEW,   // front view
	OVERHEAD_VIEW  // birds view
};

class Camera {
public:
	// Ctor and Dtor:
	Camera(const CameraType cType = GROUND_VIEW);
	Camera(const glm::vec3 eye, const glm::vec3 center, const glm::vec3 up, const CameraType cType = GROUND_VIEW);
	~Camera() {}

	// keyboard control
	void turnLeft();
	void turnRight();
	void moveForward();
	void moveBackward();
	void moveUp();
	void moveDown();

	// access methods:
	inline void setWH(const int w, const int h) {  // update width and height
		width = w;
		height = h;
		updateViewProj();
	}
	inline int getW() { return width; }
	inline int getH() { return height; }
	inline float getFovy() { return fovy; }
	inline glm::vec3 getCoords() { return camCoords; }
	void setCoords(const GLfloat x, const GLfloat y, const GLfloat z) {
		camCoords.x = x;
		camCoords.y = y;
		camCoords.z = z;
	}
	inline glm::mat4 getView() { return view; }
	inline glm::mat4 getProj() { return proj; }

protected:
	// Camera state
	CameraType camType;			// of the two types
	int width, height;			// Width and height of the window
	float fovy;					// Vertical field of view in degrees
	glm::vec3 camCoords;		// Camera coordinates
	glm::vec3 camCenter;		// the position the camera is looking at; used in glm::lookAt()
	glm::vec3 camUp;			// up vector; used in glm::lookAt()
	glm::mat4 view, proj;		// view and projection matrices

	GLfloat rotStep = 2.0f;   // rotation step
	GLfloat moveStep = 0.2f;  // moving step

	void updateViewProj();  // update view and projection matrices whenever they change
	glm::mat4 calCameraMat(const glm::vec3 eye, const glm::vec3 center, const glm::vec3 up);  // calculate the camera view matrix
	// computations:
	glm::mat4 rotate(const float degree, const glm::vec3 axis);
	glm::mat4 translate(const glm::vec3 v);
};

#endif
