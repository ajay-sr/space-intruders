#pragma once

struct SDL_Renderer;

class Enemy {
public:
    struct Coordinates {
        double x;
        double y;
    };

    static constexpr int Size = 32;

    Enemy(double x = 0.0, double y = 0.0);
    void updateCoordinates(double x, double y);
    Coordinates getCoordinates() const;
    void render(SDL_Renderer* renderer) const;

private:
    Coordinates coordinates_;
};
