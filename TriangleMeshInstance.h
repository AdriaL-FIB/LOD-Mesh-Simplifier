#ifndef _TRIANGLE_MESH_INSTANCE_INCLUDE
#define _TRIANGLE_MESH_INSTANCE_INCLUDE


#include <glm/glm.hpp>
#include "TriangleMesh.h"


class TriangleMeshInstance
{

public:
	TriangleMeshInstance();
	~TriangleMeshInstance();
	
	void init(std::vector<TriangleMesh*>* meshLODs, const glm::vec4& color = glm::vec4(1.0f), const glm::mat4& transform = glm::mat4(1.0f), int meshIndex = -1);
	void init(TriangleMesh* mesh, const glm::vec4 &color = glm::vec4(1.0f), const glm::mat4 &transform = glm::mat4(1.0f));
	void render();
	void render(const glm::vec4& color);
	
	TriangleMesh* getMesh();
	TriangleMesh* getMesh(int lod);
	void setMesh(TriangleMesh* mesh);
	void setMesh(std::vector<TriangleMesh*>* meshLODs);
	void setLod(int lod);
	int getLod();

	int lodCount() const;
	
	void setTransform(const glm::mat4 &transform);
	const glm::mat4 &getTransform() const;
	void addTransform(const glm::mat4 &addedTransform);
	
	void resetTransform();
	void translate(const glm::vec3 &move);
	void rotate(const glm::vec3 &axis, float angleDegrees);
	void scale(const glm::vec3 &factor);
	
	void setColor(const glm::vec4 &color);
	const glm::vec4 &getColor() const;
	int getMeshIndex() const;
	
private:
	TriangleMesh* mesh;
	std::vector<TriangleMesh*>* meshLODs;
	int lod;
	glm::vec4 color;
	glm::mat4 transform;

	int meshIndex;
};


#endif // _TRIANGLE_MESH_INSTANCE_INCLUDE


