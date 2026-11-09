#include "PlayerShip.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace {
constexpr double MovementSpeed = 320.0;
constexpr std::uint8_t ship[] = {
    0b00100,
    0b01110,
    0b11111
};
constexpr SDL_Color shipColor{0, 255, 0, 255};
constexpr int ShipColumns = 5;
constexpr int ShipRows = 3;
constexpr int ShipPixelSize = 6;
constexpr int ShipWidth = ShipColumns * ShipPixelSize;
constexpr int ShipHeight = ShipRows * ShipPixelSize;

std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>
createShipTexture(SDL_Renderer* renderer) {
    std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface(
        SDL_CreateRGBSurfaceWithFormat(0, ShipColumns, ShipRows, 32,
                                       SDL_PIXELFORMAT_RGBA8888), SDL_FreeSurface);
    if (!surface) {
        throw std::runtime_error(std::string("Ship surface creation failed: ") + SDL_GetError());
    }
    SDL_FillRect(surface.get(), nullptr, SDL_MapRGBA(surface->format, 0, 0, 0, 0));
    const Uint32 color = SDL_MapRGBA(surface->format, shipColor.r, shipColor.g,
                                    shipColor.b, shipColor.a);
    for (int row = 0; row < ShipRows; ++row) {
        for (int column = 0; column < ShipColumns; ++column) {
            if (ship[row] & (1 << (ShipColumns - 1 - column))) {
                const SDL_Rect pixel{column, row, 1, 1};
                SDL_FillRect(surface.get(), &pixel, color);
            }
        }
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture(
        SDL_CreateTextureFromSurface(renderer, surface.get()), SDL_DestroyTexture);
    if (!texture || SDL_SetTextureBlendMode(texture.get(), SDL_BLENDMODE_BLEND) != 0) {
        throw std::runtime_error(std::string("Ship texture creation failed: ") + SDL_GetError());
    }
    return texture;
}
} // namespace

PlayerShip::PlayerShip(int screenWidth, int screenHeight)
    : screenWidth_(screenWidth), screenHeight_(screenHeight) {
    reset();
}

void PlayerShip::reset() {
    x_ = (screenWidth_ - ShipWidth) / 2.0;
    bounds_ = {static_cast<int>(std::lround(x_)), screenHeight_ - 32 - ShipHeight,
               ShipWidth, ShipHeight};
    movingLeft_ = movingRight_ = false;
}

void PlayerShip::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
        movingLeft_ = movingRight_ = false;
    } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
        const bool pressed = event.type == SDL_KEYDOWN;
        if (event.key.keysym.sym == SDLK_LEFT) {
            movingLeft_ = pressed;
        } else if (event.key.keysym.sym == SDLK_RIGHT) {
            movingRight_ = pressed;
        }
    }
}

void PlayerShip::update(double deltaSeconds) {
    if (deltaSeconds <= 0.0) {
        return;
    }
    const int direction = static_cast<int>(movingRight_) - static_cast<int>(movingLeft_);
    x_ = std::clamp(x_ + direction * MovementSpeed * deltaSeconds,
                    0.0, static_cast<double>(screenWidth_ - bounds_.w));
    bounds_.x = static_cast<int>(std::lround(x_));
}

PlayerShip::Coordinates PlayerShip::getFiringPosition() const {
    return {x_ + bounds_.w / 2.0, static_cast<double>(bounds_.y)};
}

void PlayerShip::render(SDL_Renderer* renderer) const {
    if (!texture_) {
        texture_ = createShipTexture(renderer);
    }
    if (SDL_RenderCopy(renderer, texture_.get(), nullptr, &bounds_) != 0) {
        throw std::runtime_error(std::string("Ship rendering failed: ") + SDL_GetError());
    }
}
