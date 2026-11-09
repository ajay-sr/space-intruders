#pragma once

#include "Screen.hpp"
#include "Entity.hpp"
#include "Enemy.hpp"
#include "Bullet.hpp"

#include <array>
#include <cstddef>
#include <optional>

class GameScreen final : public Screen {
public:
    static constexpr std::size_t Rows = 4;
    static constexpr std::size_t Columns = 8;
    // Each matrix cell owns the enemy's top-left x/y coordinates.
    using EnemyMatrix = std::array<std::array<Enemy, Columns>, Rows>;

    void update(double deltaSeconds) override;
    void handleEvent(const SDL_Event& event) override;
    void render(SDL_Renderer* renderer) const override;
    bool hasLost() const { return lost_; }
    const EnemyMatrix& getEnemyGrid() const { return enemies_; }
    void markEnemyDead(std::size_t row, std::size_t column);
    bool isEnemyDead(std::size_t row, std::size_t column) const;
    const std::optional<Bullet>& getBullet() const { return bullet_; }

private:
    void moveEnemies(double deltaSeconds);
    void moveLivingEnemies(double deltaX, double deltaY);
    void updateCombat(double deltaSeconds);
    static std::size_t enemyIndex(std::size_t row, std::size_t column);

    int level_ = 1;
    double introSeconds_ = 0.0;
    std::optional<Entity> player_;
    double playerX_ = 464.0;
    bool movingLeft_ = false;
    bool movingRight_ = false;
    std::optional<Bullet> bullet_;
    EnemyMatrix enemies_{};
    // Row-major flags: row * Columns + column corresponds to enemies_[row][column].
    std::array<bool, Rows * Columns> deadEnemies_{};
    int enemyDirection_ = 1;
    bool lost_ = false;
};
