#include "TitleScreen.hpp"

#include "PixelText.hpp"

#include <SDL.h>
#include <algorithm>


void TitleScreen::render(SDL_Renderer* renderer) const {
    int width = 0;
    int height = 0;
    SDL_RenderGetLogicalSize(renderer, &width, &height);
    if ((width == 0 || height == 0)
        && SDL_GetRendererOutputSize(renderer, &width, &height) != 0) {
        return;
    }

    const int titleScale = std::max(1, std::min({7, width / 100, height / 40}));
    const int subtitleScale = std::max(1, titleScale / 2);
    SDL_SetRenderDrawColor(renderer, 80, 255, 200, 255);
    drawCenteredText(renderer, "Space Intruders", width,
                     height / 2 - 9 * titleScale, titleScale);
    SDL_SetRenderDrawColor(renderer, 220, 230, 240, 255);
    drawCenteredText(renderer, "Press any key to start", width,
                     height / 2 + 5 * titleScale, subtitleScale);
}
