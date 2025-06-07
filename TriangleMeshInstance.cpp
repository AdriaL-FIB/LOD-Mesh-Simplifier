#include "Application.h"
#include "TriangleMeshInstance.h"
#include <glm/gtc/matrix_transform.hpp>


TriangleMeshInstance::TriangleMeshInstance()
{
	meshLODs = NULL;
	mesh = NULL;
	lod = 0;
	meshIndex = -1;
}

TriangleMeshInstance::~TriangleMeshInstance()
{
}


void TriangleMeshInstance::init(std::vector<std::shared_ptr<TriangleMesh>>* meshLODs, const glm::vec4 &color, const glm::mat4 &transform, int meshIndex)
{
	this->mesh = nullptr;
	this->meshLODs = meshLODs;
	this->color = color;
	this->transform = transform;
	this->meshIndex = meshIndex;
}

void TriangleMeshInstance::init(TriangleMesh* mesh, const glm::vec4& color, const glm::mat4& transform)
{
	this->meshLODs = nullptr;
	this->mesh = mesh;
	this->color = color;
	this->transform = transform;
}


void TriangleMeshInstance::render()
{
	render(this->color);
}

void TriangleMeshInstance::render(const glm::vec4& color)
{
	if (meshLODs != NULL) {
		Application::instance().getShader()->setUniform4f("color", color.r, color.g, color.b, color.a);
		Application::instance().getShader()->setUniformMatrix4f("model", transform);
		meshLODs->at(lod)->render();
	}
	else if (mesh != nullptr) {
		Application::instance().getShader()->setUniform4f("color", color.r, color.g, color.b, color.a);
		Application::instance().getShader()->setUniformMatrix4f("model", transform);
		mesh->render();
	}
}

TriangleMesh* TriangleMeshInstance::getMesh()
{
	return getMesh(this->lod);
}

TriangleMesh* TriangleMeshInstance::getMesh(int lod)
{
	if (meshLODs != nullptr)
		return meshLODs->at(lod).get();
	else
		return mesh;
}

void TriangleMeshInstance::setMesh(TriangleMesh* mesh)
{
	this->meshLODs = nullptr;
	this->mesh = mesh;
}

void TriangleMeshInstance::setMesh(std::vector<std::shared_ptr<TriangleMesh>>* meshLODs)
{
	this->mesh = nullptr;
	this->meshLODs = meshLODs;
}

void TriangleMeshInstance::setLod(int lod)
{
	this->lod = lod;
}

int TriangleMeshInstance::getLod()
{
	return this->lod;
}

int TriangleMeshInstance::lodCount() const
{
	if (meshLODs != nullptr)
		return meshLODs->size();
	else
		return 1;
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

int TriangleMeshInstance::getMeshIndex() const
{
	return meshIndex;
}



