#pragma once

struct SDL_Renderer;
union SDL_Event;

class Screen {
public:
    virtual ~Screen() = default;
    virtual void update(double) {}
    virtual void handleEvent(const SDL_Event&) {}
    virtual void render(SDL_Renderer* renderer) const = 0;
};
