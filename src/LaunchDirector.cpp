// Owner: E3
#include "LaunchDirector.h"
#include <algorithm>
#include <cmath>

namespace {
constexpr float kStageSeconds = 3.0f;
constexpr float kAutoRotateDegPerSec = 25.0f;
constexpr float kShakeStartSec = 6.0f;     // Stage 3 window opens (index 2)
constexpr float kShakeSeconds = 0.5f;
constexpr float kShakeMagnitude = 0.1f;
constexpr float kHealthDropSeconds = 1.0f;
constexpr int kEnvironmentalStage = 2;
}  // namespace

void LaunchDirector::Start(const MissionEvaluationReport& report) {
    m_report = report;
    m_elapsed = 0.0f;
    m_stageIndex = 0;
    m_lastHighlighted = -1;
    m_firstFailIndex = -1;
    for (int i = 0; i < 4; ++i)
        if (!m_report.stages[i].passed) { m_firstFailIndex = i; break; }
    m_running = true;
    m_finished = false;
    m_shakeFired = false;
}

void LaunchDirector::ApplyHighlight(int stageIndex, Viewport3D& viewport) const {
    // Each window shows only its own parts: reset everything to gold, then colour this stage's parts.
    viewport.ClearSlotOverrides();
    viewport.SetChassisColor(COLOR_CAD_WIRE_GOLD);
    const Color c = m_report.stages[stageIndex].passed ? COLOR_STATUS_NOMINAL : COLOR_STATUS_WARNING;
    switch (stageIndex) {
        case 0: viewport.SetChassisColor(c); break;                       // Stage 1: whole chassis
        case 1: viewport.SetSlotOverride(ComponentSlot::POWER, c); break; // Stage 2
        case 2:                                                           // Stage 3
            viewport.SetSlotOverride(ComponentSlot::MOBILITY, c);
            viewport.SetSlotOverride(ComponentSlot::SHIELDING, c);
            break;
        case 3:                                                           // Stage 4
            viewport.SetSlotOverride(ComponentSlot::PAYLOAD, c);
            viewport.SetSlotOverride(ComponentSlot::OPTICS, c);
            break;
        default: break;
    }
}

void LaunchDirector::Update(float dt, Viewport3D& viewport) {
    if (!m_running) return;
    m_elapsed += dt;

    const float finishAt = (m_firstFailIndex >= 0) ? kStageSeconds * static_cast<float>(m_firstFailIndex + 1)
                                                   : kStageSeconds * 4.0f;
    if (m_elapsed >= finishAt) {
        // The sequence ends at the end of the first failing stage (v2 FIX 7); never show a later stage.
        m_stageIndex = (m_firstFailIndex >= 0) ? m_firstFailIndex : 3;
        m_running = false;
        m_finished = true;
        viewport.ClearSlotOverrides();
        viewport.SetChassisColor(COLOR_CAD_WIRE_GOLD);
        viewport.SetAutoRotateDegPerSec(0.0f);
        return;
    }

    viewport.SetAutoRotateDegPerSec(kAutoRotateDegPerSec);
    m_stageIndex = std::min(3, static_cast<int>(std::floor(m_elapsed / kStageSeconds)));
    if (m_stageIndex != m_lastHighlighted) {
        ApplyHighlight(m_stageIndex, viewport);
        m_lastHighlighted = m_stageIndex;
    }
    if (!m_shakeFired && m_firstFailIndex == kEnvironmentalStage && m_elapsed >= kShakeStartSec) {
        viewport.TriggerShake(kShakeSeconds, kShakeMagnitude);
        m_shakeFired = true;
    }
}

bool LaunchDirector::IsFinished() const { return m_finished; }
bool LaunchDirector::IsRunning() const { return m_running; }
int LaunchDirector::GetCurrentStageIndex() const { return (m_running || m_finished) ? m_stageIndex : -1; }
float LaunchDirector::GetElapsedSeconds() const { return m_elapsed; }

float LaunchDirector::GetAnimatedHealth(float fullHealth) const {
    if ((!m_running && !m_finished) || m_firstFailIndex != kEnvironmentalStage) return fullHealth;
    const float t = std::clamp((m_elapsed - kShakeStartSec) / kHealthDropSeconds, 0.0f, 1.0f);
    return fullHealth * (1.0f - t);
}

const MissionEvaluationReport& LaunchDirector::GetReport() const { return m_report; }
