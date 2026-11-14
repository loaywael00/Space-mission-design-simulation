// TEST-ONLY stand-in for raylib's rlgl.h (declares just what Viewport3D uses).
#pragma once
#include "raylib.h"
#ifdef __cplusplus
extern "C" {
#endif
void rlDisableDepthMask(void);
void rlEnableDepthMask(void);
#ifdef __cplusplus
}
#endif
