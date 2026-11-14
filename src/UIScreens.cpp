// Owner: E4
#include "UIScreens.h"
#include "UIDraw.h"

namespace {
const Rectangle kFullPanel{360.0f, 120.0f, 1200.0f, 840.0f};

bool ButtonClicked(Rectangle r, bool& hover) {
    hover = PointInRect(GetMousePosition(), r);
    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
const char* MissionTypeName(MissionType t) {
    switch (t) {
        case MissionType::FLYBY_RECON: return "FLYBY RECONNAISSANCE";
        case MissionType::ORBITAL_SAMPLE: return "ORBITAL SAMPLE CAPTURE";
        case MissionType::SURFACE_CORE_DRILL: return "SURFACE CORE DRILLING";
    }
    return "";
}
}  // namespace

Rectangle UIScreens::MenuButtonRect() { return Rectangle{760.0f, 600.0f, 400.0f, 90.0f}; }
Rectangle UIScreens::BriefingButtonRect() { return Rectangle{700.0f, 850.0f, 520.0f, 80.0f}; }
Rectangle UIScreens::DebriefButtonRect() { return Rectangle{680.0f, 860.0f, 560.0f, 80.0f}; }
Rectangle UIScreens::CompleteButtonRect() { return Rectangle{710.0f, 820.0f, 500.0f, 80.0f}; }

const char* UIScreens::DebriefButtonLabel(const MissionEvaluationReport& report, int levelIndex) {
    if (!report.overallMissionSuccess) return "RETURN TO HANGAR & REDESIGN";
    return levelIndex >= 2 ? "VIEW CAMPAIGN RESULTS" : "PROCEED TO NEXT MISSION";   // v2 FIX 10: no "PROCEED" on level 3
}

// ---------------- Main menu ----------------
ScreenAction UIScreens::UpdateMainMenu() {
    return ButtonClicked(MenuButtonRect(), m_hoverButton) ? ScreenAction::START_MISSION : ScreenAction::NONE;
}
void UIScreens::DrawMainMenu() const {
    const std::string title = "NASA SPACECRAFT DESIGN SIMULATION";
    const Vector2 m = UiMeasure(title, 52.0f);
    UiText(title, (SCREEN_WIDTH - m.x) * 0.5f, 340.0f, 52.0f, COLOR_CAD_WIRE_GOLD);
    DrawLineEx({560.0f, 420.0f}, {1360.0f, 420.0f}, 3.0f, COLOR_BORDER_GOLD);
    const std::string sub = "Design. Budget. Launch. Three missions to Mars space.";
    const Vector2 s = UiMeasure(sub, 22.0f);
    UiText(sub, (SCREEN_WIDTH - s.x) * 0.5f, 450.0f, 22.0f, COLOR_TEXT_GRAY);
    DrawButtonBox(MenuButtonRect(), "START MISSION 1", 30.0f, true, m_hoverButton);
}

// ---------------- Briefing ----------------
ScreenAction UIScreens::UpdateBriefing(const MissionLevel&) {
    return ButtonClicked(BriefingButtonRect(), m_hoverButton) ? ScreenAction::ENTER_HANGAR : ScreenAction::NONE;
}
void UIScreens::DrawBriefing(const MissionLevel& level) const {
    DrawPanel(kFullPanel, COLOR_BORDER_GOLD, 3.0f);
    const PlanetReport& pr = level.GetPlanetReport();
    const float x = kFullPanel.x + 50.0f, w = kFullPanel.width - 100.0f;
    UiText("NASA MISSION DOSSIER  -  LEVEL " + std::to_string(level.GetLevelNumber()) + " OF 3", x, kFullPanel.y + 30.0f,
           22.0f, COLOR_HEADER_TAN);
    UiText(level.GetTitle(), x, kFullPanel.y + 64.0f, 46.0f, COLOR_CAD_WIRE_GOLD);
    UiText(std::string("MISSION TYPE: ") + MissionTypeName(level.GetMissionType()), x, kFullPanel.y + 126.0f, 20.0f,
           COLOR_TEXT_GRAY);
    DrawLineEx({x, kFullPanel.y + 162.0f}, {x + w, kFullPanel.y + 162.0f}, 2.0f, COLOR_BORDER_GOLD);
    float y = DrawWrapped(pr.briefingObjective, Rectangle{x, kFullPanel.y + 184.0f, w, 100.0f}, 26.0f, 8.0f, COLOR_TEXT_WHITE);

    y = std::max(y + 20.0f, kFullPanel.y + 330.0f);
    UiText("MISSION REQUIREMENTS", x, y, 22.0f, COLOR_HEADER_TAN);
    y += 40.0f;
    auto row = [&](const std::string& l, const std::string& r, float rx) { UiText(l, x + rx, y, 20.0f, COLOR_TEXT_GRAY); UiText(r, x + rx + 260.0f, y, 20.0f, COLOR_TEXT_WHITE); };
    row("Budget cap", FormatMoney(level.GetMaxBudget()), 0);
    row("Min health", FormatNumber(level.GetMinRequiredHealth(), 0), 560); y += 34.0f;
    row("Launch window", FormatDuration(level.GetMaxBuildTimeSeconds()), 0);
    row("Min strength", FormatNumber(level.GetMinRequiredStrength(), 0), 560); y += 34.0f;
    row("Power needed", FormatNumber(level.GetRequiredPowerWatts(), 0) + " W", 0);
    row("Min thrust", FormatNumber(level.GetMinRequiredThrust(), 0), 560); y += 34.0f;
    row("Thermal tolerance", FormatNumber(level.GetMinThermalToleranceCelsius(), 0) + " C", 0);
    row("Min shielding", FormatNumber(level.GetMinRequiredShielding(), 0), 560); y += 34.0f;
    row("Optics rating", ">= " + std::to_string(level.GetMinOpticsResolution()), 0);
    row("Surface landing gear", level.GetRequiresSurfaceLandingGear() ? "REQUIRED" : "not required", 560);

    DrawButtonBox(BriefingButtonRect(), "ENTER ASSEMBLY HANGAR", 28.0f, true, m_hoverButton);
}

// ---------------- Debrief ----------------
ScreenAction UIScreens::UpdateDebrief(const MissionEvaluationReport& report, int levelIndex) {
    if (!ButtonClicked(DebriefButtonRect(), m_hoverButton)) return ScreenAction::NONE;
    (void)levelIndex;
    return report.overallMissionSuccess ? ScreenAction::PROCEED : ScreenAction::RETURN_TO_HANGAR;
}
void UIScreens::DrawDebrief(const MissionEvaluationReport& report, int levelIndex) const {
    DrawPanel(kFullPanel, report.overallMissionSuccess ? COLOR_STATUS_NOMINAL : COLOR_STATUS_WARNING, 3.0f);
    const float x = kFullPanel.x + 50.0f, w = kFullPanel.width - 100.0f;
    UiText("MISSION " + std::to_string(levelIndex + 1) + " DEBRIEF", x, kFullPanel.y + 28.0f, 22.0f, COLOR_HEADER_TAN);
    UiText(report.overallMissionSuccess ? "MISSION SUCCESS" : "MISSION FAILED", x, kFullPanel.y + 60.0f, 46.0f,
           report.overallMissionSuccess ? COLOR_STATUS_NOMINAL : COLOR_STATUS_WARNING);
    DrawLineEx({x, kFullPanel.y + 126.0f}, {x + w, kFullPanel.y + 126.0f}, 2.0f, COLOR_BORDER_GOLD);

    float y = kFullPanel.y + 150.0f;
    for (size_t i = 0; i < report.stages.size(); ++i) {
        const StageCheckResult& s = report.stages[i];
        const Color c = s.passed ? COLOR_STATUS_NOMINAL : COLOR_STATUS_WARNING;
        UiText("Stage " + std::to_string(i + 1) + ": " + s.stageName, x, y, 26.0f, COLOR_TEXT_WHITE);
        UiTextRight(s.passed ? "PASS" : "FAIL", x + w, y, 26.0f, c);
        if (!s.passed) DrawWrapped(s.failureReason, Rectangle{x + 30.0f, y + 34.0f, w - 60.0f, 60.0f}, 18.0f, 4.0f, c);
        y += s.passed ? 60.0f : 100.0f;
    }
    if (report.overallMissionSuccess) {
        UiText("EFFICIENCY SCORE: " + std::to_string(report.finalEfficiencyScore) + " / 100", x, y + 20.0f, 34.0f,
               COLOR_CAD_WIRE_GOLD);
    }
    DrawButtonBox(DebriefButtonRect(), DebriefButtonLabel(report, levelIndex), 26.0f, true, m_hoverButton);
}

// ---------------- Campaign complete ----------------
ScreenAction UIScreens::UpdateComplete() {
    return ButtonClicked(CompleteButtonRect(), m_hoverButton) ? ScreenAction::RETURN_TO_MENU : ScreenAction::NONE;
}
void UIScreens::DrawComplete(const std::array<int, 3>& scores) const {
    DrawPanel(kFullPanel, COLOR_BORDER_GOLD, 3.0f);
    const std::string title = "MISSION CAMPAIGN COMPLETE";
    const Vector2 m = UiMeasure(title, 50.0f);
    UiText(title, (SCREEN_WIDTH - m.x) * 0.5f, kFullPanel.y + 80.0f, 50.0f, COLOR_CAD_WIRE_GOLD);
    DrawLineEx({560.0f, kFullPanel.y + 160.0f}, {1360.0f, kFullPanel.y + 160.0f}, 3.0f, COLOR_BORDER_GOLD);
    const char* names[3] = {"Mars Flyby Recon", "Phobos Orbital Rock Capture", "Mars Surface Core Sample"};
    float y = kFullPanel.y + 220.0f;
    int total = 0;
    for (int i = 0; i < 3; ++i) {
        UiText("Mission " + std::to_string(i + 1) + ": " + names[i], 560.0f, y, 28.0f, COLOR_TEXT_WHITE);
        UiTextRight(std::to_string(scores[static_cast<size_t>(i)]), 1360.0f, y, 28.0f, COLOR_STATUS_NOMINAL);
        total += scores[static_cast<size_t>(i)];
        y += 64.0f;
    }
    DrawLineEx({560.0f, y}, {1360.0f, y}, 2.0f, COLOR_GRID_MAJOR);
    UiText("TOTAL EFFICIENCY", 560.0f, y + 20.0f, 28.0f, COLOR_HEADER_TAN);
    UiTextRight(std::to_string(total), 1360.0f, y + 20.0f, 28.0f, COLOR_CAD_WIRE_GOLD);
    DrawButtonBox(CompleteButtonRect(), "RETURN TO MAIN MENU", 28.0f, true, m_hoverButton);
}

// ---------------- Launch banner ----------------
void UIScreens::DrawLaunchBanner(const LaunchDirector& launch) const {
    const int idx = launch.GetCurrentStageIndex();
    if (idx < 0) return;
    const StageCheckResult& s = launch.GetReport().stages[static_cast<size_t>(idx)];
    const Rectangle b{RECT_VIEWPORT_3D_AREA.x, RECT_VIEWPORT_3D_AREA.y, RECT_VIEWPORT_3D_AREA.width, 56.0f};
    DrawRectangleRec(b, COLOR_PANEL_BG);
    DrawRectangleLinesEx(b, 2.0f, s.passed ? COLOR_STATUS_NOMINAL : COLOR_STATUS_WARNING);
    UiText("Stage " + std::to_string(idx + 1) + " of 4: " + s.stageName, b.x + 20.0f, b.y + 14.0f, 26.0f, COLOR_TEXT_WHITE);
    UiTextRight(s.passed ? "PASS" : "FAIL", b.x + b.width - 20.0f, b.y + 14.0f, 26.0f,
                s.passed ? COLOR_STATUS_NOMINAL : COLOR_STATUS_WARNING);
}
