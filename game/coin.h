#ifndef COIN_H
#define COIN_H

#include <SDL.h>
#include <SDL_image.h>
#include <string>

class Coin {
private:
    SDL_Rect rect;
    float positionX;
    int baseY;
    SDL_Texture* texture;
    bool collected;

public:
    Coin(int x, int y, int w, int h, const std::string& texturePath, SDL_Renderer* renderer);
    ~Coin();
    void update(float distance, float elapsedTime);
    void reset(int x, int y);
    void render(SDL_Renderer* renderer);
    SDL_Rect getRect() const;
    bool isCollected() const;
    void collect();
};

#endif
