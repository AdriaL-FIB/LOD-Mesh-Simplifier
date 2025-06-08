#pragma once

#include <vector>
#include <set>
#include <glm/glm.hpp>

class Visibility
{
private:
	static std::vector<glm::ivec2> bresenhamSupercover(glm::vec2 o, glm::vec2 dir, int rows, int cols);
	static glm::vec2 randomUnitVector2D();
	static glm::vec2 randomVec2InBounds(float cols, float rows);
	static std::vector<std::set<int>> computeVisibility(const std::vector<std::vector<char>>& mapInfo, int maxRays);
};

