#pragma once
// Owner: E1. Frozen contract (Section 3.6).
#include <array>
#include <string>
#include "ConfigAndTheme.h"
#include "PlanetReport.h"

class MissionLevel {
public:
    // Throws std::runtime_error on failure. Returns levels 1, 2, 3 in order.
    static std::array<MissionLevel, 3> LoadAllFromJson(const std::string& path);

    int GetLevelNumber() const;
    const std::string& GetTitle() const;
    MissionType GetMissionType() const;
    const PlanetReport& GetPlanetReport() const;
    float GetMaxBudget() const;
    float GetMaxBuildTimeSeconds() const;
    float GetMinRequiredHealth() const;
    float GetMinRequiredStrength() const;
    float GetMinRequiredThrust() const;
    float GetMinRequiredShielding() const;
    bool GetRequiresSurfaceLandingGear() const;
    int GetMinOpticsResolution() const;
    float GetRequiredPowerWatts() const;
    float GetMinThermalToleranceCelsius() const;
private:
    int m_levelNumber = 0;
    std::string m_title;
    MissionType m_missionType = MissionType::FLYBY_RECON;
    PlanetReport m_planetReport;
    float m_maxBudget = 0, m_maxBuildTimeSeconds = 0;
    float m_minRequiredHealth = 0, m_minRequiredStrength = 0;
    float m_minRequiredThrust = 0, m_minRequiredShielding = 0;
    bool m_requiresSurfaceLandingGear = false;
    int m_minOpticsResolution = 0;
    float m_requiredPowerWatts = 0, m_minThermalToleranceCelsius = 0;
};
