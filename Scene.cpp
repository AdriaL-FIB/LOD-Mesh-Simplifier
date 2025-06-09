#include <iostream>
#include <cmath>
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include "Scene.h"
#include "PLYReader.h"
#include "Application.h"
#include "Visibility.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/color_space.hpp>

#include <filesystem>

#define NUM_METHODS 4

#define MAX_COST (400000)

#define N_FRAMES 120


Scene::Scene()
{
	cube = nullptr;
	meshSimplificationMethod = 0;
	mapSize = glm::vec2(0.f, 0.f);
}

Scene::~Scene()
{
	if(cube != nullptr)
		delete cube;
	//for (std::vector<std::vector<TriangleMesh*>> meshMethodsLods : meshes)
	//	for (std::vector<TriangleMesh*> meshLods : meshMethodsLods)
	//		for (TriangleMesh* mesh : meshLods)
	//			if (mesh != nullptr)
	//				delete mesh;
	for (TriangleMeshInstance *object : objects)
		if (object != nullptr)
			delete object;
	for(vector<TriangleMeshInstance *>::iterator it=walls.begin(); it!=walls.end(); it++)
		if (*it != nullptr)
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

	int num_models;
	fin >> num_models;

	// read empty line
	std::getline(fin, line);

	std::filesystem::create_directories("../models/LODs");

	for (int i = 0; i < num_models; ++i) {
		std::getline(fin, line);
		std::stringstream ss(line);
		ss >> model_filename;
		int gridRes;
		std::vector<int> lodRes;
		while (ss >> gridRes) {
			lodRes.push_back(gridRes);
		}
		loadMesh(model_filename, lodRes, {0});
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
	lodTimers.resize(num_instances, 0);
	objectVisibility.resize(num_instances, true);


	
	fin.close();
	return true;
}

// Loads the mesh into CPU memory and sends it to GPU memory (using GL)

bool Scene::loadMesh(const string &filename, const std::vector<int>& lodRes, const std::vector<int>& methods)
{
#pragma warning( push )
#pragma warning( disable : 4101)
	PLYReader reader;
#pragma warning( pop ) 


	std::cout << "load mesh f" << std::endl;
	std::shared_ptr<TriangleMesh> mesh = std::make_shared<TriangleMesh>();
	bool bSuccess = reader.readMesh(filename, *mesh);
	if(bSuccess)
	{
		mesh->sendToOpenGL();

		std::vector<std::vector<std::shared_ptr<TriangleMesh>>> meshMethodsLods(NUM_METHODS);

		for (int method : methods) {
			assert((method < NUM_METHODS), "Number of subdivision method geater than the total number of methods");
			std::vector<std::shared_ptr<TriangleMesh>>& meshLods = meshMethodsLods.at(method);
			meshLods.resize(lodRes.size() + 1);
			meshLods[0] = mesh;
			for (int i = 0; i < lodRes.size(); ++i) { // modelname_res_method.ply
				std::shared_ptr<TriangleMesh> simplifiedMesh(mesh->getSimplifiedMesh(lodRes[i], method));
				simplifiedMesh->sendToOpenGL();
				meshLods[i+1] = simplifiedMesh;
			}
		}


		// Push all LODs of the mesh
		meshes.push_back(meshMethodsLods);
	}
	
	return bSuccess;
}

void Scene::addInstance(int mesh, const glm::mat4 &transform)
{
	TriangleMeshInstance* object = new TriangleMeshInstance();
	object->init(&meshes[mesh][meshSimplificationMethod], glm::vec4(1.0f), transform);
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
	computeObjectVisibility();
	computeLODLevels();
}

// Render the scene. First the room, then the mesh it there is one loaded.

void Scene::render()
{
	camera.render();
 	renderRoom();

	for (int i = 0; i < objects.size(); ++i) {
		TriangleMeshInstance* object = objects[i];

		if (objectVisibility[i]) {
			if (lodColoring) {
				int lodCount = object->lodCount();
				glm::vec3 hsvColor(1.f);
				hsvColor.x = object->getLod() * 360.f / float(lodCount);
				object->render(glm::vec4(glm::rgbColor(hsvColor), 1.f));
			}
			else
				object->render();
		}

	}
}

VectorCamera &Scene::getCamera()
{
	return camera;
}

void Scene::toggleLODColoring()
{
	lodColoring = !lodColoring;
}

void Scene::setMeshSimplficationMethod(int method)
{
	meshSimplificationMethod = method;
	for (TriangleMeshInstance* object : objects) {
		int meshIndex = object->getMeshIndex();
		object->setMesh(&meshes[meshIndex][method]);
	}
}


float Scene::computeBenefit(TriangleMeshInstance* object, int lod)
{
	TriangleMesh* mesh = object->getMesh(lod);
	pair<glm::vec3, glm::vec3> bb = mesh->getBoundingBox();
	glm::vec3 min = bb.first;
	glm::vec3 max = bb.second;
	float diagonal = glm::distance(min, max);

	glm::vec3 position = object->getTransform()[3];
	glm::vec3 center = (min + max) / 2.f + position;

	glm::vec3 viewPoint = camera.getPosition();
	float distance = glm::distance(center, viewPoint);

	return benefit(diagonal, distance, object->lodCount() - 1 - lod);
}

void Scene::computeLODLevels()
{
	int totalCost = 0;
	float maxCost = MAX_COST;

	int N = objects.size();

	std::vector<int> costs(N);
	std::vector<int> lods(N);
	std::vector<float> benefits(N);

	std::vector<int> lastLods(N);

	for (int i = 0; i < N; ++i) {
		if (!objectVisibility[i]) {
			lodTimers[i] = 0;
			continue;
		}

		lastLods[i] = objects[i]->getLod();

		lodTimers[i] = (lodTimers[i] > 0) ? lodTimers[i] - 1 : 0;
		int startingLod;
		if (lodTimers[i] == 0) {
			startingLod = objects[i]->lodCount() - 1;
		}
		else {
			startingLod = lastLods[i];
		}

		TriangleMeshInstance* object = objects[i];
		TriangleMesh* mesh = object->getMesh(startingLod);
		lods[i] = startingLod;
		costs[i] = mesh->getNumberOfTriangles();
		benefits[i] = computeBenefit(object, startingLod);
		totalCost += costs[i];
		objects[i]->setLod(startingLod);
	}

	if (totalCost < maxCost) {
		while (true) {
			// Find object with largest delta benefit / delta cost
			int best = -1; // LOD to increase
			float bestRatio = 0;
			float benefitOfBest = 0;
			for (int i = 0; i < N; ++i) {
				if (!objectVisibility[i] || lodTimers[i] > 0) continue;
				int currentLod = lods[i];
				int nextLod = currentLod - 1;
				if (nextLod < 0) continue;

				int currentCost = costs[i];
				float currentBenefit = benefits[i];

				TriangleMeshInstance* object = objects[i];
				TriangleMesh* mesh = object->getMesh(nextLod);

				int nextCost = mesh->getNumberOfTriangles();
				int deltaCost = (nextCost - currentCost);

				if (totalCost + deltaCost >= maxCost) continue;

				float nextBenefit = computeBenefit(object, nextLod);

				float ratio = (nextBenefit - currentBenefit) / deltaCost;
				if (ratio > bestRatio) {
					bestRatio = ratio;
					best = i;
					benefitOfBest = nextBenefit;
				}
			}

			if (best < 0) break;

			// Increase level of detail for that object (decrease variable)
			lods[best]--;
			objects[best]->setLod(lods[best]);

			// Update total cost
			int newCost = objects[best]->getMesh()->getNumberOfTriangles(); // cost of the new LOD
			totalCost += newCost - costs[best]; // increment of cost
			benefits[best] = benefitOfBest; // update benefit of object with new LOD
			costs[best] = newCost; // update cost of of object with new LOD
		}
	}



	cout << "LODS:";
	for (int i = 0; i < N; ++i) {
		if (!objectVisibility[i]) cout << " -";
		else cout << " " << lods[i];

		if (lods[i] != lastLods[i]) { // If LOD changed, reset timer
			lodTimers[i] = N_FRAMES;
		}
	}
	cout << " totalCost:" << totalCost << endl;
}

void Scene::computeObjectVisibility()
{
	glm::ivec2 cameraGridCell(floor(getCamera().getPosition().x + mapSize.x / 2.f), floor(getCamera().getPosition().z + mapSize.y / 2.f));
	for (int i = 0; i < objects.size(); ++i) {
		TriangleMeshInstance* object = objects[i];

		std::pair<glm::vec3, glm::vec3> bb = object->getBoundingBox();
		glm::vec3 minPoint = bb.first;
		glm::vec3 maxPoint = bb.second;

		int minGridX = floor(minPoint.x + mapSize.x / 2.f);
		int minGridZ = floor(minPoint.z + mapSize.y / 2.f);
		int maxGridX = floor(maxPoint.x + mapSize.x / 2.f);
		int maxGridZ = floor(maxPoint.z + mapSize.y / 2.f);

		int cameraCellIndex = cameraGridCell.y * mapSize.x + cameraGridCell.x;
		if (cameraCellIndex >= cellsVisibility.size()) {
			objectVisibility[i] = true;
			continue;
		}

		objectVisibility[i] = false;
		std::set<int>& visibleCells = cellsVisibility[cameraCellIndex];
		for (int gridX = (std::max)(minGridX, 0); gridX <= (std::min)(maxGridX, mapSize.x) && !objectVisibility[i]; ++gridX)
			for (int gridZ = (std::max)(minGridZ, 0); gridZ <= (std::min)(maxGridZ, mapSize.y) && !objectVisibility[i]; ++gridZ) {
				int cellIndex = gridZ * mapSize.x + gridX;
				if (visibleCells.count(cellIndex) > 0 || cameraCellIndex == cellIndex) {
					objectVisibility[i] = true;
				}
			}
	}
}

// Init & render the room. Both the floor and the walls are instances of the
// same initial cube scaled and translated to build the room.
void Scene::initRoom(const std::vector<std::vector<char>>& map)
{
	std::cout << "init room f" << std::endl;
	glm::mat4 transform;
	TriangleMeshInstance *instance;

	int nz = map.size(); // rows
	int nx = map[0].size(); // columns

	mapSize = glm::ivec2(nx, nz);

	for (int i = 0; i < nz; ++i) {
		for (int j = 0; j < nx; ++j) {

			transform = glm::mat4(1.0f);

			switch (map[i][j])
			{
			case 0: // Floor
				transform = glm::translate(transform, glm::vec3(j - nx / 2, -0.5f, i - nz / 2));
				transform = glm::scale(transform, glm::vec3(1.f, 1.f, 1.f));
				break;
			
			case 1: // Wall
				transform = glm::translate(transform, glm::vec3(j - nx / 2, 1.f, i - nz / 2));
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

	// Visibility
	const std::string visibilityPath = "../visibility.txt";

	bool recomputeVis = true;

	if (std::filesystem::exists(visibilityPath)) {
		ifstream fin;

		fin.open(visibilityPath);
		if (!fin.is_open())
			return;

		int vnx, vny;
		fin >> vnx >> vny;


		std::string line;
		// read empty line
		std::getline(fin, line);


		if (vnx == nx && vny == nz) {
			std::cout << "Reading visibility file..." << std::endl;

			recomputeVis = false;
			cellsVisibility.resize(nx * nz);
			// read file
			int cell_i, cell_j; // Cell i sees cell j
			while (std::getline(fin, line)) {
				std::stringstream ss(line);
				ss >> cell_i;
				while (ss >> cell_j) {
					cellsVisibility[cell_i].insert(cell_j);
				}
			}
			std::cout << "Done." << std::endl;
		}
	}

	if (recomputeVis) {
		cellsVisibility = Visibility::computeVisibility(map, 1000000);
		Visibility::saveVisibilityFile(cellsVisibility, visibilityPath, nx, nz);
	}
}

void Scene::renderRoom()
{
	for(vector<TriangleMeshInstance*>::iterator it = walls.begin(); it != walls.end(); it++)
		(*it)->render();
}