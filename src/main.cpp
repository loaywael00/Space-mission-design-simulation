// Owner: E2
#include <cstdio>
#include <exception>
#include "raylib.h"
#include "ConfigAndTheme.h"
#include "GameManager.h"

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "NASA SPACECRAFT DESIGN SIMULATION");
    SetTargetFPS(TARGET_FPS);

    GameManager game;
    try {
        game.Init();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "Startup failed: %s\n", e.what());
        CloseWindow();
        return 1;
    }

    while (!WindowShouldClose()) {
        const float dt = GetFrameTime();
        game.Update(dt);
        BeginDrawing();
        ClearBackground(COLOR_BG_DARK_SLATE);
        game.Draw();
        EndDrawing();
    }

    game.Shutdown();
    CloseWindow();
    return 0;
}
