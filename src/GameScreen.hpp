#pragma once

#include "Screen.hpp"

class GameScreen final : public Screen {
public:
    void render(SDL_Renderer*) const override {}
};
