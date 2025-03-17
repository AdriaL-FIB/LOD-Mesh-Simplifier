#ifndef _TRIANGLE_MESH_INCLUDE
#define _TRIANGLE_MESH_INCLUDE


#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"

#include <unordered_map>


using namespace std;


// Class TriangleMesh contains the geometry of a mesh built out of triangles.
// Both the vertices and the triangles are stored in vectors.
// TriangleMesh also manages the ids of the copy in the GPU, so as to 
// be able to render it using OpenGL.


class TriangleMesh
{
private:
	struct CellInfo
	{
		glm::vec3 vertexSum;
		int vertexCount;
		int outputTriangleId;
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
	
	void sendToOpenGL();
	void render() const;
	void free();

	TriangleMesh getSimplifiedMesh(int gridRes) const;

	pair<glm::vec3, glm::vec3> getBoundingBox() const;

private:
	vector<glm::vec3> vertices;
	vector<int> triangles;

	GLuint vao;
	GLuint vbo;
	GLint posLocation, normalLocation;

	glm::ivec3 getGridPos(const glm::vec3& pos, int gridSize, const glm::vec3& min) const;
};


#endif // _TRIANGLE_MESH_INCLUDE


