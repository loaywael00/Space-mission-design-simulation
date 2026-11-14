// Owner: E2
#include "MissionEvaluator.h"
#include <cmath>

namespace {
// Thrust and stats are floats built from simple sums; a tiny epsilon keeps "exactly on target"
// builds (e.g. Level 3 reference: 90.0 vs 90) from failing on float rounding.
constexpr float kEps = 1e-4f;

bool PartMeets(const Spacecraft& craft, ComponentSlot slot, const MissionLevel& level) {
    const ShipComponent* p = craft.GetEquippedComponent(slot);
    return p && p->MeetsMissionRequirement(level);   // an empty slot never meets a requirement
}
}  // namespace

MissionEvaluationReport MissionEvaluator::Evaluate(const Spacecraft& craft, const MissionLevel& level) {
    MissionEvaluationReport report;
    const CraftStats& s = craft.GetCurrentStats();

    // Stage 1: Budget & Launch Window
    {
        StageCheckResult& r = report.stages[0];
        r.stageName = "Budget & Launch Window";
        r.testedSlot = ComponentSlot::MOBILITY;   // unused for Stage 1 (LaunchDirector flashes the chassis)
        bool costOk = s.totalCost <= level.GetMaxBudget() + kEps;
        bool timeOk = s.totalBuildTimeSec <= level.GetMaxBuildTimeSeconds() + kEps;
        r.passed = costOk && timeOk;
        if (!costOk) r.failureReason = "Budget exceeded! NASA funding cap violated.";
        else if (!timeOk) r.failureReason = "Craft finished after the planetary launch window closed!";
    }
    // Stage 2: Propulsion & Power
    {
        StageCheckResult& r = report.stages[1];
        r.stageName = "Propulsion & Power";
        r.testedSlot = ComponentSlot::POWER;
        bool thrustOk = s.thrust + kEps >= level.GetMinRequiredThrust();
        bool powerOk = PartMeets(craft, ComponentSlot::POWER, level);
        r.passed = thrustOk && powerOk;
        if (!thrustOk) r.failureReason = "Insufficient thrust-to-weight ratio! Craft failed to achieve target trajectory.";
        else if (!powerOk) r.failureReason = "Insufficient power output for mission demand!";
    }
    // Stage 3: Environmental Survival
    {
        StageCheckResult& r = report.stages[2];
        r.stageName = "Environmental Survival";
        r.testedSlot = ComponentSlot::SHIELDING;
        r.passed = s.health + kEps >= level.GetMinRequiredHealth() &&
                   s.strength + kEps >= level.GetMinRequiredStrength() &&
                   s.shielding + kEps >= level.GetMinRequiredShielding() &&
                   PartMeets(craft, ComponentSlot::MOBILITY, level) &&
                   PartMeets(craft, ComponentSlot::SHIELDING, level);
        if (!r.passed)
            r.failureReason = "Structural or thermal failure! Hull/Suspension collapsed under planetary conditions.";
    }
    // Stage 4: Scientific Sample
    {
        StageCheckResult& r = report.stages[3];
        r.stageName = "Scientific Sample";
        r.testedSlot = ComponentSlot::PAYLOAD;
        r.passed = PartMeets(craft, ComponentSlot::OPTICS, level) &&
                   PartMeets(craft, ComponentSlot::PAYLOAD, level);
        if (!r.passed)
            r.failureReason = "Incompatible scientific payload equipped for this mission objective!";
    }

    report.overallMissionSuccess = true;
    for (const auto& st : report.stages) report.overallMissionSuccess = report.overallMissionSuccess && st.passed;

    if (report.overallMissionSuccess) {
        float costTerm = 1.0f - s.totalCost / level.GetMaxBudget();
        float timeTerm = 1.0f - s.totalBuildTimeSec / level.GetMaxBuildTimeSeconds();
        report.finalEfficiencyScore = static_cast<int>(std::lround(50.0f + 25.0f * costTerm + 25.0f * timeTerm));
    } else {
        report.finalEfficiencyScore = 0;
    }
    return report;
}
