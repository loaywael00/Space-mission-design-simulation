#pragma once
// Owner: E2. Frozen contract (Section 3.8). Rules: v2 FIX 4.
#include <array>
#include <string>
#include "Spacecraft.h"
#include "MissionLevel.h"

struct StageCheckResult {
    std::string stageName;
    bool passed = false;
    ComponentSlot testedSlot = ComponentSlot::MOBILITY;
    std::string failureReason;      // empty when passed
};

struct MissionEvaluationReport {
    bool overallMissionSuccess = false;
    std::array<StageCheckResult, 4> stages;
    int finalEfficiencyScore = 0;   // 0 when overallMissionSuccess is false
};

class MissionEvaluator {
public:
    static MissionEvaluationReport Evaluate(const Spacecraft& craft, const MissionLevel& level);
};
