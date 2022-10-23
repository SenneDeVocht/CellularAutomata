#pragma once
#include <vector>
#include <SDL_pixels.h>
#include <glm/glm.hpp>
#include <functional>

struct SDL_Renderer;
struct SDL_Window;

class World
{
public:
	struct Cell;

	struct CellType
	{
		bool Moves;
		float Density;
		SDL_Color Color;

		std::function<glm::vec2(const std::array<std::array<Cell, 3>, 3>&, const glm::vec2&, const glm::ivec2&)> BlockedVelocityCalculation;
	};

	struct Cell
	{
		const CellType* Type;
		glm::vec2 Velocity;
	};

	World(const glm::ivec2& size);

	void SetCell(int x, int y, const Cell& cell);
	const CellType* GetSand() const { return &m_Sand; }
	const CellType* GetWater() const { return &m_Water; }
	const CellType* GetSolid() const { return &m_Solid; }

	const std::vector<std::vector<Cell>>& GetCells() const { return m_Cells; }

	void Update();

private:
	std::vector<std::vector<Cell>> m_Cells;
	glm::ivec2 m_Size;

	const float m_Gravity = -0.1f;

	CellType m_Sand;
	CellType m_Water;
	CellType m_Solid;
};
