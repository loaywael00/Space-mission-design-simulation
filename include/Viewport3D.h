#pragma once
// Owner: E3. Frozen contract (Section 3.10).
#include <array>
#include "raylib.h"
#include "Spacecraft.h"
#include "ConfigAndTheme.h"

class Viewport3D {
public:
    void Init();                                         // builds camera, default angles
    void Update(float dt);                               // input, orbit, shake, auto-rotate
    void Draw(const Spacecraft& craft) const;            // BeginScissorMode..EndScissorMode inside
    void AddOrbitDegrees(float degrees);                 // UI buttons and keys call this
    void StartResetToHomeAngle();                        // smooth return to 45 degrees
    void SetAutoRotateDegPerSec(float degPerSec);        // 0.0f stops auto-rotate
    void SetChassisColor(Color color);                   // default COLOR_CAD_WIRE_GOLD
    void SetSlotOverride(ComponentSlot slot, Color color);
    void ClearSlotOverrides();                           // restores each slot to gold
    void TriggerShake(float seconds, float magnitude);   // magnitude in world units
    float GetOrbitAngleDeg() const;
    float GetOrthoFovY() const;                          // additive: current zoom (6..16)

    // Additive helpers (not in the frozen list)
    static float WrapDegrees(float degrees);             // result in [0, 360)
    static constexpr float kHomeAngleDeg = 45.0f;
private:
    void RebuildCamera();
    void DrawGrid() const;
    void DrawPartTwoPass(Model& model, Color wire) const;
    void DrawGhost(Model& model) const;

    Camera3D m_camera {};
    float m_orbitAngleDeg = kHomeAngleDeg;
    float m_orbitPitchDeg = 35.264f;      // atan(1/sqrt(2)) in degrees
    float m_orbitRadius = 18.0f;
    float m_orthoFovY = 10.0f;            // zoom, clamped [6, 16]

    Color m_chassisColor = {230, 213, 172, 255};
    std::array<Color, 5> m_slotColors {};
    std::array<bool, 5> m_slotOverridden {false, false, false, false, false};

    float m_autoRotateDegPerSec = 0.0f;

    bool m_resetting = false;
    float m_resetT = 0.0f;
    float m_resetStartDeg = 0.0f;
    float m_resetDeltaDeg = 0.0f;

    bool m_dragging = false;

    float m_shakeTimeLeft = 0.0f;
    float m_shakeTotal = 0.0f;
    float m_shakeMagnitude = 0.0f;
    Vector3 m_shakeOffset {0.0f, 0.0f, 0.0f};
};
