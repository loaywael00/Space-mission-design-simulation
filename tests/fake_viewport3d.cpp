// Owner: E3. Test double: link-time replacement for Viewport3D that only RECORDS what LaunchDirector asks of it.
#include <array>
#include "Viewport3D.h"

struct FakeViewportLog {
    Color chassis{230, 213, 172, 255};
    std::array<Color, 5> slot{};
    std::array<bool, 5> overridden{};
    float autoRotate = 0.0f;
    int shakeCalls = 0;
    float shakeSeconds = 0, shakeMagnitude = 0;
    int clearCalls = 0;
};
FakeViewportLog g_fakeViewport;

void Viewport3D::SetAutoRotateDegPerSec(float v) { g_fakeViewport.autoRotate = v; }
void Viewport3D::SetChassisColor(Color c) { g_fakeViewport.chassis = c; }
void Viewport3D::SetSlotOverride(ComponentSlot s, Color c) {
    g_fakeViewport.slot[static_cast<int>(s)] = c; g_fakeViewport.overridden[static_cast<int>(s)] = true;
}
void Viewport3D::ClearSlotOverrides() { g_fakeViewport.overridden.fill(false); g_fakeViewport.clearCalls++; }
void Viewport3D::TriggerShake(float seconds, float magnitude) {
    g_fakeViewport.shakeCalls++; g_fakeViewport.shakeSeconds = seconds; g_fakeViewport.shakeMagnitude = magnitude;
}
