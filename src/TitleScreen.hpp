#pragma once

#include "Screen.hpp"

class TitleScreen final : public Screen {
public:
    void render(SDL_Renderer* renderer) const override;
};
