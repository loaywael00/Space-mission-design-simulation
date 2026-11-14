// ============================================================================
// TEST-ONLY HEADLESS STAND-IN for raylib 5.0. NOT the real library.
// Declares only the API subset this project uses, with the same signatures as
// raylib 5.0, so every source file can be compiled, linked and unit-tested on a
// machine with no GPU, window or raylib install. The real build (CMake) never
// puts tests/stubs on the include path for the game executable.
// ============================================================================
#pragma once
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Vector2 { float x, y; } Vector2;
typedef struct Vector3 { float x, y, z; } Vector3;
typedef struct Color { unsigned char r, g, b, a; } Color;
typedef struct Rectangle { float x, y, width, height; } Rectangle;
typedef struct Texture { unsigned int id; int width, height, mipmaps, format; } Texture;
typedef Texture Texture2D;
typedef struct Shader { unsigned int id; int* locs; } Shader;
typedef struct MaterialMap { Texture2D texture; Color color; float value; } MaterialMap;
typedef struct Material { Shader shader; MaterialMap* maps; float params[4]; } Material;
typedef struct Model { int meshCount; int materialCount; Material* materials; } Model;
typedef struct Font { int baseSize; int glyphCount; int glyphPadding; Texture2D texture; } Font;
typedef struct Camera3D { Vector3 position, target, up; float fovy; int projection; } Camera3D;

#define WHITE ((Color){255, 255, 255, 255})

enum { MOUSE_BUTTON_LEFT = 0 };
enum { KEY_A = 65, KEY_D = 68 };
enum { CAMERA_PERSPECTIVE = 0, CAMERA_ORTHOGRAPHIC = 1 };
enum { BLEND_ALPHA = 0, BLEND_ADDITIVE = 1 };
enum { MATERIAL_MAP_DIFFUSE = 0 };
enum { TEXTURE_FILTER_POINT = 0, TEXTURE_FILTER_BILINEAR = 1 };

// Core / window
void InitWindow(int width, int height, const char* title);
bool WindowShouldClose(void);
void CloseWindow(void);
void SetTargetFPS(int fps);
float GetFrameTime(void);
void BeginDrawing(void);
void EndDrawing(void);
void ClearBackground(Color color);
void BeginScissorMode(int x, int y, int width, int height);
void EndScissorMode(void);
void BeginMode3D(Camera3D camera);
void EndMode3D(void);
void BeginBlendMode(int mode);
void EndBlendMode(void);
int GetRandomValue(int min, int max);

// Input
Vector2 GetMousePosition(void);
Vector2 GetMouseDelta(void);
float GetMouseWheelMove(void);
bool IsMouseButtonPressed(int button);
bool IsMouseButtonDown(int button);
bool IsKeyDown(int key);
bool CheckCollisionPointRec(Vector2 point, Rectangle rec);

// 2D drawing
void DrawRectangleRec(Rectangle rec, Color color);
void DrawRectangleLinesEx(Rectangle rec, float lineThick, Color color);
void DrawLineEx(Vector2 startPos, Vector2 endPos, float thick, Color color);
void DrawCircleV(Vector2 center, float radius, Color color);
void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color);
void DrawTexturePro(Texture2D texture, Rectangle source, Rectangle dest, Vector2 origin,
                    float rotation, Color tint);
void DrawTextEx(Font font, const char* text, Vector2 position, float fontSize, float spacing, Color tint);
Vector2 MeasureTextEx(Font font, const char* text, float fontSize, float spacing);

// 3D drawing
void DrawLine3D(Vector3 startPos, Vector3 endPos, Color color);
void DrawModel(Model model, Vector3 position, float scale, Color tint);
void DrawModelWiresEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle,
                      Vector3 scale, Color tint);

// Resources
Model LoadModel(const char* fileName);
bool IsModelReady(Model model);
void UnloadModel(Model model);
Texture2D LoadTexture(const char* fileName);
bool IsTextureReady(Texture2D texture);
void UnloadTexture(Texture2D texture);
void SetTextureFilter(Texture2D texture, int filter);
Font LoadFontEx(const char* fileName, int fontSize, int* codepoints, int codepointCount);
bool IsFontReady(Font font);
void UnloadFont(Font font);

#ifdef __cplusplus
}
#endif
