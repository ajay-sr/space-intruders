#include "Enemy.hpp"

#include <SDL.h>
#include <cmath>

Enemy::Enemy(double x, double y) : coordinates_{x, y} {}

void Enemy::updateCoordinates(double x, double y) {
    coordinates_ = {x, y};
}

Enemy::Coordinates Enemy::getCoordinates() const {
    return coordinates_;
}

void Enemy::render(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, 255, 80, 80, 255);
    const SDL_Rect bounds{static_cast<int>(std::lround(coordinates_.x)),
                          static_cast<int>(std::lround(coordinates_.y)), Size, Size};
    SDL_RenderFillRect(renderer, &bounds);
}
