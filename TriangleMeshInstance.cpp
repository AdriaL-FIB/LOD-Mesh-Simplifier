#include "Application.h"
#include "TriangleMeshInstance.h"
#include <glm/gtc/matrix_transform.hpp>


TriangleMeshInstance::TriangleMeshInstance()
{
	meshLODs = NULL;
	mesh = NULL;
	lod = 0;
	meshIndex = -1;
	bbHasToBeComputed = true;
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

TriangleMesh* TriangleMeshInstance::getMesh() const
{
	return getMesh(this->lod);
}

TriangleMesh* TriangleMeshInstance::getMesh(int lod) const
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

int TriangleMeshInstance::getLod() const
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
	bbHasToBeComputed = true;
}

const glm::mat4 &TriangleMeshInstance::getTransform() const
{
	return transform;
}

void TriangleMeshInstance::addTransform(const glm::mat4 &addedTransform)
{
	transform = addedTransform * transform;
	bbHasToBeComputed = true;
}

void TriangleMeshInstance::resetTransform()
{
	transform = glm::mat4(1.0f);
	bbHasToBeComputed = true;
}

void TriangleMeshInstance::translate(const glm::vec3 &move)
{
	transform = glm::translate(glm::mat4(1.0f), move) * transform;
	bbHasToBeComputed = true;
}

void TriangleMeshInstance::rotate(const glm::vec3 &axis, float angleDegrees)
{
	transform = glm::rotate(glm::mat4(1.0f), angleDegrees * 3.14159f / 180.f, axis) * transform;
	bbHasToBeComputed = true;
}

void TriangleMeshInstance::scale(const glm::vec3 &factor)
{
	transform = glm::scale(glm::mat4(1.0f), factor) * transform;
	bbHasToBeComputed = true;
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

pair<glm::vec3, glm::vec3> TriangleMeshInstance::getBoundingBox() const
{
	if (bbHasToBeComputed) {
		pair<glm::vec3, glm::vec3> bb = getMesh()->getBoundingBox();

		std::vector<glm::vec3> corners = getBoundingBoxCorners(bb.first, bb.second);

		glm::vec3 min, max;
		min = max = glm::vec3(transform * glm::vec4(corners[0], 1.f));

		for (int i = 1; i < corners.size(); ++i) {
			glm::vec3 p = glm::vec3(transform * glm::vec4(corners[i], 1.f));
			min = (glm::min)(min, p);
			max = (glm::max)(max, p);
		}

		this->bb = { min, max };
		bbHasToBeComputed = false;
	}
	
	return this->bb;
}

std::vector<glm::vec3> TriangleMeshInstance::getBoundingBoxCorners(const glm::vec3& min, const glm::vec3& max) const
{
	return {
		{min.x, min.y, min.z},
		{min.x, min.y, max.z},
		{min.x, max.y, min.z},
		{min.x, max.y, max.z},
		{max.x, min.y, min.z},
		{max.x, min.y, max.z},
		{max.x, max.y, min.z},
		{max.x, max.y, max.z}
	};
}
