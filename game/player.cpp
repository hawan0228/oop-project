#include "player.h"
#include <SDL_image.h>
#include <algorithm>
#include <cmath>
#include <iostream>

Player::Player(int startX, int startY, int w, int h, int moveSpeed, SDL_Color c)
    : x(static_cast<float>(startX)), y(static_cast<float>(startY)),
      width(w), height(h), speed(moveSpeed), texture(nullptr), color(c) {}

Player::~Player() {
    SDL_DestroyTexture(texture);
}

void Player::move(const Uint8* keyState, int screenWidth, int groundY, float deltaTime) {
    if (keyState[SDL_SCANCODE_LEFT]) x -= speed * deltaTime;
    if (keyState[SDL_SCANCODE_RIGHT]) x += speed * deltaTime;
    x = std::clamp(x, 0.0f, static_cast<float>(screenWidth - width));

    const bool jumpPressed = keyState[SDL_SCANCODE_SPACE] || keyState[SDL_SCANCODE_UP];
    if (jumpPressed && !jumpHeld && y >= groundY - height) {
        verticalVelocity = -650.0f;
    }
    jumpHeld = jumpPressed;
    if (y < groundY - height || verticalVelocity < 0.0f) {

        y += verticalVelocity * deltaTime + 0.5f * 1800.0f * deltaTime * deltaTime;
        verticalVelocity += 1800.0f * deltaTime;
        if (y >= groundY - height) {
            y = static_cast<float>(groundY - height);
            verticalVelocity = 0.0f;
        }
    }
    animationTime += deltaTime;
}

void Player::reset(int startX, int startY) {
    x = static_cast<float>(startX);
    y = static_cast<float>(startY);
    verticalVelocity = 0.0f;
    animationTime = 0.0f;
    jumpHeld = false;
}

void Player::render(SDL_Renderer* renderer) const {
    SDL_Rect destination = { static_cast<int>(x), static_cast<int>(y), width, height };
    if (texture) {
        const double tilt = verticalVelocity == 0.0f ? std::sin(animationTime * 18.0f) * 2.0 : 0.0;
        SDL_RenderCopyEx(renderer, texture, nullptr, &destination, tilt, nullptr, SDL_FLIP_NONE);
    } else {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderer, &destination);
    }
}

bool Player::loadTexture(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) {
        std::cout << "Failed to load texture: " << IMG_GetError() << std::endl;
        return false;
    }
    SDL_DestroyTexture(texture);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture != nullptr;
}

bool Player::checkCollision(const SDL_Rect& other) const {

    SDL_Rect playerRect = { static_cast<int>(x) + 6, static_cast<int>(y) + 4,
                            width - 12, height - 8 };
    return SDL_HasIntersection(&playerRect, &other);
}
