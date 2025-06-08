#include "Visibility.h"

#include <glm/gtc/constants.hpp>
#include <random>
#include <cmath>


std::vector<glm::ivec2> Visibility::bresenhamSupercover(glm::vec2 o, glm::vec2 dir, int rows, int cols)
{
	float distBetweenColumns = (dir.x == 0) ? INFINITY : std::abs(1 / dir.x);
	float distBetweenRows = (dir.y == 0) ? INFINITY : std::abs(1 / dir.y);

	glm::ivec2 gridCell(int(o.x), int(o.y));

	float distToCol;
	float distToRow;
	int signx, signy;
	// init dist to
	if (dir.x < 0) {
		distToCol = o.x - gridCell.x;
		signx = -1;
	}
	else {
		distToCol = gridCell.x + 1 - o.x;
		signx = 1;
	}

	if (dir.y < 0) {
		distToRow = o.y - gridCell.y;
		signy = -1;
	}
	else {
		distToRow = gridCell.y + 1 - o.y;
		signy = 1;
	}

	std::vector<glm::ivec2> cells;
	while (0 <= gridCell.x && 0 <= gridCell.y && gridCell.x < cols && gridCell.y < rows) {
		cells.push_back(gridCell);

		// compute next one
		if (distToCol < distToRow) {
			gridCell.x += signx;
			distToCol += distBetweenColumns;
		}
		else {
			gridCell.y += signy;
			distToRow += distBetweenRows;
		}
	}
	return cells;
}

glm::vec2 Visibility::randomUnitVector2D()
{
	static std::mt19937 rng(std::random_device{}());
	static std::uniform_real_distribution<float> angleDist(0.0f, glm::two_pi<float>());

	float angle = angleDist(rng);
	return glm::vec2(std::cos(angle), std::sin(angle));
}

glm::vec2 Visibility::randomVec2InBounds(float cols, float rows) {
	static std::mt19937 rng(std::random_device{}());
	std::uniform_real_distribution<float> distX(0.0f, cols);
	std::uniform_real_distribution<float> distY(0.0f, rows);

	float x = distX(rng);
	float y = distY(rng);
	return glm::vec2(x, y);
}

std::vector<std::set<int>> Visibility::computeVisibility(const std::vector<std::vector<char>>& mapInfo, int maxRays)
{
	int rows = mapInfo.size();
	int cols = mapInfo[0].size();

	std::vector<std::set<int>> cellsVisibility(rows * cols);

	for (size_t i = 0; i < maxRays; ++i) {
		glm::vec2 randOrigin = randomVec2InBounds(cols, rows);
		glm::vec2 randDir = randomUnitVector2D();
		
		std::vector<glm::ivec2> cells = bresenhamSupercover(randOrigin, randDir, rows, cols);

		std::vector<glm::ivec2> visibleSet;
		
		for (const glm::ivec2& cell : cells) {
			if (mapInfo[cell.y][cell.x] == 1) {
				visibleSet.clear();
			}
			else {
				int index = cell.y * cols + cell.x;
				for (const glm::ivec2& otherCell : visibleSet) { // This cell has visibility over all the previous cells in the visibleSet
					int indexOther = otherCell.y * cols + otherCell.x;
					cellsVisibility[index].insert(indexOther);
				}
				visibleSet.push_back(cell); // Push cell to visible set
			}
		}
	}

	return cellsVisibility;
}
