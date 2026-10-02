#include "scoreboard.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <string>
#include <iostream>

Scoreboard::Scoreboard(int startX, int startY, SDL_Color c, TTF_Font* font)
    : score(0), x(startX), y(startY), color(c), font(font) {
}

void Scoreboard::addScore(int points) {
    score += points;
}

void Scoreboard::resetScore() {
    score = 0;
}

void Scoreboard::render(SDL_Renderer* renderer) const {
    std::string scoreText = "Score: " + std::to_string(score);
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, scoreText.c_str(), color);
    if (textSurface == nullptr) {
        std::cout << "Unable to render text surface: " << TTF_GetError() << std::endl;
        return;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect renderQuad = { x, y, textSurface->w, textSurface->h };
    SDL_RenderCopy(renderer, texture, nullptr, &renderQuad);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(texture);
}
