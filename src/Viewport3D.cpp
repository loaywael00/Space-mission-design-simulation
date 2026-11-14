// Owner: E3
#include "Viewport3D.h"
#include <algorithm>
#include <cmath>
#include "rlgl.h"
#include "AssetManager.h"

namespace {
constexpr float kPi = 3.14159265358979323846f;
constexpr float kKeyOrbitDegPerSec = 90.0f;   // A / D
constexpr float kMouseDegPerPixel = 0.4f;
constexpr float kResetSeconds = 0.5f;
constexpr float kZoomStep = 0.5f;
constexpr float kFovMin = 6.0f, kFovMax = 16.0f;
constexpr float kTargetY = 1.2f;
constexpr int kGridHalf = 20;                 // lines at -20..+20 => 40 x 40 cells

bool InRect(Vector2 p, Rectangle r) { return CheckCollisionPointRec(p, r); }
int SlotIndex(ComponentSlot s) { return static_cast<int>(s); }
}  // namespace

float Viewport3D::WrapDegrees(float degrees) {
    float a = std::fmod(degrees, 360.0f);
    if (a < 0.0f) a += 360.0f;
    if (a >= 360.0f) a = 0.0f;   // guards -1e-8 style results
    return a;
}

void Viewport3D::Init() {
    m_orbitAngleDeg = kHomeAngleDeg;
    m_orbitPitchDeg = 35.264f;
    m_orbitRadius = 18.0f;
    m_orthoFovY = 10.0f;
    m_chassisColor = COLOR_CAD_WIRE_GOLD;
    ClearSlotOverrides();
    m_autoRotateDegPerSec = 0.0f;
    m_resetting = false;
    m_dragging = false;
    m_shakeTimeLeft = 0.0f;
    m_shakeOffset = Vector3{0.0f, 0.0f, 0.0f};
    m_camera.up = Vector3{0.0f, 1.0f, 0.0f};
    m_camera.projection = CAMERA_ORTHOGRAPHIC;
    RebuildCamera();
}

void Viewport3D::RebuildCamera() {
    const float theta = m_orbitAngleDeg * kPi / 180.0f;
    const float phi = m_orbitPitchDeg * kPi / 180.0f;
    m_camera.target = Vector3{m_shakeOffset.x, kTargetY + m_shakeOffset.y, m_shakeOffset.z};
    m_camera.position = Vector3{
        m_orbitRadius * std::cos(phi) * std::sin(theta) + m_shakeOffset.x,
        m_orbitRadius * std::sin(phi) + kTargetY + m_shakeOffset.y,
        m_orbitRadius * std::cos(phi) * std::cos(theta) + m_shakeOffset.z};
    m_camera.up = Vector3{0.0f, 1.0f, 0.0f};
    m_camera.fovy = m_orthoFovY;
    m_camera.projection = CAMERA_ORTHOGRAPHIC;
}

void Viewport3D::Update(float dt) {
    float delta = 0.0f;
    if (IsKeyDown(KEY_A)) delta -= kKeyOrbitDegPerSec * dt;
    if (IsKeyDown(KEY_D)) delta += kKeyOrbitDegPerSec * dt;

    const Vector2 mouse = GetMousePosition();
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && InRect(mouse, RECT_VIEWPORT_3D_AREA) &&
        !InRect(mouse, RECT_ROTATION_CONTROLS))   // the on-screen rotate buttons are not a drag handle
        m_dragging = true;
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) m_dragging = false;
    if (m_dragging) delta += GetMouseDelta().x * kMouseDegPerPixel;

    const float wheel = GetMouseWheelMove();
    if (wheel != 0.0f && InRect(mouse, RECT_VIEWPORT_3D_AREA))
        m_orthoFovY = std::clamp(m_orthoFovY - wheel * kZoomStep, kFovMin, kFovMax);

    if (delta != 0.0f) {
        m_resetting = false;   // manual input cancels the smooth reset
        AddOrbitDegrees(delta);
    }

    if (m_resetting) {
        m_resetT = std::min(1.0f, m_resetT + dt / kResetSeconds);
        const float ease = m_resetT * m_resetT * (3.0f - 2.0f * m_resetT);   // smoothstep
        m_orbitAngleDeg = WrapDegrees(m_resetStartDeg + m_resetDeltaDeg * ease);
        if (m_resetT >= 1.0f) {
            m_orbitAngleDeg = kHomeAngleDeg;
            m_resetting = false;
        }
    }

    if (m_autoRotateDegPerSec != 0.0f) AddOrbitDegrees(m_autoRotateDegPerSec * dt);

    if (m_shakeTimeLeft > 0.0f) {
        m_shakeTimeLeft = std::max(0.0f, m_shakeTimeLeft - dt);
        const float falloff = m_shakeTotal > 0.0f ? m_shakeTimeLeft / m_shakeTotal : 0.0f;
        const float m = m_shakeMagnitude * falloff;
        auto rnd = []() { return static_cast<float>(GetRandomValue(-1000, 1000)) / 1000.0f; };
        m_shakeOffset = Vector3{rnd() * m, rnd() * m, rnd() * m};
    } else {
        m_shakeOffset = Vector3{0.0f, 0.0f, 0.0f};
    }
    RebuildCamera();
}

void Viewport3D::AddOrbitDegrees(float degrees) { m_orbitAngleDeg = WrapDegrees(m_orbitAngleDeg + degrees); }

void Viewport3D::StartResetToHomeAngle() {
    m_resetStartDeg = m_orbitAngleDeg;
    float d = std::fmod(kHomeAngleDeg - m_orbitAngleDeg, 360.0f);   // shortest signed path to 45
    if (d > 180.0f) d -= 360.0f;
    if (d < -180.0f) d += 360.0f;
    m_resetDeltaDeg = d;
    m_resetT = 0.0f;
    m_resetting = true;
}

void Viewport3D::SetAutoRotateDegPerSec(float degPerSec) { m_autoRotateDegPerSec = degPerSec; }
void Viewport3D::SetChassisColor(Color color) { m_chassisColor = color; }
void Viewport3D::SetSlotOverride(ComponentSlot slot, Color color) {
    const int i = SlotIndex(slot);
    if (i < 0 || i >= 5) return;
    m_slotColors[i] = color;
    m_slotOverridden[i] = true;
}
void Viewport3D::ClearSlotOverrides() {
    for (int i = 0; i < 5; ++i) { m_slotColors[i] = COLOR_CAD_WIRE_GOLD; m_slotOverridden[i] = false; }
}
void Viewport3D::TriggerShake(float seconds, float magnitude) {
    m_shakeTotal = m_shakeTimeLeft = std::max(0.0f, seconds);
    m_shakeMagnitude = magnitude;
}
float Viewport3D::GetOrbitAngleDeg() const { return m_orbitAngleDeg; }
float Viewport3D::GetOrthoFovY() const { return m_orthoFovY; }

void Viewport3D::DrawGrid() const {
    const float extent = static_cast<float>(kGridHalf);
    for (int i = -kGridHalf; i <= kGridHalf; ++i) {
        const Color c = (i % 5 == 0) ? COLOR_GRID_MAJOR : COLOR_GRID_MINOR;
        const float f = static_cast<float>(i);
        DrawLine3D(Vector3{f, 0.0f, -extent}, Vector3{f, 0.0f, extent}, c);
        DrawLine3D(Vector3{-extent, 0.0f, f}, Vector3{extent, 0.0f, f}, c);
    }
}

// v2 FIX 9, passes 1 and 2.
void Viewport3D::DrawPartTwoPass(Model& model, Color wire) const {
    const Vector3 origin{0.0f, 0.0f, 0.0f};
    // PASS 1: occlusion fill (set the material colour on every draw, models are shared).
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = COLOR_BG_DARK_SLATE;
    DrawModel(model, origin, 1.0f, WHITE);
    // PASS 2: CAD wireframe, must not write depth.
    rlDisableDepthMask();
    DrawModelWiresEx(model, origin, Vector3{0.0f, 1.0f, 0.0f}, 0.0f, Vector3{1.0f, 1.0f, 1.0f}, wire);
    rlEnableDepthMask();
}

// v2 FIX 9, ghost preview.
void Viewport3D::DrawGhost(Model& model) const {
    const Vector3 origin{0.0f, 0.0f, 0.0f};
    model.materials[0].maps[MATERIAL_MAP_DIFFUSE].color = COLOR_GHOST_CYAN_FILL;
    BeginBlendMode(BLEND_ALPHA);
    rlDisableDepthMask();
    DrawModel(model, origin, 1.0f, WHITE);
    rlEnableDepthMask();
    EndBlendMode();

    BeginBlendMode(BLEND_ADDITIVE);
    rlDisableDepthMask();
    DrawModelWiresEx(model, origin, Vector3{0.0f, 1.0f, 0.0f}, 0.0f, Vector3{1.0f, 1.0f, 1.0f},
                     COLOR_GHOST_CYAN_WIRE);
    rlEnableDepthMask();
    EndBlendMode();
}

void Viewport3D::Draw(const Spacecraft& craft) const {
    AssetManager& assets = AssetManager::GetInstance();
    BeginScissorMode(static_cast<int>(RECT_VIEWPORT_3D_AREA.x), static_cast<int>(RECT_VIEWPORT_3D_AREA.y),
                     static_cast<int>(RECT_VIEWPORT_3D_AREA.width), static_cast<int>(RECT_VIEWPORT_3D_AREA.height));
    BeginMode3D(m_camera);
    DrawGrid();                                                   // Step A

    DrawPartTwoPass(assets.GetModel("base_chassis"), m_chassisColor);   // Step B

    const ShipComponent* ghost = craft.GetGhostComponent();
    for (int i = 0; i < 5; ++i) {                                 // Step C
        const ComponentSlot slot = static_cast<ComponentSlot>(i);
        const ShipComponent* part = craft.GetEquippedComponent(slot);
        if (!part) continue;
        if (ghost && ghost->GetSlotType() == slot) continue;      // ghost replaces the equipped part visually
        const Color wire = m_slotOverridden[i] ? m_slotColors[i] : COLOR_CAD_WIRE_GOLD;
        DrawPartTwoPass(assets.GetModel(part->GetModelKey()), wire);
    }
    if (ghost) DrawGhost(assets.GetModel(ghost->GetModelKey())); // Step D

    EndMode3D();
    EndScissorMode();
}
