#include <iostream>
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Scene.h"
#include "PLYReader.h"
#include "Application.h"


Scene::Scene()
{
	cube = NULL;
	// mesh = NULL;
}

Scene::~Scene()
{
	if(cube != NULL)
		delete cube;
	for (TriangleMesh *mesh : meshes)
		delete mesh;
	for (TriangleMeshInstance *object : objects)
		delete object;
	for(vector<TriangleMeshInstance *>::iterator it=walls.begin(); it!=walls.end(); it++)
		delete *it;
}


// Initialize the scene. This includes the cube we will use to render
// the floor and walls, as well as the camera.

void Scene::init()
{
	cube = new TriangleMesh();
	cube->buildCube();
	cube->sendToOpenGL();
	currentTime = 0.0f;
	//initRoom();
	
	camera.init(glm::vec3(0.f, 0.75f, 2.f));
}

// Load the map & all its associated models

bool Scene::loadMap(const string &filename)
{
	std::cout << "load map f" << std::endl;
	ifstream fin;
	string model_filename;
	
	fin.open(filename);
	if(!fin.is_open())
		return false;

	int nx, ny;


	std::vector<std::vector<char>> mapInfo;

	fin >> nx >> ny;
	std::string line; 

	// read empty line
	std::getline(fin, line);

	for (int i = 0; i < ny; ++i) {
		std::getline(fin, line);
		// cout << "read line: " << line << " " << line.size() << endl;
		std::vector<char> l;
		for (char c : line) {
			l.push_back(c - '0');
		}
		mapInfo.push_back(l);
	}
	initRoom(mapInfo);

	// for (auto a : mapInfo) {
	// 	for (char c : a) {
	// 		cout << c;
	// 	}
	// 	cout << endl;
	// }


	int num_models;
	fin >> num_models;
	for (int i = 0; i < num_models; ++i) {
		fin >> model_filename;
		loadMesh(model_filename);
	}
	std::cout << "Loaded " << num_models << " models." << std::endl;

	int num_instances;
	fin >> num_instances;
	for (int i = 0; i < num_instances; ++i) {
		int meshIndx;
		glm::vec3 t, s;
		fin >> meshIndx;
		fin >> t.x >> t.y >> t.z;
		fin >> s.x >> s.y >> s.z;

		glm::mat4 transform(1.0f);
		transform = glm::translate(transform, t);
		transform = glm::scale(transform, s);

		addInstance(meshIndx, transform);
	}
	std::cout << "Loaded " << num_instances << " instances." << std::endl;

	
	fin.close();
	return true;
}

// Loads the mesh into CPU memory and sends it to GPU memory (using GL)

bool Scene::loadMesh(const string &filename)
{
#pragma warning( push )
#pragma warning( disable : 4101)
	PLYReader reader;
#pragma warning( pop ) 


	std::cout << "load mesh f" << std::endl;
	// if(mesh != NULL)
	// {
	// 	mesh->free();
	// 	delete mesh;
	// }
	TriangleMesh *mesh = new TriangleMesh();
	bool bSuccess = reader.readMesh(filename, *mesh);
	if(bSuccess)
	{
		TriangleMesh* simplifiedMesh = mesh->getSimplifiedMesh(16);
		simplifiedMesh->sendToOpenGL();

		// TriangleMeshInstance* object = new TriangleMeshInstance();
		// object->init(mesh, glm::vec4(1.0f), glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
		meshes.push_back(simplifiedMesh);
	}
	delete mesh;
	
	return bSuccess;
}

void Scene::addInstance(int mesh, const glm::mat4 &transform)
{
	TriangleMeshInstance* object = new TriangleMeshInstance();
	object->init(meshes[mesh], glm::vec4(1.0f), transform);
	objects.push_back(object);
}

void Scene::addInstances(int mesh, const std::vector<glm::mat4> &transform)
{
	for (int i = 0; i < transform.size(); ++i) {
		addInstance(mesh, transform[i]);
	}
}

void Scene::update(int deltaTime)
{
	currentTime += deltaTime;
}

// Render the scene. First the room, then the mesh it there is one loaded.

void Scene::render()
{
	camera.render();
 	renderRoom();
	for (TriangleMeshInstance *object : objects)
 		object->render();
}

VectorCamera &Scene::getCamera()
{
	return camera;
}

// Init & render the room. Both the floor and the walls are instances of the
// same initial cube scaled and translated to build the room.

void Scene::initRoom(const std::vector<std::vector<char>>& map)
{
	std::cout << "init room f" << std::endl;
	glm::mat4 transform;
	TriangleMeshInstance *instance;

	int ny = map.size();
	int nx = map[0].size();

	for (int i = 0; i < ny; ++i) {
		for (int j = 0; j < nx; ++j) {

			transform = glm::mat4(1.0f);

			switch (map[i][j])
			{
			case 0: // Floor
				transform = glm::translate(transform, glm::vec3(i - ny/2, -0.5f, j - nx/2));
				transform = glm::scale(transform, glm::vec3(1.f, 1.f, 1.f));
				break;
			
			case 1: // Wall
				transform = glm::translate(transform, glm::vec3(i - ny/2, 1.f, j - nx/2));
				transform = glm::scale(transform, glm::vec3(1.f, 2.f, 1.f));
				break;
			default:
				std::cout << "Error" << std::endl;
			}

			instance = new TriangleMeshInstance();
			instance->init(cube, glm::vec4(1.0f), transform);
			walls.push_back(instance);

			// cout << "loaded with x:" << i - ny/2 << " z: " << j - nx/2 << endl;
		}
	}
}

void Scene::renderRoom()
{
	for(vector<TriangleMeshInstance*>::iterator it = walls.begin(); it != walls.end(); it++)
		(*it)->render();
}