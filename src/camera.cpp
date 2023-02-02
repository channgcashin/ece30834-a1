#define NOMINMAX
#include <iostream>
#include <cmath>
#include "camera.hpp"
#include "scene.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace std;

// constructor
Camera::Camera(const CameraType cType) :
	camType(cType),
	width(1), height(1),
	fovy(45.0f),
	camCoords(0.0f, 0.0f, 10.0f)
	{}

// constructor
Camera::Camera(const glm::vec3 eye, const glm::vec3 center, const glm::vec3 up, const CameraType cType) :
	camCoords(eye),
	camCenter(center),
	camUp(up),
	camType(cType),
	width(1), height(1),
	fovy(45.0f) {
	updateViewProj();
}

void Camera::updateViewProj() {
	float aspect = (float)width / (float)height;
	// ################### TODO1 ###################
	// Task: construct the view matrix using eye, center and up.
	// currently we are using a built-in function "glm::lookAt" to calculate "view" matrix;
	// and in this assignment you need to implement a function (which is "Camera::calCameraMat") to calculate "view" on your own.
	// Steps:
	// 1. implement "Camera::calCameraMat" which returns the "view" matrix; it has arguments: "camCoords", "camCenter" and "camUp".
	// 2. REMOVE the line below, and replace that by calling your own function for computing "view": view = calCameraMat(..., ..., ...);
	//view = glm::lookAt(camCoords, camCenter, camUp);  //  should NOT be used in this assignment
	view = calCameraMat(camCoords, camCenter, camUp);

	if (camType == GROUND_VIEW || camType == OVERHEAD_VIEW) {  // perspective projection
		proj = glm::perspective(glm::radians(fovy), aspect, 0.1f, 100.0f);
	}
	else  // orthographic projection
		proj = glm::ortho(-15.0f, 15.0f, -15.0f, 15.0f, 0.1f, 100.0f);
}

glm::mat4 Camera::calCameraMat(const glm::vec3 eye, const glm::vec3 center, const glm::vec3 up) {  // calculate the camera view matrix
	auto computeNorm = [=](glm::vec3 v) {  // glm::normalize
		float mag = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
		return v / mag;
	};

	auto computeCross = [=](glm::vec3 v1, glm::vec3 v2) {  // glm::cross
		return glm::vec3(
			v1.y * v2.z - v1.z * v2.y,
			v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x
		);
	};

	auto computeDot = [=](glm::vec3 v1, glm::vec3 v2) {  // glm::dot
		return (v1.x * v2.x) + (v1.y * v2.y) + (v1.z * v2.z);
	};

	auto computeTranspose = [=](glm::mat4 mat) {  // glm::transpose
		glm::mat4 res;
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				res[i][j] = mat[j][i];
		return res;
	};

	// ################### TODO2 ###################
	// Task: construct the view matrix using eye, center and up.
	// We already provide you with util functions (see above) for normalization, cross product, dot product and transpose.
	// Steps:
	// 1. Calculate the camera coordinate system using "eye", "center" and "up", represent it using 3 vectors: X, Y and Z.
	// 2. Construct "view" matrix, whose structure is like the matrix below:
	/*
	X.x X.y X.z dot(-X, eye)
	Y.x Y.y Y.z dot(-Y, eye)
	Z.x Z.y Z.z dot(-Z, eye)
	0   0   0   1
	*/
	// 3. Store the result in "res" (below).
	glm::mat4 res = glm::mat4(1.0f);  // view matrix
	glm::vec3 X = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 Y = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 Z = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec4 W = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

	Z.x = eye.x - center.x;
  	Z.y = eye.y - center.y;
  	Z.z = eye.z - center.z;

	Z = computeNorm(Z);
	X = computeNorm(computeCross(up, Z));
	Y = computeNorm(computeCross(Z, X));

	glm::vec4 X_4 = glm::vec4(X, -computeDot(X, eye));
	glm::vec4 Y_4 = glm::vec4(Y, -computeDot(Y, eye));
	glm::vec4 Z_4 = glm::vec4(Z, -computeDot(Z, eye));

	res = glm::mat4(X_4, Y_4, Z_4, W);

	return computeTranspose(res);
}

glm::mat4 Camera::rotate(const float degree, const glm::vec3 axis) {
	// degree: rotation degree
	// axis: which axis to rotate around

	glm::mat4 res = glm::mat4(1.0f);
	// ################### TODO3 ###################
	// Task: implement the function for rotation.
	// Step1: convert degree to radians (you can use "glm::radians").
	// Step2: recall the 3 basic rotation matrices (around x, y and z) you learn in class.
	// Step3: there should be 3 cases, conditioned on which axis to rotate around.
	// Step4: fill in the rotation matrix "res" (above).
	if(axis.x == 1.0f){
		res[1] = glm::vec4(0.0f, cos(glm::radians(degree)), -sin(glm::radians(degree)), 0.0f);
		res[2] = glm::vec4(0.0f, sin(glm::radians(degree)), cos(glm::radians(degree)), 0.0f);
	}else if(axis.y == 1.0f) {
		res[0] = glm::vec4(cos(glm::radians(degree)), 0.0f, -sin(glm::radians(degree)), 0.0f);
		res[2] = glm::vec4(sin(glm::radians(degree)), 0.0f, cos(glm::radians(degree)), 0.0f);
	}else{
		res[0] = glm::vec4(cos(glm::radians(degree)), -sin(glm::radians(degree)), 0.0f, 0.0f);
		res[1] = glm::vec4(sin(glm::radians(degree)), cos(glm::radians(degree)), 0.0f, 0.0f);
	}

	return res;
}

glm::mat4 Camera::translate(const glm::vec3 v) {
	glm::mat4 res = glm::mat4(1.0f);
	res[3] = glm::vec4(v, 1.0f);
	return res;
}

void Camera::turnLeft() {
	if (camType == OVERHEAD_VIEW)  // only work in the ground view
		return;
	
	// ################### TODO4 ###################
	// Task: (part of city roaming) implement the "turning left" function (controlled by keyboard).
	// Remember that the camera should rotate around the current position (to be more specific, about the y axis pointing up),
	//     meaning the camera should not move to another location. It should look at somewhere to the left.
	// Step1: get the rotation matrix using "Camera::rotate" you just implemented and using "rotStep".
	// Step2: update "camCenter".

	updateViewProj();  // update view
}

void Camera::turnRight() {
	if (camType == OVERHEAD_VIEW)  // only work in the ground view
		return;

	// ################### TODO5 ###################
	// Task: (part of city roaming) implement the "turning right" function (controlled by keyboard).
	// Step1: similar to "Camera::turnLeft"

	updateViewProj();  // update view
}

void Camera::moveForward() {
	if (camType == GROUND_VIEW) {  // only work in the ground view
		// ################### TODO6 ###################
		// Task: (part of city roaming) implement the "moving forward" function (controlled by keyboard).
		// Remember that the movement of the camera should be parallel to plane defined by xz.
		// Step1: get the translation matrix using "Camera::translate" provided and "moveStep".
		// Step2: update "camCoords" and "camCenter"; they should be updated in the same way.
	}
	else
		return;
	updateViewProj();  // update view
}

void Camera::moveBackward() {
	if (camType == GROUND_VIEW) {  // only work in the ground view
		// ################### TODO7 ###################
		// Task: (part of city roaming) implement the "moving backward" function (controlled by keyboard).
		// Remember that the movement of the camera should be parallel to plane defined by xz.
		// Step1: similar to "Camera::moveForward".
	}
	else
		return;
	updateViewProj();
}

void Camera::moveUp() {
	if (camType == GROUND_VIEW) {
		glm::vec3 up = glm::vec3(0.0, 1.0, 0.0);  // get the direction vector (pointing up)
		glm::mat4 translation = translate(up * moveStep);  // get the translation matrix
		glm::vec4 temp = translation * glm::vec4(camCoords, 1.0);  // move the camera coordinates up
		camCoords.x = temp.x / temp.w; camCoords.y = temp.y / temp.w; camCoords.z = temp.z / temp.w;  // normalize
		// also move the center
		temp = translation * glm::vec4(camCenter, 1.0);
		camCenter.x = temp.x / temp.w; camCenter.y = temp.y / temp.w; camCenter.z = temp.z / temp.w;  // normalize
	}
	updateViewProj();
}

void Camera::moveDown() {
	if (camType == GROUND_VIEW) {
		glm::vec3 down = glm::vec3(0.0, -1.0, 0.0);  // get the direction vector (pointing down)
		glm::mat4 translation = translate(down * moveStep);  // get the translation matrix
		glm::vec4 temp = translation * glm::vec4(camCoords, 1.0);  // move the camera coordinates down
		camCoords.x = temp.x / temp.w; camCoords.y = temp.y / temp.w; camCoords.z = temp.z / temp.w;  // normalize
		// also move the center
		temp = translation * glm::vec4(camCenter, 1.0);
		camCenter.x = temp.x / temp.w; camCenter.y = temp.y / temp.w; camCenter.z = temp.z / temp.w;  // normalize
	}
	updateViewProj();
}
