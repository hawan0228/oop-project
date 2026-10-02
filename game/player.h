#ifndef PLAYER_H
#define PLAYER_H

#include <SDL.h>

class Player {
private:
    float x, y;
    int width, height, speed;
    float verticalVelocity = 0.0f;
    float animationTime = 0.0f;
    bool jumpHeld = false;
    SDL_Texture* texture;
    SDL_Color color;

public:
    Player(int startX, int startY, int w, int h, int moveSpeed, SDL_Color c);
    ~Player();
    void move(const Uint8* keyState, int screenWidth, int groundY, float deltaTime);
    void reset(int startX, int startY);
    void render(SDL_Renderer* renderer) const;
    bool checkCollision(const SDL_Rect& other) const;
    bool loadTexture(SDL_Renderer* renderer, const char* filePath);
};

#endif
