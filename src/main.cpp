#include "StateManager.hpp"

#include <SDL.h>

#include <algorithm>
#include <chrono>
#include <thread>

int main(int, char**) {
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Space Intruders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Keep pixel art sharp while SDL scales the 960 x 540 game to the window.
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (SDL_RenderSetLogicalSize(renderer, 960, 540) != 0) {
        SDL_Log("Logical render size setup failed: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    StateManager states;
    using Clock = std::chrono::steady_clock;
    const auto frameDuration = std::chrono::duration_cast<Clock::duration>(
        std::chrono::duration<double>(1.0 / 60.0));
    auto previousFrame = Clock::now();
    auto nextFrame = previousFrame;
    bool running = true;
    while (running) {
        const auto frameStart = Clock::now();
        const double deltaSeconds = std::chrono::duration<double>(
            frameStart - previousFrame).count();
        previousFrame = frameStart;

        // Update before input so a newly started level gets a full intro.
        states.update(deltaSeconds);
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else {
                states.handleEvent(event);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        states.render(renderer);
        SDL_RenderPresent(renderer);
        // Pace complete frames, including processing and rendering, to 60 FPS.
        nextFrame = std::max(nextFrame + frameDuration, Clock::now());
        std::this_thread::sleep_until(nextFrame);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
