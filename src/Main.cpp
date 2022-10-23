﻿#include <iostream>
#include <chrono>
#include <SDL.h>
#undef main

#include "World.h"

// Size
constexpr int g_WindowWidth = 90*5;
constexpr int g_WindowHeight = 90*5;
constexpr int g_WorldWidth = 90;
constexpr int g_WorldHeight = 90;

// Rendering
SDL_Renderer* g_pRenderer = nullptr;

// Input
bool g_LeftMousePressed = false;
bool g_RightMousePressed = false;
int g_MouseX, g_MouseY;

bool HandleInput()
{
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
			case SDL_QUIT:
				return true;

			default:
				break;
		}
	}

	uint32_t buttons = SDL_GetMouseState(&g_MouseX, &g_MouseY);
	g_MouseY = g_WindowHeight - g_MouseY;
	g_LeftMousePressed = buttons & SDL_BUTTON_LMASK;
	g_RightMousePressed = buttons & SDL_BUTTON_RMASK;

	return false;
}

// Render
void RenderWorld(const World& world)
{
	constexpr float cellWidth = static_cast<float>(g_WindowWidth) / static_cast<float>(g_WorldWidth);
	constexpr float cellHeight = static_cast<float>(g_WindowHeight) / static_cast<float>(g_WorldHeight);

	const auto& cells = world.GetCells();

	for (int x = 0; x < g_WorldWidth; ++x)
	{
		for (int y = 0; y < g_WorldHeight; ++y)
		{
			if (cells[x][y].Type == nullptr)
				continue;

			SDL_FRect rect{
				x * cellWidth,
				g_WindowHeight - (y + 1) * cellHeight,
				cellWidth,
				cellHeight
			};

			const auto color = cells[x][y].Type->Color;
			SDL_SetRenderDrawColor(g_pRenderer, color.r, color.g, color.b, color.a);
			SDL_RenderFillRectF(g_pRenderer, &rect);
		}
	}
}

int main()
{
    // Init SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
        std::cout << "error initializing SDL:" << SDL_GetError() << std::endl;

    // Create window and renderer
    SDL_Window* pWindow = SDL_CreateWindow(
		"Cellular Automata", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		g_WindowWidth, g_WindowHeight, 
		0);

	g_pRenderer = SDL_CreateRenderer(pWindow, -1, SDL_RENDERER_ACCELERATED);

	// Create world
	World world({ g_WorldWidth, g_WorldHeight });

	// Timer
	float timer = 0;
	constexpr float updateInterval = 1/60.f;
	//constexpr float updateInterval = 0.2f;

	// Loop
    bool quit = false;
	while (!quit)
	{
		auto start = std::chrono::high_resolution_clock::now();

		quit = HandleInput();

		while (timer >= updateInterval)
		{
			// input
			const int worldX = (float)g_MouseX / g_WindowWidth * g_WorldWidth;
			const int worldY = (float)g_MouseY / g_WindowHeight * g_WorldHeight;

			if (g_LeftMousePressed)
				world.SetCell(worldX, worldY, { world.GetSand(), {0, 0} });

			if (g_RightMousePressed)
			{
				world.SetCell(worldX, worldY, { world.GetWater(), {0, 0} });
				world.SetCell(worldX + 1, worldY, { world.GetWater(), {0, 0} });
				world.SetCell(worldX - 1, worldY, { world.GetWater(), {0, 0} });
				world.SetCell(worldX, worldY + 1, { world.GetWater(), {0, 0} });
				world.SetCell(worldX, worldY - 1, { world.GetWater(), {0, 0} });
			}

			world.Update();
			timer -= updateInterval;
		}
		
		SDL_SetRenderDrawColor(g_pRenderer, 255, 255, 255, 255);
		SDL_RenderClear(g_pRenderer);

		RenderWorld(world);

		SDL_RenderPresent(g_pRenderer);

		auto end = std::chrono::high_resolution_clock::now();
		timer += std::chrono::duration<float>(end - start).count();
	}

    return 0;
}