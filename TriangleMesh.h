#ifndef _TRIANGLE_MESH_INCLUDE
#define _TRIANGLE_MESH_INCLUDE


#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"

#include <unordered_map>

#include <Eigen/SVD>


using namespace std;

constexpr int MAX_KEY_DIM = 4;
using GridKey = glm::vec<MAX_KEY_DIM, glm::i32, glm::defaultp>;

// Class TriangleMesh contains the geometry of a mesh built out of triangles.
// Both the vertices and the triangles are stored in vectors.
// TriangleMesh also manages the ids of the copy in the GPU, so as to 
// be able to render it using OpenGL.


class TriangleMesh
{
private:
	struct CellInfo
	{
		Eigen::Matrix4f Q;
		glm::vec3 vertexSum = glm::vec3(0);
		glm::vec3 outputVertex;
		int vertexCount = 0;
		int outputVertexId;
	};

public:
	TriangleMesh();
	~TriangleMesh();

	void addVertex(const glm::vec3 &position);
	void addTriangle(int v0, int v1, int v2);

	void initVertices(const vector<float> &newVertices);
	void initTriangles(const vector<int> &newTriangles);

	void buildCube();
	
	const vector<glm::vec3> &getVertices() const;
	vector<glm::vec3> &getVertices();
	const vector<int> &getTriangles() const;
	vector<int> &getTriangles();

	int getNumberOfTriangles() const;
	
	void sendToOpenGL();
	void render() const;
	void free();

	TriangleMesh* getSimplifiedMesh(int gridRes);

	pair<glm::vec3, glm::vec3> getBoundingBox() const;

private:
	vector<glm::vec3> vertices;
	vector<int> triangles;
	int nOfTriangles;

	vector<glm::vec3> normals;

	GLuint vao;
	GLuint vbo;
	GLint posLocation, normalLocation;

	glm::vec3 computeNormalTriangle(int triangleInd) const;
	void computeAverageNormals();

	glm::ivec3 getGridPos(const glm::vec3& pos, float gridSize, const glm::vec3& min) const;

	void computeRepresentative(int method, std::unordered_map<GridKey, CellInfo>& grid, float gridSize,const glm::vec3& min) const;
};


#endif // _TRIANGLE_MESH_INCLUDE


