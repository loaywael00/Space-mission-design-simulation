#pragma once
// Owner: E1. Frozen contract (Section 3.5).
#include <string>

struct PlanetReport {
    std::string planetTitle;
    std::string telemetryModeText;
    std::string coordinatesText;
    std::string rangeText;
    std::string environmentType;
    std::string surfaceTempText;
    std::string atmosphereText;
    std::string briefingObjective;
};
