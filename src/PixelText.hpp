#pragma once

#include <string_view>

struct SDL_Renderer;

void drawCenteredText(SDL_Renderer* renderer, std::string_view text,
                      int width, int y, int scale);
