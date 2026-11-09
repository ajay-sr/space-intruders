#include "GameScreen.hpp"

#include "PixelText.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
constexpr int ScreenWidth = 960;
constexpr int ScreenHeight = 540;
constexpr int LossLine = ScreenHeight * 3 / 4;
constexpr int ColumnSpacing = 56;
constexpr int RowSpacing = 48;
constexpr int EdgeMargin = 16;
constexpr double EnemySpeed = 96.0; // Pixels per second, independent of frame rate.
constexpr double PlayerSpeed = 320.0;
constexpr double BulletSpeed = 480.0;
constexpr double CombatStep = 1.0 / 120.0;
} // namespace

void GameScreen::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
        movingLeft_ = movingRight_ = false;
        return;
    }
    if (lost_ || (event.type != SDL_KEYDOWN && event.type != SDL_KEYUP)) {
        return;
    }
    const bool pressed = event.type == SDL_KEYDOWN;
    switch (event.key.keysym.sym) {
    case SDLK_LEFT:
        movingLeft_ = pressed;
        break;
    case SDLK_RIGHT:
        movingRight_ = pressed;
        break;
    case SDLK_UP:
        if (pressed && !event.key.repeat && player_ && !bullet_) {
            bullet_ = Bullet{playerX_ + (player_->bounds.w - Bullet::Width) / 2.0,
                             static_cast<double>(player_->bounds.y - Bullet::Height)};
        }
        break;
    default:
        break;
    }
}

std::size_t GameScreen::enemyIndex(std::size_t row, std::size_t column) {
    if (row >= Rows || column >= Columns) {
        throw std::out_of_range("Enemy grid index out of range");
    }
    return row * Columns + column;
}

void GameScreen::markEnemyDead(std::size_t row, std::size_t column) {
    deadEnemies_[enemyIndex(row, column)] = true;
}

bool GameScreen::isEnemyDead(std::size_t row, std::size_t column) const {
    return deadEnemies_[enemyIndex(row, column)];
}

void GameScreen::update(double deltaSeconds) {
    if (lost_ || deltaSeconds <= 0.0) {
        return;
    }

    if (!player_) {
        introSeconds_ += deltaSeconds;
        if (introSeconds_ < 1.0) {
            return;
        }
        player_ = Entity{{464, 476, 32, 32}, {255, 255, 255, 255}};
        for (std::size_t row = 0; row < Rows; ++row) {
            for (std::size_t column = 0; column < Columns; ++column) {
                enemies_[row][column].updateCoordinates(
                    EdgeMargin + column * ColumnSpacing, 32 + row * RowSpacing);
            }
        }
        deltaSeconds = introSeconds_ - 1.0;
    }

    const int direction = static_cast<int>(movingRight_) - static_cast<int>(movingLeft_);
    playerX_ = std::clamp(playerX_ + direction * PlayerSpeed * deltaSeconds,
                         0.0, static_cast<double>(ScreenWidth - player_->bounds.w));
    player_->bounds.x = static_cast<int>(std::lround(playerX_));
    updateCombat(deltaSeconds);
}

void GameScreen::updateCombat(double deltaSeconds) {
    // Small steps keep fast bullets from skipping enemies during a slow frame.
    while (bullet_ && deltaSeconds > 0.0 && !lost_) {
        const double step = std::min(deltaSeconds, CombatStep);
        bullet_->y -= BulletSpeed * step;
        moveEnemies(step);
        if (bullet_ && bullet_->y + Bullet::Height <= 0.0) {
            bullet_.reset();
        }
        deltaSeconds -= step;
    }
    if (!lost_) {
        moveEnemies(deltaSeconds);
    }
}

void GameScreen::moveLivingEnemies(double deltaX, double deltaY) {
    for (std::size_t row = 0; row < Rows; ++row) {
        for (std::size_t column = 0; column < Columns; ++column) {
            if (isEnemyDead(row, column)) {
                continue;
            }
            Enemy& enemy = enemies_[row][column];
            const auto position = enemy.getCoordinates();
            enemy.updateCoordinates(position.x + deltaX, position.y + deltaY);
            const auto updated = enemy.getCoordinates();
            // Include the enemy's swept area when it steps down a row.
            if (bullet_ && bullet_->x < std::max(position.x, updated.x) + Enemy::Size
                && bullet_->x + Bullet::Width > std::min(position.x, updated.x)
                && bullet_->y < std::max(position.y, updated.y) + Enemy::Size
                && bullet_->y + Bullet::Height > std::min(position.y, updated.y)) {
                markEnemyDead(row, column);
                bullet_.reset();
                continue;
            }
            // Check every live enemy immediately after either kind of movement.
            if (enemy.getCoordinates().y + Enemy::Size > LossLine) {
                lost_ = true;
            }
        }
    }
}

void GameScreen::moveEnemies(double deltaSeconds) {
    double distanceRemaining = EnemySpeed * deltaSeconds;
    while (distanceRemaining > 0.0 && !lost_) {
        double left = std::numeric_limits<double>::infinity();
        double right = -std::numeric_limits<double>::infinity();
        for (std::size_t row = 0; row < Rows; ++row) {
            for (std::size_t column = 0; column < Columns; ++column) {
                if (isEnemyDead(row, column)) {
                    continue;
                }
                const auto position = enemies_[row][column].getCoordinates();
                left = std::min(left, position.x);
                right = std::max(right, position.x + Enemy::Size);
            }
        }
        if (left == std::numeric_limits<double>::infinity()) {
            return; // No living enemies remain to move or cross the loss line.
        }
        const double distanceToEdge = std::max(0.0, enemyDirection_ > 0
            ? ScreenWidth - EdgeMargin - right : left - EdgeMargin);
        const double step = std::min(distanceRemaining, distanceToEdge);
        moveLivingEnemies(enemyDirection_ * step, 0.0);
        distanceRemaining -= step;

        if (!lost_ && step == distanceToEdge) {
            // Keep the formation together and use leftover time after reversing.
            enemyDirection_ = -enemyDirection_;
            moveLivingEnemies(0.0, RowSpacing);
        }
    }
}

void GameScreen::render(SDL_Renderer* renderer) const {
    if (!player_) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        drawCenteredText(renderer, "Level " + std::to_string(level_), ScreenWidth,
                         (ScreenHeight - 7 * 6) / 2, 6);
        return;
    }

    const SDL_Color color = player_->color;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &player_->bounds);
    if (bullet_) {
        const SDL_Rect bounds{static_cast<int>(std::lround(bullet_->x)),
                              static_cast<int>(std::lround(bullet_->y)),
                              Bullet::Width, Bullet::Height};
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &bounds);
    }
    for (std::size_t row = 0; row < Rows; ++row) {
        for (std::size_t column = 0; column < Columns; ++column) {
            if (!isEnemyDead(row, column)) {
                enemies_[row][column].render(renderer);
            }
        }
    }
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawLine(renderer, 0, LossLine, ScreenWidth - 1, LossLine);

    if (lost_) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        const SDL_Rect backdrop{300, 230, 360, 80};
        SDL_RenderFillRect(renderer, &backdrop);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        drawCenteredText(renderer, "Game Over", ScreenWidth,
                         (ScreenHeight - 7 * 5) / 2, 5);
    }
}
