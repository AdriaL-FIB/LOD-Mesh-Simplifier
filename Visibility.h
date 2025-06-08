#pragma once

#include <vector>
#include <set>
#include <glm/glm.hpp>
#include <string>

class Visibility
{
private:
	static std::vector<glm::ivec2> bresenhamSupercover(glm::vec2 o, glm::vec2 dir, int rows, int cols);
	static glm::vec2 randomUnitVector2D();
	static glm::vec2 randomVec2InBounds(float cols, float rows);
public:
	static std::vector<std::set<int>> computeVisibility(const std::vector<std::vector<char>>& mapInfo, int maxRays);
	static void saveVisibilityFile(const std::vector<std::set<int>>& vis, const std::string& outfile, int nx, int ny);
};

