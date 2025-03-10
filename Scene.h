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
	bool loadMesh(const string &filename);
	void update(int deltaTime);
	void render();

	VectorCamera &getCamera();

private:
	void computeModelViewMatrix();
	
	void initRoom(const std::vector<std::vector<char>>& mapInfo);
	void renderRoom();

private:
	VectorCamera camera;
	TriangleMesh *cube, *mesh;
	TriangleMeshInstance *object;
	vector<TriangleMeshInstance *> walls;
	float currentTime;

};


#endif // _SCENE_INCLUDE

