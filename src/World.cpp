#include "World.h"
#include <SDL.h>
#include <array>
#include <iostream>

int prevSum = 0;

World::World(const glm::ivec2& size)
	: m_Cells(size.x, std::vector<Cell>(size.y, { nullptr, {0, 0} }))
	, m_Size(size)
{
	m_Sand.Moves = true;
	m_Sand.Density = 1.f;
	m_Sand.Color = { 255, 200, 0, 255 };
	m_Sand.BlockedVelocityCalculation = [&](const std::array<std::array<Cell, 3>, 3>& neighbourhood,
		const glm::vec2& currentVelocity, const glm::ivec2& wantedDir)
	{
		const int randDir = (rand() % 2) * 2 - 1;
		float additionalVelX = 0;

		// go down slope
		if ((neighbourhood[randDir + 1][1].Type == nullptr || neighbourhood[randDir + 1][1].Type->Density < m_Sand.Density) &&
			(neighbourhood[randDir + 1][0].Type == nullptr || neighbourhood[randDir + 1][0].Type->Density < m_Sand.Density))
		{
			additionalVelX = currentVelocity.y * 0.1f * randDir;
			if (abs(additionalVelX) < 0.1f)
				additionalVelX = 0.1f * randDir;
		}

		// bounce
		glm::vec2 newVel;
		newVel.x = currentVelocity.x * -0.1f * abs(wantedDir.x) + additionalVelX;
		newVel.y = currentVelocity.y * -0.1f * abs(wantedDir.y);

		return newVel;
	};

	m_Water.Moves = true;
	m_Water.Density = 0.5;
	m_Water.Color = { 0, 0, 255, 255 };
	m_Water.BlockedVelocityCalculation = [&](const std::array<std::array<Cell, 3>, 3>& neighbourhood,
		const glm::vec2& currentVelocity, const glm::ivec2& wantedDir)
	{
		int dir = glm::sign(currentVelocity.x);
		if (dir == 0)
		{
			dir = (rand() % 2) * 2 - 1;
		}
		float velX = 0;

		// spread out
		if (neighbourhood[dir + 1][1].Type == nullptr)
			velX = dir;
		else
			velX = -dir;

		return glm::vec2{ velX, 0 };
	};

	m_Solid.Moves = false;
	m_Solid.Density = 1.f;
	m_Solid.Color = { 0, 0 , 0, 0 };
	m_Solid.BlockedVelocityCalculation = [](const std::array<std::array<Cell, 3>, 3>& neighbourhood,
		const glm::vec2& currentVelocity, const glm::ivec2& wantedDir)
	{
		return glm::vec2{ 0, 0 };
	};
}

void World::SetCell(int x, int y, const Cell& cell)
{
	// Check if in bounds
	if (x < 0 || x >= m_Size.x ||
		y < 0 || y >= m_Size.y)
	{
		return;
	}

	// Set State
	m_Cells[x][y] = cell;
}

float randFloat()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

void World::Update()
{
	// Apply gravity
	for (int y = 0; y < m_Size.y; ++y)
	{
		for (int x = 0; x < m_Size.x; ++x)
		{
			if (m_Cells[x][y].Type == nullptr ||
				!m_Cells[x][y].Type->Moves)
				continue;

			m_Cells[x][y].Velocity.y += m_Gravity;
		}
	}

	// Calculate wanted directions
	std::vector<std::vector<glm::ivec2>> directions(m_Size.x, std::vector<glm::ivec2>(m_Size.y, { 0, 0 }));
	for (int y = 0; y < m_Size.y; ++y)
	{
		for (int x = 0; x < m_Size.x; ++x)
		{
			if (m_Cells[x][y].Type == nullptr ||
				!m_Cells[x][y].Type->Moves)
				continue;

			directions[x][y].x = (randFloat() < abs(m_Cells[x][y].Velocity.x)) * glm::sign(m_Cells[x][y].Velocity.x);
			directions[x][y].y = (randFloat() < abs(m_Cells[x][y].Velocity.y)) * glm::sign(m_Cells[x][y].Velocity.y);
		}
	}

	// Move cells
	for (int y = 0; y < m_Size.y; ++y)
	{
		for (int x = 0; x < m_Size.x; ++x)
		{
			if (m_Cells[x][y].Type == nullptr)
				continue;

			const auto moveDir = directions[x][y];
			if (moveDir == glm::ivec2{ 0, 0 })
				continue;

			const bool outOfBounds = x + moveDir.x < 0 || x + moveDir.x >= m_Size.x || 
				y + moveDir.y < 0 || y + moveDir.y >= m_Size.y;
			
			const bool blocked = outOfBounds || m_Cells[x + moveDir.x][y + moveDir.y].Type != nullptr;

			// if not blocked, occupy this space
			if (!blocked)
			{
				m_Cells[x + moveDir.x][y + moveDir.y] = m_Cells[x][y];
				m_Cells[x][y] = { nullptr, {0, 0} };
			}
			// else, swap if densities alow it
			else if (!outOfBounds && blocked &&
				(m_Cells[x][y].Type->Density < m_Cells[x + moveDir.x][y + moveDir.y].Type->Density && moveDir.y > 0 ||
				m_Cells[x][y].Type->Density > m_Cells[x + moveDir.x][y + moveDir.y].Type->Density && moveDir.y < 0))
			{
				const Cell temp = m_Cells[x][y];
				m_Cells[x][y] = m_Cells[x + moveDir.x][y + moveDir.y];
				m_Cells[x + moveDir.x][y + moveDir.y] = temp;

				// bouyancy
				m_Cells[x][y].Velocity.y += m_Cells[x + moveDir.x][y + moveDir.y].Type->Density - m_Cells[x][y].Type->Density;
				m_Cells[x + moveDir.x][y + moveDir.y].Velocity.y += m_Cells[x][y].Type->Density - m_Cells[x + moveDir.x][y + moveDir.y].Type->Density;
			}
			// else, change velocity
			else
			{
				// Calculate new velocity
				// get neighbourhood
				std::array<std::array<Cell, 3>, 3> neighbourhood;
				for (int nx = -1; nx <= 1; ++nx)
				{
					for (int ny = -1; ny <= 1; ++ny)
					{
						// Out of bounds = solid
						if (x + nx < 0 || x + nx >= m_Size.x ||
							y + ny < 0 || y + ny >= m_Size.y)
						{
							neighbourhood[nx + 1][ny + 1] = { &m_Solid, {0, 0} };
						}
						else
						{
							neighbourhood[nx + 1][ny + 1] = m_Cells[x + nx][y + ny];
						}
					}
				}

				// call lambda
				m_Cells[x][y].Velocity = m_Cells[x][y].Type->BlockedVelocityCalculation(
					neighbourhood,
					m_Cells[x][y].Velocity,
					moveDir);
			}
		}
	}
}