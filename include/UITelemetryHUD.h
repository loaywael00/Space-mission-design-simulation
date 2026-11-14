#pragma once
// Owner: E4. Frozen contract (Section 3.13).
#include <string>
#include "Spacecraft.h"
#include "MissionLevel.h"
#include "LaunchDirector.h"

class UITelemetryHUD {
public:
    // Returns true on the frame the LAUNCH MISSION button is clicked.
    bool Update(const Spacecraft& craft, bool locked);
    // launch may be nullptr in the hangar. When non-null, health uses
    // launch->GetAnimatedHealth(...).
    void Draw(const Spacecraft& craft, const MissionLevel& level,
              const LaunchDirector* launch, bool locked) const;

    // Gauge ranges (v2 FIX 5), exposed for tests.
    static constexpr float kHealthMax = 250.0f;
    static constexpr float kStrengthMax = 250.0f;
    static constexpr float kThrustMax = 150.0f;
    static constexpr int kLedSegments = 12;
    static int LitSegments(float value, float maxValue);   // ceil(value/max*12), clamped 0..12
    bool IsLaunchEnabled() const { return m_launchEnabled; }
private:
    void DrawGauge(float y, const std::string& label, const std::string& valueText, float value, float previewValue,
                   float maxValue, float target, Color barColor, bool segmented, bool over, bool showPreview) const;
    void DrawLedBar(float y, const std::string& label, const std::string& valueText, float value, float maxValue, Color lit) const;

    bool m_launchHover = false;
    bool m_launchEnabled = false;
    Vector2 m_mouse {0.0f, 0.0f};
};
