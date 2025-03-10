#include "Application.h"
#include "TriangleMeshInstance.h"
#include <glm/gtc/matrix_transform.hpp>


TriangleMeshInstance::TriangleMeshInstance()
{
	mesh = NULL;
}

TriangleMeshInstance::~TriangleMeshInstance()
{
}


void TriangleMeshInstance::init(TriangleMesh *mesh, const glm::vec4 &color, const glm::mat4 &transform)
{
	this->mesh = mesh;
	this->color = color;
	this->transform = transform;
}

void TriangleMeshInstance::render()
{
	if(mesh != NULL)
	{
 		Application::instance().getShader()->setUniform4f("color", color.r, color.g, color.b, color.a);
		Application::instance().getShader()->setUniformMatrix4f("model", transform);
		mesh->render();
	}
}

TriangleMesh *TriangleMeshInstance::getMesh()
{
	return mesh;
}

void TriangleMeshInstance::setTransform(const glm::mat4 &transform)
{
	this->transform = transform;
}

const glm::mat4 &TriangleMeshInstance::getTransform() const
{
	return transform;
}

void TriangleMeshInstance::addTransform(const glm::mat4 &addedTransform)
{
	transform = addedTransform * transform;
}

void TriangleMeshInstance::resetTransform()
{
	transform = glm::mat4(1.0f);
}

void TriangleMeshInstance::translate(const glm::vec3 &move)
{
	transform = glm::translate(glm::mat4(1.0f), move) * transform;
}

void TriangleMeshInstance::rotate(const glm::vec3 &axis, float angleDegrees)
{
	transform = glm::rotate(glm::mat4(1.0f), angleDegrees * 3.14159f / 180.f, axis) * transform;
}

void TriangleMeshInstance::scale(const glm::vec3 &factor)
{
	transform = glm::scale(glm::mat4(1.0f), factor) * transform;
}

void TriangleMeshInstance::setColor(const glm::vec4 &color)
{
	this->color = color;
}

const glm::vec4 &TriangleMeshInstance::getColor() const
{
	return color;
}



