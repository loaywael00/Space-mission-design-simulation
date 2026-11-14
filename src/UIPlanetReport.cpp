// Owner: E4
#include "UIPlanetReport.h"
#include "UIDraw.h"

Rectangle UIPlanetReport::RotateLeftRect() {
    return Rectangle{RECT_ROTATION_CONTROLS.x + 10.0f, RECT_ROTATION_CONTROLS.y + 10.0f, 140.0f, 45.0f};
}
Rectangle UIPlanetReport::ResetRect() {
    return Rectangle{RECT_ROTATION_CONTROLS.x + RECT_ROTATION_CONTROLS.width * 0.5f - 25.0f,
                     RECT_ROTATION_CONTROLS.y + 7.5f, 50.0f, 50.0f};
}
Rectangle UIPlanetReport::RotateRightRect() {
    return Rectangle{RECT_ROTATION_CONTROLS.x + RECT_ROTATION_CONTROLS.width - 150.0f,
                     RECT_ROTATION_CONTROLS.y + 10.0f, 140.0f, 45.0f};
}

void UIPlanetReport::Update(Viewport3D& viewport, bool locked) {
    m_locked = locked;
    m_hover = 0;
    m_held = false;
    if (locked) return;
    const Vector2 mouse = GetMousePosition();
    if (PointInRect(mouse, RotateLeftRect())) m_hover = 1;
    else if (PointInRect(mouse, ResetRect())) m_hover = 2;
    else if (PointInRect(mouse, RotateRightRect())) m_hover = 3;

    m_held = (m_hover != 0) && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    const float dt = GetFrameTime();
    if (m_hover == 1 && m_held) viewport.AddOrbitDegrees(-kRotateDegPerSec * dt);      // rotate left (counter-clockwise)
    if (m_hover == 3 && m_held) viewport.AddOrbitDegrees(+kRotateDegPerSec * dt);      // rotate right
    if (m_hover == 2 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) viewport.StartResetToHomeAngle();
}

void UIPlanetReport::Draw(const PlanetReport& report) const {
    // ---- 360 turntable control ----
    const Rectangle L = RotateLeftRect(), R = RotateRightRect(), C = ResetRect();
    auto face = [&](Rectangle r, int id) {
        const bool hot = (m_hover == id) && !m_locked;
        DrawRectangleRec(r, hot && m_held ? WithAlpha(COLOR_BORDER_CYAN, 110)
                                          : (hot ? WithAlpha(COLOR_BORDER_CYAN, 50) : COLOR_PANEL_BG));
        DrawRectangleLinesEx(r, 2.0f, m_locked ? COLOR_GRID_MAJOR : (hot ? COLOR_BORDER_CYAN : COLOR_BORDER_GOLD));
    };
    face(L, 1); face(C, 2); face(R, 3);
    const Color glyph = m_locked ? COLOR_TEXT_GRAY : COLOR_TEXT_WHITE;
    UiTextCentered("<", L, 28.0f, glyph);
    UiTextCentered(">", R, 28.0f, glyph);
    UiTextCentered("v", C, 24.0f, glyph);
    Vector2 m = UiMeasure("360 deg", 14.0f);
    UiText("360 deg", RECT_ROTATION_CONTROLS.x + (RECT_ROTATION_CONTROLS.width - m.x) * 0.5f,
           RECT_ROTATION_CONTROLS.y + RECT_ROTATION_CONTROLS.height + 3.0f, 14.0f, COLOR_TEXT_GRAY);

    // ---- Planet report panel ----
    const Rectangle P = RECT_PLANET_REPORT_PANEL;
    DrawPanel(P, COLOR_BORDER_GOLD, 2.0f);
    const Rectangle header{P.x, P.y, P.width, 42.0f};
    DrawRectangleRec(header, COLOR_HEADER_TAN);
    UiText(report.planetTitle, header.x + 20.0f, header.y + 9.0f, 24.0f, COLOR_BG_DARK_SLATE);

    const float lx = P.x + 24.0f, rx = P.x + P.width * 0.5f + 10.0f;
    float y = P.y + 62.0f;
    const float dy = 44.0f;
    UiText("Telemetry: " + report.telemetryModeText, lx, y, 16.0f, COLOR_TEXT_WHITE);
    UiText(report.environmentType, rx, y, 16.0f, COLOR_TEXT_WHITE);
    y += dy;
    UiText("Data: " + report.coordinatesText, lx, y, 16.0f, COLOR_TEXT_WHITE);
    UiText("Surface Temp: " + report.surfaceTempText, rx, y, 16.0f, COLOR_TEXT_WHITE);
    y += dy;
    UiText("Range: " + report.rangeText, lx, y, 16.0f, COLOR_TEXT_WHITE);
    UiText("Atmosphere: " + report.atmosphereText, rx, y, 16.0f, COLOR_TEXT_WHITE);
}
