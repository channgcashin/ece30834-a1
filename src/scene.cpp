#define NOMINMAX
#include <iostream>
#include <filesystem>
#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "scene.hpp"
using namespace std;
namespace fs = std::filesystem;

void Scene::parseScene() {
	string modelsDir = fs::current_path().string() + "/models/";  // current directory
	string sceneFile = modelsDir + "scene_a1.txt";  // scene file
	ifstream istr(sceneFile);
	try {  // read the file
		istr >> nObj;
		for (int i = 0; i < nObj; i++) {  // for each object
			string objFilename;
			istr >> objFilename;
			glm::mat3 rotMat;  // rotation matrix
			glm::vec3 translation;  // translation vector
			for (int j = 0; j < 3; j++) {
				for (int k = 0; k < 3; k++) {
					istr >> rotMat[j][k];
				}
			}
			for (int j = 0; j < 3; j++)
				istr >> translation[j];

			glm::mat4 modelMat = calModelMat(rotMat, translation);  // model matrix
			auto mesh = std::make_shared<Mesh>(modelsDir + objFilename);  // construct the mesh
			mesh->setModelMat(modelMat);
			objects.push_back(mesh);  // store the mesh
		}
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;  // fail to open the file
	}
}

glm::mat4 Scene::calModelMat(const glm::mat3 rotMat, const glm::vec3 translation) {
	glm::mat4 modelMat = glm::mat4(1.0);  // initialize the matrix as an identity matrix
	glm::mat4 rotateMat = glm::mat4(1.0);
	glm::mat4 translateMat = glm::mat4(1.0);
	// copy
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			rotateMat[i][j] = rotMat[i][j];
		}
	}
	for (int i = 0; i < 3; i++)
		translateMat[i][3] = translation[i];
	modelMat = glm::transpose(translateMat) * glm::transpose(rotateMat);
	return modelMat;
}

void Scene::printMat3(const glm::mat3 mat) {
	for (int i = 0; i < 3; i++) {  // copy the rotation matrix
		for (int j = 0; j < 3; j++) {
			std::cout << mat[i][j] << " ";
		}
		std::cout << "\n";
	}
	std::cout << std::endl;
}

void Scene::printMat4(const glm::mat4 mat) {
	for (int i = 0; i < 4; i++) {  // copy the rotation matrix
		for (int j = 0; j < 4; j++) {
			std::cout << mat[i][j] << " ";
		}
		std::cout << "\n";
	}
	std::cout << std::endl;
}
