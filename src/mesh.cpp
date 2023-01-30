#define NOMINMAX
#include "mesh.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

// Helper functions
int indexOfNumberLetter(std::string& str, int offset);
int lastIndexOfNumberLetter(std::string& str);
std::vector<std::string> split(const std::string &s, char delim);

// Constructor - load mesh from file
Mesh::Mesh(std::string filename, bool keepLocalGeometry) {
	minBB = glm::vec3(std::numeric_limits<float>::max());
	maxBB = glm::vec3(std::numeric_limits<float>::lowest());

	modelMat = glm::mat4(1.0f);  // initialize with an identity matrix

	vao = 0;
	vbuf = 0;
	vcount = 0;
	//loadOBJ(filename, keepLocalGeometry);
	loadPLY(filename, keepLocalGeometry);
}

// Draw the mesh
void Mesh::draw() {
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vcount);
	glBindVertexArray(0);
}

// Load a wavefront OBJ file
void Mesh::loadOBJ(std::string filename, bool keepLocalGeometry) {
	// Release resources
	release();

	std::ifstream file(filename);
	if (!file.is_open()) {
		std::stringstream ss;
		ss << "Error reading " << filename << ": failed to open file";
		throw std::runtime_error(ss.str());
	}

	// Store vertex and normal data while reading
	std::vector<glm::vec3> raw_vertices;
	std::vector<glm::vec3> raw_normals;
	std::vector<unsigned int> v_elements;
	std::vector<unsigned int> n_elements;

	std::string line;
	while (getline(file, line)) {
		if (line.substr(0, 2) == "v ") {
			// Read position data
			int index1 = indexOfNumberLetter(line, 2);
			int index2 = lastIndexOfNumberLetter(line);
			std::vector<std::string> values = split(line.substr(index1, index2 - index1 + 1), ' ');
			glm::vec3 vert(stof(values[0]), stof(values[1]), stof(values[2]));
			raw_vertices.push_back(vert);

			// Update bounding box
			minBB = glm::min(minBB, vert);
			maxBB = glm::max(maxBB, vert);
		} else if (line.substr(0, 3) == "vn ") {
			// Read normal data
			int index1 = indexOfNumberLetter(line, 2);
			int index2 = lastIndexOfNumberLetter(line);
			std::vector<std::string> values = split(line.substr(index1, index2 - index1 + 1), ' ');
			raw_normals.push_back(glm::vec3(stof(values[0]), stof(values[1]), stof(values[2])));

		} else if (line.substr(0, 2) == "f ") {
			// Read face data
			int index1 = indexOfNumberLetter(line, 2);
			int index2 = lastIndexOfNumberLetter(line);
			std::vector<std::string> values = split(line.substr(index1, index2 - index1 + 1), ' ');
			for (int i = 0; i < int(values.size()) - 2; i++) {
				// Split up vertex indices
				std::vector<std::string> v1 = split(values[0], '/');		// Triangle fan for ngons
				std::vector<std::string> v2 = split(values[i+1], '/');
				std::vector<std::string> v3 = split(values[i+2], '/');

				// Store position indices
				v_elements.push_back(stoul(v1[0]) - 1);
				v_elements.push_back(stoul(v2[0]) - 1);
				v_elements.push_back(stoul(v3[0]) - 1);

				// Check for normals
				if (v1.size() >= 3 && v1[2].length() > 0) {
					n_elements.push_back(stoul(v1[2]) - 1);
					n_elements.push_back(stoul(v2[2]) - 1);
					n_elements.push_back(stoul(v3[2]) - 1);
				}
			}
		}
	}
	file.close();

	// Check if the file was invalid
	if (raw_vertices.empty() || v_elements.empty()) {
		std::stringstream ss;
		ss << "Error reading " << filename << ": invalid file or no geometry";
		throw std::runtime_error(ss.str());
	}

	// Create vertex array
	vertices = std::vector<Vertex>(v_elements.size());
	for (int i = 0; i < int(v_elements.size()); i += 3) {
		// Store positions
		vertices[i+0].pos = raw_vertices[v_elements[i+0]];
		vertices[i+1].pos = raw_vertices[v_elements[i+1]];
		vertices[i+2].pos = raw_vertices[v_elements[i+2]];

		// Check for normals
		if (n_elements.size() > 0) {
			// Store normals
			vertices[i+0].norm = raw_normals[n_elements[i+0]];
			vertices[i+1].norm = raw_normals[n_elements[i+1]];
			vertices[i+2].norm = raw_normals[n_elements[i+2]];
		} else {
			// Calculate normal
			glm::vec3 normal = normalize(cross(vertices[i+1].pos - vertices[i+0].pos,
				vertices[i+2].pos - vertices[i+0].pos));
			vertices[i+0].norm = normal;
			vertices[i+1].norm = normal;
			vertices[i+2].norm = normal;
		}
	}
	vcount = (GLsizei)vertices.size();

	// Load vertices into OpenGL
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbuf);
	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Delete local copy of geometry
	if (!keepLocalGeometry)
		vertices.clear();
}

// load a model stored in .ply format
void Mesh::loadPLY(std::string filename, bool keepLocalGeometry /* = false */) {
	// Release resources
	release();

	std::ifstream file(filename);
	if (!file.is_open()) {
		std::stringstream ss;
		ss << "Error reading " << filename << ": failed to open file";
		throw std::runtime_error(ss.str());
	}

	// Store vertex and normal data while reading
	std::vector<glm::vec3> raw_vertices;
	std::vector<glm::vec3> raw_normals;
	std::vector<unsigned int> v_elements;  // index starts from 0
	std::vector<unsigned int> n_elements;
	std::vector<glm::vec3> colors;  // color of each vertex; each consecutive three colors belong to the same face and should have the same color

	std::string line;
	int n_vertex, n_face;
	// 1. read number of vertices and number of faces from ply header
	// 2. when reaching "end_header", start reading the vertex data
	// set up two flags
	// flag_vertex_section: whether enter the vertex data section
	// flag_face_section: whether enter the face data section
	bool flag_vertex_section = false, flag_face_section = false;
	// count the lines when we are in the vertex data section, so that we know when we will enter the face data section
	int line_counter = 0;
	while (getline(file, line)) {
		if (line.find("element vertex") != std::string::npos) {
			std::vector<std::string> values = split(line, ' ');
			n_vertex = stoi(values[2]);
		}
		if (line.find("element face") != std::string::npos) {
			std::vector<std::string> values = split(line, ' ');
			n_face = stoi(values[2]);
		}
		if (line.find("end_header") != std::string::npos) {
			flag_vertex_section = true;
			continue;
		}
		if (flag_vertex_section) {
			std::vector<std::string> values = split(line, ' ');
			glm::vec3 vert(stof(values[0]), stof(values[1]), stof(values[2]));  // the first 3 values of this line are coordinates of the vertex
			raw_vertices.push_back(vert);
			raw_normals.push_back(glm::vec3(stof(values[3]), stof(values[4]), stof(values[5])));  // the last 3 values of this line are normals
			line_counter++;

			// Update bounding box
			minBB = glm::min(minBB, vert);
			maxBB = glm::max(maxBB, vert);

			if (line_counter == n_vertex) {
				flag_vertex_section = false;
				flag_face_section = true;
				continue;
			}
		}
		if (flag_face_section) {
			std::vector<std::string> values = split(line, ' ');

			if (stoul(values[0]) != 3) {
				std::stringstream ss;
				ss << "Error reading " << filename << ": only supports triangle meshes.";
				throw std::runtime_error(ss.str());
			}

			v_elements.push_back(stoul(values[1]));
			v_elements.push_back(stoul(values[2]));
			v_elements.push_back(stoul(values[3]));
			n_elements.push_back(stoul(values[1]));
			n_elements.push_back(stoul(values[2]));
			n_elements.push_back(stoul(values[3]));
			glm::vec3 color(
				stof(values[4]) / 255.0,
				stof(values[5]) / 255.0,
				stof(values[6]) / 255.0
			);
			colors.push_back(color);
			colors.push_back(color);
			colors.push_back(color);
		}
	}
	// std::cout << "num of vertex: " << n_vertex << ", num of face: " << n_face << std::endl;
	file.close();

	// Check if the file was invalid
	if (raw_vertices.empty() || v_elements.empty()) {
		std::stringstream ss;
		ss << "Error reading " << filename << ": invalid file or no geometry";
		throw std::runtime_error(ss.str());
	}

	// Create vertex array
	vertices = std::vector<Vertex>(v_elements.size());
	for (int i = 0; i < int(v_elements.size()); i += 3) {  // traverse each face (of 3 vertices)
		// Store positions
		vertices[i + 0].pos = raw_vertices[v_elements[i + 0]];
		vertices[i + 1].pos = raw_vertices[v_elements[i + 1]];
		vertices[i + 2].pos = raw_vertices[v_elements[i + 2]];

		vertices[i + 0].color = colors[i + 0];
		vertices[i + 1].color = colors[i + 1];
		vertices[i + 2].color = colors[i + 2];

		// Check for normals
		if (n_elements.size() > 0) {
			// Store normals
			vertices[i + 0].norm = raw_normals[n_elements[i + 0]];
			vertices[i + 1].norm = raw_normals[n_elements[i + 1]];
			vertices[i + 2].norm = raw_normals[n_elements[i + 2]];
		}
		else {
			// Calculate normal
			glm::vec3 normal = normalize(cross(vertices[i + 1].pos - vertices[i + 0].pos,
				vertices[i + 2].pos - vertices[i + 0].pos));
			vertices[i + 0].norm = normal;
			vertices[i + 1].norm = normal;
			vertices[i + 2].norm = normal;
		}
	}
	vcount = (GLsizei)vertices.size();

	// Load vertices into OpenGL
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbuf);
	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(glm::vec3));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(glm::vec3)));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Delete local copy of geometry
	if (!keepLocalGeometry)
		vertices.clear();
}

// Release resources
void Mesh::release() {
	minBB = glm::vec3(std::numeric_limits<float>::max());
	maxBB = glm::vec3(std::numeric_limits<float>::lowest());

	vertices.clear();
	if (vao) { glDeleteVertexArrays(1, &vao); vao = 0; }
	if (vbuf) { glDeleteBuffers(1, &vbuf); vbuf = 0; }
	vcount = 0;
}

int indexOfNumberLetter(std::string& str, int offset) {
	for (int i = offset; i < int(str.length()); ++i) {
		if ((str[i] >= '0' && str[i] <= '9') || str[i] == '-' || str[i] == '.') return i;
	}
	return (int)str.length();
}
int lastIndexOfNumberLetter(std::string& str) {
	for (int i = int(str.length()) - 1; i >= 0; --i) {
		if ((str[i] >= '0' && str[i] <= '9') || str[i] == '-' || str[i] == '.') return i;
	}
	return 0;
}
std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;

	std::stringstream ss(s);
    std::string item;
    while (getline(ss, item, delim)) {
        elems.push_back(item);
    }

    return elems;
}
