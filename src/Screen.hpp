#pragma once

struct SDL_Renderer;

class Screen {
public:
    virtual ~Screen() = default;
    virtual void render(SDL_Renderer* renderer) const = 0;
};
