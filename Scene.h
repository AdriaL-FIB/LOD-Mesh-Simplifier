#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include <vector>
#include "VectorCamera.h"
#include "TriangleMesh.h"
#include "TriangleMeshInstance.h"


using namespace std;


// Scene contains all the entities of our game.
// It is responsible for updating and render them.


class Scene
{

public:
	Scene();
	~Scene();

	void init();
	bool loadMap(const string &filename);
	bool loadMesh(const string &filename, const std::vector<int>& lodRes, const std::vector<int>& methods);
	void addInstance(int mesh, const glm::mat4& transform);
	void addInstances(int mesh, const std::vector<glm::mat4>& transform);
	void update(int deltaTime);
	void render();

	VectorCamera &getCamera();

	void toggleLODColoring();
	void setMeshSimplficationMethod(int method);

private:
	//void computeModelViewMatrix();
	inline float benefit(float diagonal, float distance, int lod) const { return 1 - diagonal / (std::powf(2, lod) * distance); }

	float computeBenefit(TriangleMeshInstance* mesh, int lod);
	void computeLODLevels();
	
	void initRoom(const std::vector<std::vector<char>>& mapInfo);
	void renderRoom();

private:
	VectorCamera camera;
	TriangleMesh *cube;
	// TriangleMeshInstance *object;
	vector<vector<vector<std::shared_ptr<TriangleMesh>>>> meshes; // Mesh --> SimplificationMethod --> LODs
	vector<TriangleMeshInstance *> objects;
	vector<TriangleMeshInstance *> walls;
	float currentTime;

	bool lodColoring;
	int meshSimplificationMethod;
};


#endif // _SCENE_INCLUDE

