#include "coin.h"
#include <cmath>
#include <iostream>

Coin::Coin(int x, int y, int w, int h, const std::string& texturePath, SDL_Renderer* renderer)
    : rect{ x, y, w, h }, positionX(static_cast<float>(x)), baseY(y),
      texture(nullptr), collected(false) {
    SDL_Surface* surface = IMG_Load(texturePath.c_str());
    if (!surface) {
        std::cout << "Failed to load coin image: " << IMG_GetError() << std::endl;
        return;
    }
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
}

Coin::~Coin() {
    SDL_DestroyTexture(texture);
}

void Coin::update(float distance, float elapsedTime) {
    positionX -= distance;
    rect.x = static_cast<int>(positionX);
    rect.y = baseY + static_cast<int>(std::sin(elapsedTime * 5.0f) * 4.0f);
}

void Coin::reset(int x, int y) {
    positionX = static_cast<float>(x);
    baseY = y;
    rect.x = x;
    rect.y = y;
    collected = false;
}

void Coin::render(SDL_Renderer* renderer) {
    if (!collected && texture) SDL_RenderCopy(renderer, texture, nullptr, &rect);
}

SDL_Rect Coin::getRect() const { return rect; }
bool Coin::isCollected() const { return collected; }
void Coin::collect() { collected = true; }
