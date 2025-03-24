#include <iostream>
#include <vector>
#include "TriangleMesh.h"
#include "Application.h"

#include <Eigen/SVD>

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#define AVG_METHOD 0
#define QEM_METHOD 1

using namespace std;


TriangleMesh::TriangleMesh()
{
	vao = -1;
	vbo = -1;
}

TriangleMesh::~TriangleMesh()
{
	free();
}


void TriangleMesh::addVertex(const glm::vec3 &position)
{
	vertices.push_back(position);
}

void TriangleMesh::addTriangle(int v0, int v1, int v2)
{
	triangles.push_back(v0);
	triangles.push_back(v1);
	triangles.push_back(v2);
}

void TriangleMesh::initVertices(const vector<float> &newVertices)
{
	vertices.resize(newVertices.size() / 3);
	for(unsigned int i=0; i<vertices.size(); i++)
		vertices[i] = glm::vec3(newVertices[3*i], newVertices[3*i+1], newVertices[3*i+2]);
}

void TriangleMesh::initTriangles(const vector<int> &newTriangles)
{
	triangles = newTriangles;
}

void TriangleMesh::buildCube()
{
	float vertices[] = {-1, -1, -1,
                      1, -1, -1,
                      1,  1, -1,
                      -1,  1, -1,
                      -1, -1,  1,
                      1, -1,  1,
                      1,  1,  1,
                      -1,  1,  1
	};

	int faces[] = {3, 1, 0, 3, 2, 1,
                 5, 6, 7, 4, 5, 7,
                 7, 3, 0, 0, 4, 7,
                 1, 2, 6, 6, 5, 1,
                 0, 1, 4, 5, 4, 1,
                 2, 3, 7, 7, 6, 2
	};

	int i;

	for(i=0; i<8; i+=1)
		addVertex(0.5f * glm::vec3(vertices[3*i], vertices[3*i+1], vertices[3*i+2]));
	for(i=0; i<12; i++)
		addTriangle(faces[3*i], faces[3*i+1], faces[3*i+2]);
}

const vector<glm::vec3> &TriangleMesh::getVertices() const
{
	return vertices;
}

vector<glm::vec3> &TriangleMesh::getVertices()
{
	return vertices;
}

const vector<int> &TriangleMesh::getTriangles() const
{
	return triangles;
}

vector<int> &TriangleMesh::getTriangles()
{
	return triangles;
}

void TriangleMesh::sendToOpenGL()
{
	vector<float> data;
	
	for(unsigned int tri=0; tri<triangles.size(); tri+=3)
	{
		glm::vec3 normal = computeNormalTriangle(tri);
		for(unsigned int vrtx=0; vrtx<3; vrtx++)
		{
			data.push_back(vertices[triangles[tri + vrtx]].x);
			data.push_back(vertices[triangles[tri + vrtx]].y);
			data.push_back(vertices[triangles[tri + vrtx]].z);

			data.push_back(normal.x);
			data.push_back(normal.y);
			data.push_back(normal.z);
		}
	}

	// Send data to OpenGL
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
	posLocation = Application::instance().getShader()->bindVertexAttribute("position", 3, 6*sizeof(float), 0);
	normalLocation = Application::instance().getShader()->bindVertexAttribute("normal", 3, 6*sizeof(float), (void *)(3*sizeof(float)));
}

void TriangleMesh::render() const
{
	Application::instance().getShader()->use();

	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	glEnableVertexAttribArray(normalLocation);
	glDrawArrays(GL_TRIANGLES, 0, triangles.size());
}

void TriangleMesh::free()
{
	if(vbo != -1)
		glDeleteBuffers(1, &vbo);
	if(vao != -1)
		glDeleteVertexArrays(1, &vao);
	
	vertices.clear();
	triangles.clear();
}



glm::vec3 TriangleMesh::computeNormalTriangle(int triangleInd) const {
	glm::vec3 normal;
	normal = glm::cross(vertices[triangles[triangleInd+1]] - vertices[triangles[triangleInd]], 
						vertices[triangles[triangleInd+2]] - vertices[triangles[triangleInd]]);
	normal = glm::normalize(normal);
	return normal;
}


void TriangleMesh::computeRepresentative(int method, std::unordered_map<glm::ivec3, CellInfo>& grid, int gridSize, const glm::vec3& min) const {
	if (method == AVG_METHOD) {
		for (int ind = 0; ind < vertices.size(); ++ind) {
			const glm::vec3 &v = vertices[ind];
			
			glm::ivec3 gridPos = getGridPos(v, gridSize, min);

			if (grid.count(gridPos) > 0) {
				CellInfo& ci = grid.at(gridPos);
				ci.vertexSum += v;
				ci.vertexCount++;
			}
			else {
				CellInfo ci{v, 1};
				grid.insert({gridPos, ci});
			}
		}
	}
	else {
		for (int tri = 0; tri < triangles.size(); tri+=3) {
			for(unsigned int vrtx=0; vrtx<3; vrtx++) {
				const glm::vec3 &v = vertices[triangles[tri+vrtx]];
				glm::vec3 normal = computeNormalTriangle(tri);

				glm::ivec3 gridPos = getGridPos(v, gridSize, min);
				glm::vec3 nodeCenter = static_cast<glm::vec3>(gridPos) * float(gridSize) + gridSize/2.f;

				float d = - glm::dot(normal, v - nodeCenter);

				if (grid.count(gridPos) > 0) {
					CellInfo& ci = grid.at(gridPos);

				}
				else { // Init Q 4x4 matrix
					
				}

			}
		}
		
		// Eigen::MatrixXf m = Eigen::MatrixXf();
		// cout << "Here is the matrix m:" << endl << m << endl;
		// Eigen::JacobiSVD<Eigen::MatrixXf, Eigen::ComputeThinU | Eigen::ComputeThinV> svd(m);
		// cout << "Its singular values are:" << endl << svd.singularValues() << endl;
		// cout << "Its left singular vectors are the columns of the thin U matrix:" << endl << svd.matrixU() << endl;
		// cout << "Its right singular vectors are the columns of the thin V matrix:" << endl << svd.matrixV() << endl;
		// Eigen::Vector3f rhs(1, 0, 0);
		// cout << "Now consider this rhs vector:" << endl << rhs << endl;
		// cout << "A least-squares solution of m*x = rhs is:" << endl << svd.solve(rhs) << endl;


	}
}

TriangleMesh* TriangleMesh::getSimplifiedMesh(int gridRes) const
{
	pair<glm::vec3, glm::vec3> bb = getBoundingBox();
	glm::vec3 min = bb.first;
	glm::vec3 max = bb.second;

	float gridSize = (max.x - min.x) / gridRes;


	std::unordered_map<glm::ivec3, CellInfo> grid;

	computeRepresentative(AVG_METHOD, grid, gridSize, min);

	TriangleMesh* m = new TriangleMesh();

	int vertexCount = 0;
	for (auto& it: grid) {
		CellInfo& ci = it.second;
		glm::vec3 vertex(ci.vertexSum / float(ci.vertexCount));
		ci.outputVertexId = vertexCount++;
		m->addVertex(vertex);
	}

	for (int tri = 0; tri < triangles.size(); tri+=3) {
		std::vector<int> indx(3);
		for(unsigned int vrtx=0; vrtx<3; vrtx++)
		{
			glm::ivec3 gridPos = getGridPos(vertices[triangles[tri + vrtx]], gridSize, min);
			CellInfo ci = grid.at(gridPos);
			indx[vrtx] = ci.outputVertexId; 
		}

		// Check if all 3 vertices are different
		bool equal = indx[0] == indx[1] || indx[1] == indx[2] || indx[2] == indx[0];
		if (!equal) {
			m->addTriangle(indx[0], indx[1], indx[2]);
		}
	}

	cout << "Simplified from " << this->triangles.size() << " to " << m->getTriangles().size() << endl;

	return m;
}

pair<glm::vec3, glm::vec3> TriangleMesh::getBoundingBox() const
{
	glm::vec3 min, max;
	min = max = vertices[0];

	for (int i = 1; i < vertices.size(); ++i) {
		if (min.x > vertices[i].x)
			min.x = vertices[i].x;
		if (min.y > vertices[i].y)
			min.y = vertices[i].y;
		if (min.z > vertices[i].z)
			min.z = vertices[i].z;

		if (max.x < vertices[i].x)
			max.x = vertices[i].x;
		if (max.y < vertices[i].y)
			max.y = vertices[i].y;
		if (max.z < vertices[i].z)
			max.z = vertices[i].z;
	}

	return {min, max};
}

glm::ivec3 TriangleMesh::getGridPos(const glm::vec3& pos, float gridSize, const glm::vec3& min) const
{
	int i = (pos.x - min.x) / gridSize;
	int j = (pos.y - min.y) / gridSize;
	int k = (pos.z - min.z) / gridSize;
    return glm::ivec3(i,j,k);
}




