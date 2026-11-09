#pragma once

#include <SDL.h>
#include <memory>

class PlayerShip {
public:
    struct Coordinates {
        double x;
        double y;
    };

    PlayerShip(int screenWidth = 960, int screenHeight = 540);

    void handleEvent(const SDL_Event& event);
    void update(double deltaSeconds);
    void reset();
    void render(SDL_Renderer* renderer) const;
    SDL_Rect getBounds() const { return bounds_; }
    Coordinates getFiringPosition() const;

private:
    int screenWidth_;
    int screenHeight_;
    SDL_Rect bounds_{};
    double x_ = 0.0;
    bool movingLeft_ = false;
    bool movingRight_ = false;
    // Retain the texture when resetting position and input for a new level.
    mutable std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>
        texture_{nullptr, SDL_DestroyTexture};
};
