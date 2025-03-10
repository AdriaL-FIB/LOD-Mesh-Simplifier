#ifndef _TRIANGLE_MESH_INSTANCE_INCLUDE
#define _TRIANGLE_MESH_INSTANCE_INCLUDE


#include <glm/glm.hpp>
#include "TriangleMesh.h"


class TriangleMeshInstance
{

public:
	TriangleMeshInstance();
	~TriangleMeshInstance();
	
	void init(TriangleMesh *mesh, const glm::vec4 &color = glm::vec4(1.0f), const glm::mat4 &transform = glm::mat4(1.0f));
	void render();
	
	TriangleMesh *getMesh();
	
	void setTransform(const glm::mat4 &transform);
	const glm::mat4 &getTransform() const;
	void addTransform(const glm::mat4 &addedTransform);
	
	void resetTransform();
	void translate(const glm::vec3 &move);
	void rotate(const glm::vec3 &axis, float angleDegrees);
	void scale(const glm::vec3 &factor);
	
	void setColor(const glm::vec4 &color);
	const glm::vec4 &getColor() const;
	
private:
	TriangleMesh *mesh;
	glm::vec4 color;
	glm::mat4 transform;

};


#endif // _TRIANGLE_MESH_INSTANCE_INCLUDE


