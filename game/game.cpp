#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <random>
#include <string>
#include "player.h"
#include "scoreboard.h"
#include "coin.h"

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int GROUND_Y = 450;

SDL_Texture* loadBackground(SDL_Renderer* renderer, const char* filePath) {
    SDL_Surface* surface = IMG_Load(filePath);
    if (!surface) {
        std::cout << "Failed to load image: " << IMG_GetError() << std::endl;
        return nullptr;
    }
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

bool init(SDL_Window*& window, SDL_Renderer*& renderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return false;
    window = SDL_CreateWindow("Dino Run", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) return false;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    return renderer != nullptr;
}

void close(SDL_Window*& window, SDL_Renderer*& renderer, TTF_Font*& font) {
    if (font) TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text,
                SDL_Color color, int x, int y) {
    SDL_Surface* surface = TTF_RenderUTF8_Solid(font, text, color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect destination = { x, y, surface->w, surface->h };
    SDL_FreeSurface(surface);
    if (texture) {
        SDL_RenderCopy(renderer, texture, nullptr, &destination);
        SDL_DestroyTexture(texture);
    }
}

int main(int argc, char* argv[]) {
    char* basePath = SDL_GetBasePath();
    if (basePath) {
        std::error_code error;
        std::filesystem::current_path(std::filesystem::u8path(basePath), error);
        SDL_free(basePath);
        if (error) return 1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    if (!init(window, renderer) ||
        (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG)) !=
            (IMG_INIT_PNG | IMG_INIT_JPG) || TTF_Init() == -1) {
        std::cout << "Initialization failed: " << SDL_GetError() << std::endl;
        close(window, renderer, font);
        return 1;
    }
    font = TTF_OpenFont("font/Arial.ttf", 24);
    SDL_Texture* backgroundTexture = loadBackground(renderer, "asset/background.png");
    if (!font || !backgroundTexture) {
        std::cout << "Unable to load game assets." << std::endl;
        SDL_DestroyTexture(backgroundTexture);
        close(window, renderer, font);
        return 1;
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    const SDL_Color black = { 60, 60, 60, 255 };
    const SDL_Color gold = { 166, 112, 18, 255 };
    int result = 0;

    {
        Player player(100, GROUND_Y - 50, 50, 50, 220, black);
        Scoreboard scoreboard(20, 18, black, font);
        if (!player.loadTexture(renderer, "asset/dino.png")) {
            result = 1;
        } else {
            Coin coin(600, GROUND_Y - 36, 36, 36, "asset/coin.jpg", renderer);
            Coin coin1(1150, GROUND_Y - 112, 36, 36, "asset/coin1.jpg", renderer);
            SDL_Rect obstacle = { 900, GROUND_Y - 60, 32, 60 };
            const SDL_Rect groundSource = { 0, 115, 500, 35 };
            const SDL_Rect cactusSource = { 570, 96, 42, 40 };
            float obstacleX = 900.0f;
            float groundOffset = 0.0f;
            float elapsedTime = 0.0f;

            SDL_Rect items[2] = { {750, GROUND_Y - 36, 32, 32},
                                  {450, GROUND_Y - 36, 32, 32} };
            float itemX[2] = {750.0f, 450.0f};
            bool itemAvailable[2] = {true, true};
            const SDL_Color itemColors[2] = { {40, 120, 190, 255}, {195, 65, 80, 255} };
            bool shieldActive = false;
            bool obstacleBlocked = false;
            float magnetRemaining = 0.0f;
            bool started = false;
            bool gameOver = false;
            bool focused = true;
            bool running = true;
            std::mt19937 random(SDL_GetTicks());
            Uint64 lastFrame = SDL_GetPerformanceCounter();

            while (running) {
                const Uint64 frameStart = SDL_GetTicks64();
                const Uint64 now = SDL_GetPerformanceCounter();
                const float deltaTime = std::min(
                    static_cast<float>(now - lastFrame) / SDL_GetPerformanceFrequency(), 0.033f);
                lastFrame = now;
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) running = false;
                    if (event.type == SDL_WINDOWEVENT) {
                        if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) focused = false;
                        if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) focused = true;
                    }
                    if (event.type == SDL_KEYDOWN && !event.key.repeat) {
                        if (event.key.keysym.sym == SDLK_ESCAPE) running = false;
                        if (!started && (event.key.keysym.sym == SDLK_SPACE ||
                                         event.key.keysym.sym == SDLK_UP)) started = true;
                        if (gameOver && event.key.keysym.sym == SDLK_r) {
                            player.reset(100, GROUND_Y - 50);
                            scoreboard.resetScore();
                            coin.reset(600, GROUND_Y - 36);
                            coin1.reset(1150, GROUND_Y - 112);
                            obstacleX = 900.0f;
                            obstacle.x = 900;
                            groundOffset = 0.0f;
                            elapsedTime = 0.0f;
                            itemX[0] = 750.0f;
                            itemX[1] = 450.0f;
                            items[0] = {750, GROUND_Y - 36, 32, 32};
                            items[1] = {450, GROUND_Y - 36, 32, 32};
                            itemAvailable[0] = itemAvailable[1] = true;
                            shieldActive = false;
                            obstacleBlocked = false;
                            magnetRemaining = 0.0f;
                            gameOver = false;
                        }
                    }
                }
                if (!running) break;

                if (started && !gameOver && focused) {
                    elapsedTime += deltaTime;
                    const float scrollSpeed = 260.0f + std::min(elapsedTime * 2.0f, 120.0f);
                    const float distance = scrollSpeed * deltaTime;
                    magnetRemaining = std::max(0.0f, magnetRemaining - deltaTime);
                    groundOffset = std::fmod(groundOffset + distance, static_cast<float>(SCREEN_WIDTH));
                    obstacleX -= distance;
                    if (obstacleX < -80.0f) {
                        obstacleX = static_cast<float>(SCREEN_WIDTH + 240 + random() % 360);
                        obstacleBlocked = false;
                    }
                    obstacle.x = static_cast<int>(obstacleX);
                    player.move(SDL_GetKeyboardState(nullptr), SCREEN_WIDTH, GROUND_Y, deltaTime);
                    for (int i = 0; i < 2; ++i) {
                        itemX[i] -= distance;
                        if (itemX[i] < -40.0f) {
                            itemX[i] = static_cast<float>(SCREEN_WIDTH + (i == 0 ? 1000 : 1500));
                            itemAvailable[i] = true;
                        }
                        items[i].x = static_cast<int>(itemX[i]);
                        items[i].y = GROUND_Y - 36 +
                            static_cast<int>(std::sin(elapsedTime * 4.0f + i) * 4.0f);
                        if (itemAvailable[i] && player.checkCollision(items[i])) {
                            itemAvailable[i] = false;
                            if (i == 0) shieldActive = true;
                            else magnetRemaining = 6.0f;
                        }
                    }
                    coin.update(distance, elapsedTime);
                    coin1.update(distance, elapsedTime);
                    if (coin.getRect().x < -40) coin.reset(SCREEN_WIDTH + 300, GROUND_Y - 36);
                    if (coin1.getRect().x < -40) coin1.reset(SCREEN_WIDTH + 680, GROUND_Y - 112);
                    Coin* coins[2] = { &coin, &coin1 };
                    for (int i = 0; i < 2; ++i) {
                        SDL_Rect collectionArea = coins[i]->getRect();
                        if (magnetRemaining > 0.0f) {
                            collectionArea.x -= 140;
                            collectionArea.y -= 140;
                            collectionArea.w += 280;
                            collectionArea.h += 280;
                        }
                        if (!coins[i]->isCollected() && player.checkCollision(collectionArea)) {
                            coins[i]->collect();
                            scoreboard.addScore(i == 0 ? 5 : 10);
                        }
                    }
                    if (!obstacleBlocked && player.checkCollision(obstacle)) {
                        if (shieldActive) {
                            shieldActive = false;
                            obstacleBlocked = true;
                        } else gameOver = true;
                    }
                }

                SDL_SetRenderDrawColor(renderer, 250, 250, 247, 255);
                SDL_RenderClear(renderer);
                SDL_Rect ground = { -static_cast<int>(groundOffset), GROUND_Y - 14, SCREEN_WIDTH, 56 };
                SDL_RenderCopy(renderer, backgroundTexture, &groundSource, &ground);
                ground.x += SCREEN_WIDTH;
                SDL_RenderCopy(renderer, backgroundTexture, &groundSource, &ground);
                SDL_Rect cactus = { obstacle.x - 8, GROUND_Y - 70, 48, 82 };
                SDL_SetTextureAlphaMod(backgroundTexture, obstacleBlocked ? 100 : 255);
                SDL_RenderCopy(renderer, backgroundTexture, &cactusSource, &cactus);
                SDL_SetTextureAlphaMod(backgroundTexture, 255);
                for (int i = 0; i < 2; ++i) {
                    if (!itemAvailable[i] || items[i].x >= SCREEN_WIDTH || items[i].x < -32) continue;
                    SDL_SetRenderDrawColor(renderer, itemColors[i].r, itemColors[i].g, itemColors[i].b, 255);
                    SDL_RenderFillRect(renderer, &items[i]);
                    renderText(renderer, font, i == 0 ? "S" : "M", {255, 255, 255, 255},
                               items[i].x + 7, items[i].y + 2);
                }
                coin.render(renderer);
                coin1.render(renderer);
                player.render(renderer);
                scoreboard.render(renderer);
                renderText(renderer, font, "DINO RUN", gold, 650, 18);
                renderText(renderer, font, "SPACE / UP: jump    LEFT / RIGHT: move", black, 20, 62);
                renderText(renderer, font, shieldActive ? "Shield: READY" : "Shield: -",
                           shieldActive ? itemColors[0] : black, 20, 105);
                const std::string magnetStatus = magnetRemaining > 0.0f ?
                    "Magnet: " + std::to_string(static_cast<int>(std::ceil(magnetRemaining))) + "s" : "Magnet: -";
                renderText(renderer, font, magnetStatus.c_str(),
                           magnetRemaining > 0.0f ? itemColors[1] : black, 255, 105);
                renderText(renderer, font, "S: shield (1 hit)    M: coin magnet (6s)", black, 20, 530);

                if (!started || gameOver || !focused) {
                    SDL_Rect panel = { 150, 170, 500, 150 };
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 238);
                    SDL_RenderFillRect(renderer, &panel);
                    SDL_SetRenderDrawColor(renderer, 210, 210, 200, 255);
                    SDL_RenderDrawRect(renderer, &panel);
                    const char* title = !started ? "READY TO RUN" : gameOver ? "GAME OVER" : "PAUSED";
                    const char* action = !started ? "Press SPACE to start" :
                                         gameOver ? "Press R to try again" : "Return to this window to continue";
                    renderText(renderer, font, title, gold, 250, 190);
                    renderText(renderer, font, action, black, 200, 230);
                    renderText(renderer, font, "Collect coins. Jump over the cactus!", black, 190, 273);
                }
                SDL_RenderPresent(renderer);
                const Uint64 frameDuration = SDL_GetTicks64() - frameStart;
                if (frameDuration < 16) SDL_Delay(static_cast<Uint32>(16 - frameDuration));
            }
        }
    }
    SDL_DestroyTexture(backgroundTexture);
    close(window, renderer, font);
    return result;
}
