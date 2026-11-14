// TEST-ONLY headless implementation of the raylib subset (see raylib.h in this folder).
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include "raylib.h"
#include "rlgl.h"
#include "stub_hooks.h"

static StubInput g_in;
static std::vector<std::string> g_texts;

StubInput& GetStubInput() { return g_in; }
bool StubInput::lastTextHas(const char* n) const { return StubTextDrawn(n); }
void StubResetFrame() {
    g_in.pressed = false; g_in.delta = {0, 0}; g_in.wheel = 0.0f;
    g_in.modelDraws = g_in.wireDraws = g_in.depthMaskOff = g_in.depthMaskOn = g_in.textDraws = 0;
    g_texts.clear();
}
void StubClick(float x, float y) { g_in.mouse = {x, y}; g_in.pressed = true; g_in.down = true; }
void StubMove(float x, float y) { g_in.mouse = {x, y}; }
bool StubTextDrawn(const char* n) {
    for (const auto& t : g_texts) if (t.find(n) != std::string::npos) return true;
    return false;
}

static bool FileOk(const char* p) { FILE* f = std::fopen(p, "rb"); if (!f) return false; std::fclose(f); return true; }

extern "C" {
void InitWindow(int, int, const char*) {}
bool WindowShouldClose(void) { return true; }
void CloseWindow(void) {}
void SetTargetFPS(int) {}
float GetFrameTime(void) { return 1.0f / 60.0f; }
void BeginDrawing(void) {}
void EndDrawing(void) {}
void ClearBackground(Color) {}
void BeginScissorMode(int, int, int, int) {}
void EndScissorMode(void) {}
void BeginMode3D(Camera3D) {}
void EndMode3D(void) {}
void BeginBlendMode(int) {}
void EndBlendMode(void) {}
int GetRandomValue(int min, int max) { return (min + max) / 2; }

Vector2 GetMousePosition(void) { return g_in.mouse; }
Vector2 GetMouseDelta(void) { return g_in.delta; }
float GetMouseWheelMove(void) { return g_in.wheel; }
bool IsMouseButtonPressed(int) { return g_in.pressed; }
bool IsMouseButtonDown(int) { return g_in.down; }
bool IsKeyDown(int k) { return k >= 0 && k < 512 && g_in.keys[k]; }
bool CheckCollisionPointRec(Vector2 p, Rectangle r) {
    return p.x >= r.x && p.x < r.x + r.width && p.y >= r.y && p.y < r.y + r.height;
}

void DrawRectangleRec(Rectangle, Color) {}
void DrawRectangleLinesEx(Rectangle, float, Color) {}
void DrawLineEx(Vector2, Vector2, float, Color) {}
void DrawCircleV(Vector2, float, Color) {}
void DrawTriangle(Vector2, Vector2, Vector2, Color) {}
void DrawTexturePro(Texture2D, Rectangle, Rectangle, Vector2, float, Color) {}
void DrawTextEx(Font, const char* text, Vector2, float, float, Color) {
    g_in.textDraws++;
    g_texts.emplace_back(text ? text : "");
}
Vector2 MeasureTextEx(Font, const char* text, float size, float spacing) {
    float n = static_cast<float>(std::strlen(text));
    return Vector2{n * (size * 0.6f + spacing), size};
}

void DrawLine3D(Vector3, Vector3, Color) {}
void DrawModel(Model, Vector3, float, Color) { g_in.modelDraws++; }
void DrawModelWiresEx(Model, Vector3, Vector3, float, Vector3, Color) { g_in.wireDraws++; }
void rlDisableDepthMask(void) { g_in.depthMaskOff++; }
void rlEnableDepthMask(void) { g_in.depthMaskOn++; }

Model LoadModel(const char* path) {
    Model m{0, 0, nullptr};
    if (!FileOk(path)) return m;
    m.meshCount = 1; m.materialCount = 1;
    m.materials = static_cast<Material*>(std::calloc(1, sizeof(Material)));
    m.materials[0].maps = static_cast<MaterialMap*>(std::calloc(12, sizeof(MaterialMap)));
    return m;
}
bool IsModelReady(Model m) { return m.meshCount > 0 && m.materials != nullptr; }
void UnloadModel(Model m) {
    if (m.materials) { std::free(m.materials[0].maps); std::free(m.materials); }
}
Texture2D LoadTexture(const char* path) {
    Texture2D t{0, 0, 0, 0, 0};
    if (FileOk(path)) { t.id = 1; t.width = 64; t.height = 64; }
    return t;
}
bool IsTextureReady(Texture2D t) { return t.id != 0; }
void UnloadTexture(Texture2D) {}
void SetTextureFilter(Texture2D, int) {}
Font LoadFontEx(const char* path, int size, int*, int) {
    Font f{0, 0, 0, {0, 0, 0, 0, 0}};
    if (FileOk(path)) { f.baseSize = size; f.glyphCount = 95; f.texture.id = 1; }
    return f;
}
bool IsFontReady(Font f) { return f.texture.id != 0; }
void UnloadFont(Font) {}
}
