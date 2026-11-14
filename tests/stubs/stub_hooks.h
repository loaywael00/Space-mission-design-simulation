// TEST-ONLY: lets tests drive the headless raylib stand-in (mouse, keys, draw counters).
#pragma once
#include "raylib.h"

struct StubInput {
    Vector2 mouse {0, 0};
    Vector2 delta {0, 0};
    float wheel = 0.0f;
    bool pressed = false;   // IsMouseButtonPressed(LEFT) this frame
    bool down = false;      // IsMouseButtonDown(LEFT)
    bool keys[512] = {false};
    int modelDraws = 0;     // DrawModel calls since last reset
    int wireDraws = 0;      // DrawModelWiresEx calls since last reset
    int depthMaskOff = 0;   // rlDisableDepthMask calls
    int depthMaskOn = 0;    // rlEnableDepthMask calls
    int textDraws = 0;
    bool lastTextHas(const char* needle) const;
};
StubInput& GetStubInput();
void StubResetFrame();            // clears pressed/delta/wheel and counters (call between simulated frames)
void StubClick(float x, float y); // mouse to (x,y) with a press this frame
void StubMove(float x, float y);  // mouse to (x,y), no press
bool StubTextDrawn(const char* needle);   // was `needle` part of any text drawn since StubResetFrame()
