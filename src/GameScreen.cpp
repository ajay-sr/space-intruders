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
constexpr double InitialEnemySpeed = 48.0; // Pixels per second.
constexpr double EnemySpeedPerLevel = 12.0;
constexpr double BulletSpeed = 960.0;
constexpr double CombatStep = 1.0 / 120.0;
} // namespace

void GameScreen::handleEvent(const SDL_Event& event) {
    if (lost_) {
        return;
    }
    player_.handleEvent(event);
    if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_UP
        && !event.key.repeat && stageActive_ && !bullet_) {
        const auto muzzle = player_.getFiringPosition();
        bullet_ = Bullet{muzzle.x - Bullet::Width / 2.0, muzzle.y - Bullet::Height};
    }
}

std::size_t GameScreen::enemyIndex(std::size_t row, std::size_t column) {
    if (row >= Rows || column >= Columns) {
        throw std::out_of_range("Enemy grid index out of range");
    }
    return row * Columns + column;
}

void GameScreen::markEnemyDead(std::size_t row, std::size_t column) {
    const auto index = enemyIndex(row, column);
    if (!stageActive_ || lost_) {
        return;
    }
    deadEnemies_[index] = true;
    if (std::all_of(deadEnemies_.begin(), deadEnemies_.end(), [](bool dead) { return dead; })) {
        ++level_;
        resetStage();
    }
}

void GameScreen::resetStage() {
    introSeconds_ = 0.0;
    player_.reset();
    stageActive_ = false;
    bullet_.reset();
    enemies_ = EnemyMatrix{};
    deadEnemies_.fill(false);
    enemyDirection_ = 1;
    lost_ = false;
}

bool GameScreen::isEnemyDead(std::size_t row, std::size_t column) const {
    return deadEnemies_[enemyIndex(row, column)];
}

void GameScreen::update(double deltaSeconds) {
    if (lost_ || deltaSeconds <= 0.0) {
        return;
    }

    if (!stageActive_) {
        introSeconds_ += deltaSeconds;
        if (introSeconds_ < 1.0) {
            return;
        }
        stageActive_ = true;
        for (std::size_t row = 0; row < Rows; ++row) {
            for (std::size_t column = 0; column < Columns; ++column) {
                enemies_[row][column].updateCoordinates(
                    EdgeMargin + column * ColumnSpacing, 32 + row * RowSpacing);
            }
        }
        deltaSeconds = introSeconds_ - 1.0;
    }

    player_.update(deltaSeconds);
    updateCombat(deltaSeconds);
}

void GameScreen::updateCombat(double deltaSeconds) {
    // Small steps keep fast bullets from skipping enemies during a slow frame.
    while (bullet_ && deltaSeconds > 0.0 && !lost_) {
        const double step = std::min(deltaSeconds, CombatStep);
        bullet_->y -= BulletSpeed * step;
        moveEnemies(step);
        if (!stageActive_) {
            return; // A cleared stage gets its full intro on subsequent updates.
        }
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
                if (!stageActive_) {
                    return;
                }
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
    const double speed = InitialEnemySpeed + (level_ - 1) * EnemySpeedPerLevel;
    double distanceRemaining = speed * deltaSeconds;
    while (stageActive_ && distanceRemaining > 0.0 && !lost_) {
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
        if (!stageActive_) {
            return;
        }
        distanceRemaining -= step;

        if (!lost_ && step == distanceToEdge) {
            // Keep the formation together and use leftover time after reversing.
            enemyDirection_ = -enemyDirection_;
            moveLivingEnemies(0.0, RowSpacing);
        }
    }
}

void GameScreen::render(SDL_Renderer* renderer) const {
    if (!stageActive_) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        drawCenteredText(renderer, "Level " + std::to_string(level_), ScreenWidth,
                         (ScreenHeight - 7 * 6) / 2, 6);
        return;
    }

    player_.render(renderer);
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
