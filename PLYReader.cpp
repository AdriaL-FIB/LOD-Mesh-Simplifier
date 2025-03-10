#include <iostream>
#include <cstring>
#include <vector>
#include "PLYReader.h"


// Reads the mesh from the PLY file, first the header, then the vertex data, 
// and finally the face data. Then it rescales the model so that it fits a
// box of size 1x1x1 centered at the origin

bool PLYReader::readMesh(const string &filename, TriangleMesh &mesh)
{
	ifstream fin;
	int nVertices, nFaces;

	fin.open(filename.c_str(), ios_base::in | ios_base::binary);
	if(!fin.is_open())
		return false;
	if(!loadHeader(fin, nVertices, nFaces))
	{
		fin.close();
		return false;
	}

	vector<float> plyVertices;
	vector<int> plyTriangles;

	loadVertices(fin, nVertices, plyVertices);
	loadFaces(fin, nFaces, plyTriangles);
	fin.close();

	rescaleModel(plyVertices);
	addModelToMesh(plyVertices, plyTriangles, mesh);

	return true;
}

// Reads the header of a PLY file.
// It first checks that the file is really a PLY. 
// Then it reads lines until it finds the 'end_header'
// The 'element vertex' and 'element face' lines contain the number 
// of primitives in the file.

bool PLYReader::loadHeader(ifstream &fin, int &nVertices, int &nFaces)
{
	char line[100];

	fin.getline(line, 100);
	if(strncmp(line, "ply", 3) != 0)
		return false;
	nVertices = 0;
	fin.getline(line, 100);
	while(strncmp(line, "end_header", 10) != 0)
	{
		if(strncmp(line, "element vertex", 14) == 0)
			nVertices = atoi(&line[15]);
		fin.getline(line, 100);
		if(strncmp(line, "element face", 12) == 0)
			nFaces = atoi(&line[13]);
	}
	if(nVertices <= 0)
		return false;
	cout << "Loading triangle mesh" << endl;
	cout << "\tVertices = " << nVertices << endl;
	cout << "\tFaces = " << nFaces << endl;
	cout << endl;

	return true;
}

// Loads the vertices' coordinates into a vector

void PLYReader::loadVertices(ifstream &fin, int nVertices, vector<float> &plyVertices)
{
	int i;
	float value;

	plyVertices.resize(3*nVertices);
	for(i=0; i<nVertices; i++)
	{
		fin.read((char *)&value, sizeof(float));
		plyVertices[3*i] = value;
		fin.read((char *)&value, sizeof(float));
		plyVertices[3*i+1] = value;
		fin.read((char *)&value, sizeof(float));
		plyVertices[3*i+2] = value;
	}
}

// Same thing for the faces. Those with more than three sides
// are subdivided into triangles.

void PLYReader::loadFaces(ifstream &fin, int nFaces, vector<int> &plyTriangles)
{
	int i, tri[3];
	unsigned char nVrtxPerFace;

	plyTriangles.reserve(nFaces);
	for(i=0; i<nFaces; i++)
	{
		fin.read((char *)&nVrtxPerFace, sizeof(unsigned char));
		fin.read((char *)&tri[0], sizeof(int));
		fin.read((char *)&tri[1], sizeof(int));
		fin.read((char *)&tri[2], sizeof(int));
		plyTriangles.push_back(tri[0]);
		plyTriangles.push_back(tri[1]);
		plyTriangles.push_back(tri[2]);
		for(; nVrtxPerFace>3; nVrtxPerFace--)
		{
			tri[1] = tri[2];
			fin.read((char *)&tri[2], sizeof(int));
			plyTriangles.push_back(tri[0]);
			plyTriangles.push_back(tri[1]);
			plyTriangles.push_back(tri[2]);
		}
	}
}

// Rescales the model to fit a box of 1x1x1 with its base centered at the origin

void PLYReader::rescaleModel(vector<float> &plyVertices)
{
	unsigned int i;
	glm::vec3 baseCenter, size[2];

	size[0] = glm::vec3(1e10, 1e10, 1e10);
	size[1] = glm::vec3(-1e10, -1e10, -1e10);
	for(i=0; i<plyVertices.size(); i+=3)
	{
		size[0][0] = glm::min(size[0][0], plyVertices[i]);
		size[0][1] = glm::min(size[0][1], plyVertices[i+1]);
		size[0][2] = glm::min(size[0][2], plyVertices[i+2]);
		size[1][0] = glm::max(size[1][0], plyVertices[i]);
		size[1][1] = glm::max(size[1][1], plyVertices[i+1]);
		size[1][2] = glm::max(size[1][2], plyVertices[i+2]);
	}
	baseCenter = glm::vec3((size[0][0] + size[1][0]) / 2.f, size[0][1], (size[0][2] + size[1][2]) / 2.f);

	float largestSize = glm::max(size[1][0] - size[0][0], glm::max(size[1][1] - size[0][1], size[1][2] - size[0][2]));

	for(i=0; i<plyVertices.size(); i+=3)
	{
		plyVertices[i] = (plyVertices[i] - baseCenter[0]) / largestSize;
		plyVertices[i+1] = (plyVertices[i+1] - baseCenter[1]) / largestSize;
		plyVertices[i+2] = (plyVertices[i+2] - baseCenter[2]) / largestSize;
	}
}

// Vertex and face data are added to the model using this function

void PLYReader::addModelToMesh(const vector<float> &plyVertices, const vector<int> &plyTriangles, TriangleMesh &mesh)
{
	mesh.initVertices(plyVertices);
	mesh.initTriangles(plyTriangles);
}


