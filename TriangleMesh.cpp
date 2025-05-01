#include <iostream>
#include <vector>
#include "TriangleMesh.h"
#include "Application.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

#define AVG_METHOD 0
#define QEM_METHOD 1
#define QEM_NORMALS_METHOD 2
#define AVG_NORMALS_METHOD 3

using namespace std;


static GridKey makeKey(const std::vector<int>& components) {
	GridKey key(0);
	int count = (std::min)(int(components.size()), MAX_KEY_DIM);
	for (int i = 0; i < count; ++i) {
		key[i] = components[i];
	}
	return key;
}



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


void TriangleMesh::computeRepresentative(int method, std::unordered_map<GridKey, CellInfo>& grid, float cellSize, const glm::vec3& min) const {
	if (method == AVG_METHOD) {
		for (int ind = 0; ind < vertices.size(); ++ind) {
			const glm::vec3 &v = vertices[ind];
			
			glm::ivec3 gridPos = getGridPos(v, cellSize, min);
			GridKey k = makeKey({ gridPos.x, gridPos.y, gridPos.z }); // new line

			if (grid.count(k) > 0) {
				CellInfo& ci = grid.at(k);
				ci.vertexSum += v;
				ci.vertexCount++;
			}
			else {
				CellInfo ci;
				ci.vertexSum = v;
				ci.vertexCount = 1;
				grid.insert({ k, ci });
			}
		}

		for (auto& it : grid) {
			CellInfo& ci = it.second;
			ci.outputVertex = (ci.vertexSum / float(ci.vertexCount));
		}
	}
	else if (method == AVG_NORMALS_METHOD) {
		for (int tri = 0; tri < triangles.size(); tri += 3) {
			glm::vec3 n = computeNormalTriangle(tri);
			for (unsigned int vrtx = 0; vrtx < 3; vrtx++) {
				const glm::vec3& v = vertices[triangles[tri + vrtx]];
				glm::ivec3 gridPos = getGridPos(v, cellSize, min);
				int normalId = (n.x < 0 ? 4 : 0) + (n.y < 0 ? 2 : 0) + (n.z < 0 ? 1 : 0);
				std::vector<int> components({ gridPos.x, gridPos.y, gridPos.z, normalId });
				GridKey k = makeKey(components);

				if (grid.count(k) > 0) {
					CellInfo& ci = grid.at(k);
					ci.vertexSum += v;
					ci.vertexCount++;
				}
				else {
					CellInfo ci;
					ci.vertexSum = v;
					ci.vertexCount = 1;
					grid.insert({ k, ci });
				}
			}
		}

		for (auto& it : grid) {
			CellInfo& ci = it.second;
			ci.outputVertex = (ci.vertexSum / float(ci.vertexCount));
		}
	}
	else {
		for (int tri = 0; tri < triangles.size(); tri+=3) {
			glm::vec3 n = computeNormalTriangle(tri);
			for(unsigned int vrtx=0; vrtx<3; vrtx++) {
				const glm::vec3 &v = vertices[triangles[tri+vrtx]];

				// create key for map
				glm::ivec3 gridPos = getGridPos(v, cellSize, min);
				std::vector<int> components({ gridPos.x, gridPos.y, gridPos.z });

				if (method == QEM_NORMALS_METHOD) { // Group by normals
					int normalId = (n.x < 0 ? 4 : 0) + (n.y < 0 ? 2 : 0) + (n.z < 0 ? 1 : 0);
					components.push_back(normalId);
				}
				GridKey k = makeKey(components); // new line


				glm::vec3 nodeCenter = min + static_cast<glm::vec3>(gridPos) * float(cellSize) + cellSize/2.f;

				float d = - glm::dot(n, v - nodeCenter);

				CellInfo* ci;

				if (grid.count(k) == 0) { // Init Q 4x4 matrix
					CellInfo ci;
					ci.Q.setZero();
					grid.insert({ k, ci });
				}
				ci = &grid.at(k);
				
				//Eigen::Vector4f plane(n.x, n.y, n.z, d);
				//ci->Q += plane * plane.transpose();

				ci->Q(0,0) += n.x * n.x;
				ci->Q(0,1) += n.x * n.y;
				ci->Q(0,2) += n.x * n.z;
				ci->Q(0,3) += n.x * d;

				ci->Q(1,0) += n.y * n.x;
				ci->Q(1,1) += n.y * n.y;
				ci->Q(1,2) += n.y * n.z;
				ci->Q(1,3) += n.y * d;

				ci->Q(2,0) += n.z * n.x;
				ci->Q(2,1) += n.z * n.y;
				ci->Q(2,2) += n.z * n.z;
				ci->Q(2,3) += n.z * d;


			}
		}

		// For every "active" cell in grid
		for (auto& it : grid) {
			CellInfo& ci = it.second;

			ci.Q(3, 3) = 1.f;
			//ci.Q.row(3) = Eigen::Vector4f(0.f, 0.f, 0.f, 1.f);

			Eigen::JacobiSVD<Eigen::Matrix4f, Eigen::ComputeThinU | Eigen::ComputeThinV> svd(ci.Q);

			Eigen::Vector4f sValues = svd.singularValues();

			float max = 0;
			float threshold = 1e-2f;
			for (int i = 0; i < 4; ++i) {
				if (sValues(i) > threshold) {
					sValues(i) = 1 / sValues(i); // INVERT VALUES ABOVE THRESHOLD
				}
				else {
					sValues(i) = 0;
				}
				max = (std::max)(max, sValues(i));
			}

			Eigen::Matrix4f sigmaPseudoInverse = sValues.asDiagonal();

			Eigen::Matrix4f QPseudoInverse = svd.matrixV() * sigmaPseudoInverse * svd.matrixU().transpose();

			Eigen::Vector4f b(0,0,0,1);
			Eigen::Vector4f p = QPseudoInverse * b;

			glm::vec3 nodeCenter = min + static_cast<glm::vec3>(it.first) * float(cellSize) + cellSize / 2.f;
			ci.outputVertex = glm::vec3(p.x(), p.y(), p.z()) + nodeCenter;
		}
	}
}

TriangleMesh* TriangleMesh::getSimplifiedMesh(int gridRes) const
{
	pair<glm::vec3, glm::vec3> bb = getBoundingBox();
	glm::vec3 min = bb.first;
	glm::vec3 max = bb.second;

	float cellSize = (max.x - min.x) / gridRes;


	std::unordered_map<GridKey, CellInfo> grid;

	constexpr int method = QEM_NORMALS_METHOD;
	computeRepresentative(method, grid, cellSize, min);

	TriangleMesh* m = new TriangleMesh();

	int vertexCount = 0;
	for (auto& it: grid) {
		CellInfo& ci = it.second;
		ci.outputVertexId = vertexCount++;
		m->addVertex(ci.outputVertex);
	}

	for (int tri = 0; tri < triangles.size(); tri+=3) {
		std::vector<int> indx(3);
		glm::vec3 n = computeNormalTriangle(tri);
		for(unsigned int vrtx=0; vrtx<3; vrtx++)
		{
			glm::ivec3 gridPos = getGridPos(vertices[triangles[tri + vrtx]], cellSize, min);
			std::vector<int> components({ gridPos.x, gridPos.y, gridPos.z });
			if (method == QEM_NORMALS_METHOD || method == AVG_NORMALS_METHOD) {
				int normalId = (n.x < 0 ? 4 : 0) + (n.y < 0 ? 2 : 0) + (n.z < 0 ? 1 : 0);
				components.push_back(normalId);
			}
			GridKey k = makeKey(components);
			
			CellInfo& ci = grid.at(k);
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

glm::ivec3 TriangleMesh::getGridPos(const glm::vec3& pos, float cellSize, const glm::vec3& min) const
{
	int i = (pos.x - min.x) / cellSize;
	int j = (pos.y - min.y) / cellSize;
	int k = (pos.z - min.z) / cellSize;
    return glm::ivec3(i,j,k);
}