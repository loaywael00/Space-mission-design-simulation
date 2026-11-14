#pragma once
// Owner: E3. Frozen contract (Section 3.9).
#include "MissionEvaluator.h"
#include "Viewport3D.h"

class LaunchDirector {
public:
    void Start(const MissionEvaluationReport& report);   // resets timer to 0
    void Update(float dt, Viewport3D& viewport);         // drives the 12 s sequence
    bool IsFinished() const;
    bool IsRunning() const;                              // true after Start, until finished
    int GetCurrentStageIndex() const;                    // 0..3, -1 before Start
    float GetElapsedSeconds() const;
    float GetAnimatedHealth(float fullHealth) const;     // value to show on the Health gauge
    const MissionEvaluationReport& GetReport() const;
private:
    void ApplyHighlight(int stageIndex, Viewport3D& viewport) const;

    MissionEvaluationReport m_report;
    float m_elapsed = 0.0f;
    int m_stageIndex = -1;
    int m_lastHighlighted = -1;
    int m_firstFailIndex = -1;     // -1 when every stage passed
    bool m_running = false;
    bool m_finished = false;
    bool m_shakeFired = false;
};
