// Owner: E4
#include "UITelemetryHUD.h"
#include <cmath>
#include "UIDraw.h"

int UITelemetryHUD::LitSegments(float value, float maxValue) {
    if (maxValue <= 0.0f || value <= 0.0f) return 0;
    int lit = static_cast<int>(std::ceil(value / maxValue * static_cast<float>(kLedSegments) - 1e-4f));
    return std::min(kLedSegments, std::max(0, lit));
}

bool UITelemetryHUD::Update(const Spacecraft& craft, bool locked) {
    m_mouse = GetMousePosition();
    m_launchEnabled = craft.AreAllMandatorySlotsFilled() && !locked;
    m_launchHover = PointInRect(m_mouse, RECT_LAUNCH_BUTTON);
    return m_launchEnabled && m_launchHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void UITelemetryHUD::DrawGauge(float y, const std::string& label, const std::string& valueText, float value,
                               float previewValue, float maxValue, float target, Color barColor, bool segmented,
                               bool over, bool showPreview) const {
    const float x = RECT_TELEMETRY_TOP_PANEL.x + 20.0f;
    const float w = RECT_TELEMETRY_TOP_PANEL.width - 40.0f;
    const Rectangle track{x, y + 24.0f, w, 22.0f};
    UiText(label, x, y, 18.0f, COLOR_TEXT_WHITE);
    UiTextRight(valueText, x + w, y, 18.0f, over ? COLOR_STATUS_WARNING : COLOR_TEXT_WHITE);

    DrawRectangleRec(track, COLOR_GRID_MINOR);
    DrawRectangleLinesEx(track, 1.0f, COLOR_GRID_MAJOR);

    const float fillW = w * Clamp01(value / maxValue);
    DrawRectangleRec(Rectangle{track.x, track.y, fillW, track.height}, over ? COLOR_STATUS_WARNING : barColor);
    if (segmented)   // budget gauge: dark separators make the bar look segmented
        for (float sx = track.x + 14.0f; sx < track.x + fillW; sx += 14.0f)
            DrawLineEx({sx, track.y}, {sx, track.y + track.height}, 2.0f, COLOR_GRID_MINOR);

    if (showPreview) {   // translucent cyan band between current and preview values
        const float a = w * Clamp01(std::min(value, previewValue) / maxValue);
        const float b = w * Clamp01(std::max(value, previewValue) / maxValue);
        if (b > a) DrawRectangleRec(Rectangle{track.x + a, track.y, b - a, track.height}, COLOR_GHOST_CYAN_FILL);
    }
    // cyan slider cap at the current value
    DrawRectangleRec(Rectangle{track.x + fillW - 2.0f, track.y - 3.0f, 4.0f, track.height + 6.0f}, COLOR_GHOST_CYAN_WIRE);
    // target marker
    if (target >= 0.0f) {
        const float tx = track.x + w * Clamp01(target / maxValue);
        DrawLineEx({tx, track.y - 6.0f}, {tx, track.y + track.height + 6.0f}, 2.0f, COLOR_BORDER_CYAN);
    }
}

void UITelemetryHUD::DrawLedBar(float y, const std::string& label, const std::string& valueText, float value,
                                float maxValue, Color lit) const {
    const float x = RECT_STATS_BOTTOM_PANEL.x + 20.0f;
    const float w = RECT_STATS_BOTTOM_PANEL.width - 40.0f;
    UiText(label, x, y, 18.0f, COLOR_TEXT_WHITE);
    UiTextRight(valueText, x + w, y, 18.0f, COLOR_TEXT_WHITE);
    const int on = LitSegments(value, maxValue);
    const float segW = w / static_cast<float>(kLedSegments);
    for (int i = 0; i < kLedSegments; ++i)
        DrawRectangleRec(Rectangle{x + segW * static_cast<float>(i), y + 24.0f, segW - 4.0f, 18.0f},
                         i < on ? lit : COLOR_GRID_MINOR);
}

void UITelemetryHUD::Draw(const Spacecraft& craft, const MissionLevel& level, const LaunchDirector* launch,
                          bool locked) const {
    const CraftStats& cur = craft.GetCurrentStats();
    const CraftStats& prev = craft.GetPreviewStats();
    const bool ghost = craft.GetGhostComponent() != nullptr && !locked;
    const float health = launch ? launch->GetAnimatedHealth(cur.health) : cur.health;

    // ---- Top panel: TELEMETRY (5 continuous gauges) ----
    DrawPanel(RECT_TELEMETRY_TOP_PANEL, COLOR_BORDER_CYAN, 2.0f);
    UiText("TELEMETRY", RECT_TELEMETRY_TOP_PANEL.x + 20.0f, RECT_TELEMETRY_TOP_PANEL.y + 12.0f, 24.0f, COLOR_HEADER_TAN);
    const float y0 = RECT_TELEMETRY_TOP_PANEL.y + 60.0f, dy = 66.0f;
    DrawGauge(y0 + 0 * dy, "Health", FormatNumber(health, 0), health, prev.health, kHealthMax,
              level.GetMinRequiredHealth(), COLOR_CAD_WIRE_GOLD, false, false, ghost && !launch);
    DrawGauge(y0 + 1 * dy, "Strength", FormatNumber(cur.strength, 0), cur.strength, prev.strength, kStrengthMax,
              level.GetMinRequiredStrength(), COLOR_CAD_WIRE_GOLD, false, false, ghost);
    DrawGauge(y0 + 2 * dy, "Thrust", FormatNumber(cur.thrust, 1), cur.thrust, prev.thrust, kThrustMax,
              level.GetMinRequiredThrust(), COLOR_CAD_WIRE_GOLD, false, false, ghost);
    DrawGauge(y0 + 3 * dy, "Budget", FormatMoney(cur.totalCost) + " / " + FormatMoney(level.GetMaxBudget()),
              cur.totalCost, prev.totalCost, level.GetMaxBudget(), level.GetMaxBudget(), COLOR_BORDER_GOLD, true,
              cur.totalCost > level.GetMaxBudget(), ghost);
    DrawGauge(y0 + 4 * dy, "Crafting Time",
              FormatDuration(cur.totalBuildTimeSec) + " / " + FormatDuration(level.GetMaxBuildTimeSeconds()),
              cur.totalBuildTimeSec, prev.totalBuildTimeSec, level.GetMaxBuildTimeSeconds(),
              level.GetMaxBuildTimeSeconds(), COLOR_BORDER_CYAN, false,
              cur.totalBuildTimeSec > level.GetMaxBuildTimeSeconds(), ghost);

    // ---- Bottom panel: STATS (12-block LED bars + readouts) ----
    DrawPanel(RECT_STATS_BOTTOM_PANEL, COLOR_BORDER_CYAN, 2.0f);
    UiText("STATS", RECT_STATS_BOTTOM_PANEL.x + 20.0f, RECT_STATS_BOTTOM_PANEL.y + 10.0f, 24.0f, COLOR_HEADER_TAN);
    const float ly = RECT_STATS_BOTTOM_PANEL.y + 45.0f, ldy = 56.0f;
    DrawLedBar(ly + 0 * ldy, "Health", FormatNumber(health, 0), health, kHealthMax, COLOR_BORDER_CYAN);
    DrawLedBar(ly + 1 * ldy, "Strength", FormatNumber(cur.strength, 0), cur.strength, kStrengthMax, COLOR_BORDER_CYAN);
    DrawLedBar(ly + 2 * ldy, "Thrust", FormatNumber(cur.thrust, 1), cur.thrust, kThrustMax, COLOR_CAD_WIRE_GOLD);

    const float by = RECT_STATS_BOTTOM_PANEL.y + 218.0f;
    const Rectangle budgetBox{RECT_STATS_BOTTOM_PANEL.x + 20.0f, by, 180.0f, 56.0f};
    const Rectangle timeBox{RECT_STATS_BOTTOM_PANEL.x + 210.0f, by, 180.0f, 56.0f};
    const bool overBudget = cur.totalCost > level.GetMaxBudget();
    const bool overTime = cur.totalBuildTimeSec > level.GetMaxBuildTimeSeconds();
    DrawRectangleLinesEx(budgetBox, 2.0f, overBudget ? COLOR_STATUS_WARNING : COLOR_BORDER_GOLD);
    DrawRectangleLinesEx(timeBox, 2.0f, overTime ? COLOR_STATUS_WARNING : COLOR_BORDER_CYAN);
    UiText("BUDGET", budgetBox.x + 10.0f, budgetBox.y + 4.0f, 14.0f, COLOR_TEXT_GRAY);
    UiText(FormatMoney(cur.totalCost), budgetBox.x + 10.0f, budgetBox.y + 24.0f, 24.0f,
           overBudget ? COLOR_STATUS_WARNING : COLOR_BORDER_GOLD);
    UiText("CRAFTING TIME", timeBox.x + 10.0f, timeBox.y + 4.0f, 14.0f, COLOR_TEXT_GRAY);
    UiText(FormatDuration(cur.totalBuildTimeSec), timeBox.x + 10.0f, timeBox.y + 24.0f, 24.0f,
           overTime ? COLOR_STATUS_WARNING : COLOR_BORDER_CYAN);

    // ---- LAUNCH MISSION button ----
    const Rectangle B = RECT_LAUNCH_BUTTON;
    const bool enabled = craft.AreAllMandatorySlotsFilled() && !locked;
    DrawRectangleRec(B, enabled && m_launchHover ? WithAlpha(COLOR_BORDER_CYAN, 70) : COLOR_PANEL_BG);
    DrawRectangleLinesEx(Rectangle{B.x + 6.0f, B.y + 6.0f, B.width - 12.0f, B.height - 12.0f}, 2.0f,
                         enabled ? COLOR_BORDER_CYAN : COLOR_GRID_MAJOR);
    DrawCornerBrackets(B, 22.0f, 3.0f, enabled ? COLOR_BORDER_GOLD : COLOR_GRID_MAJOR);
    UiTextCentered(locked ? "MISSION IN PROGRESS" : "LAUNCH MISSION", B, 30.0f,
                   enabled ? COLOR_BORDER_CYAN : COLOR_TEXT_GRAY);

    if (!enabled && !locked && m_launchHover) {   // tooltip follows the mouse
        const std::string tip = "INSTALL ALL 5 SUBSYSTEMS TO LAUNCH";
        const Vector2 m = UiMeasure(tip, 16.0f);
        const Rectangle t{m_mouse.x - m.x - 24.0f, m_mouse.y - 40.0f, m.x + 20.0f, 30.0f};
        DrawRectangleRec(t, COLOR_PANEL_BG);
        DrawRectangleLinesEx(t, 2.0f, COLOR_STATUS_WARNING);
        UiTextCentered(tip, t, 16.0f, COLOR_TEXT_WHITE);
    }
}
