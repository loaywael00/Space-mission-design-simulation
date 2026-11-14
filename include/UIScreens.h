#pragma once
// Owner: E4. Frozen contract (Section 3.15).
#include <array>
#include "MissionLevel.h"
#include "MissionEvaluator.h"
#include "LaunchDirector.h"

enum class ScreenAction { NONE, START_MISSION, ENTER_HANGAR, LAUNCH,
                          PROCEED, RETURN_TO_HANGAR, RETURN_TO_MENU };

class UIScreens {
public:
    ScreenAction UpdateMainMenu();
    void DrawMainMenu() const;

    ScreenAction UpdateBriefing(const MissionLevel& level);
    void DrawBriefing(const MissionLevel& level) const;

    ScreenAction UpdateDebrief(const MissionEvaluationReport& report, int levelIndex);
    void DrawDebrief(const MissionEvaluationReport& report, int levelIndex) const;

    ScreenAction UpdateComplete();
    void DrawComplete(const std::array<int, 3>& scores) const;

    void DrawLaunchBanner(const LaunchDirector& launch) const;   // top of viewport

    // Button geometry and labels (additive, used by tests)
    static Rectangle MenuButtonRect();
    static Rectangle BriefingButtonRect();
    static Rectangle DebriefButtonRect();
    static Rectangle CompleteButtonRect();
    static const char* DebriefButtonLabel(const MissionEvaluationReport& report, int levelIndex);
private:
    bool m_hoverButton = false;   // set by the Update* of the screen currently shown
};
