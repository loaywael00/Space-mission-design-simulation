// Owner: E1
#include "MissionLevel.h"
#include <fstream>
#include <stdexcept>
#include "third_party/json.hpp"

using json = nlohmann::json;

namespace {
MissionType ParseMissionType(const std::string& s) {
    if (s == "FLYBY_RECON") return MissionType::FLYBY_RECON;
    if (s == "ORBITAL_SAMPLE") return MissionType::ORBITAL_SAMPLE;
    if (s == "SURFACE_CORE_DRILL") return MissionType::SURFACE_CORE_DRILL;
    throw std::runtime_error("Unknown mission_type '" + s + "'");
}
std::string Str(const json& j, const char* key) { return j.at(key).get<std::string>(); }
float Num(const json& j, const char* key) { return j.at(key).get<float>(); }
}  // namespace

std::array<MissionLevel, 3> MissionLevel::LoadAllFromJson(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Cannot open levels file: " + path);
    json root;
    try {
        root = json::parse(in);
    } catch (const std::exception& e) {
        throw std::runtime_error("Malformed levels file '" + path + "': " + e.what());
    }
    std::array<MissionLevel, 3> out;
    try {
        const json& levels = root.at("levels");
        if (!levels.is_array() || levels.size() != 3)
            throw std::runtime_error("expected exactly 3 levels, found " +
                                     std::to_string(levels.is_array() ? levels.size() : 0));
        for (size_t i = 0; i < 3; ++i) {
            const json& j = levels.at(i);
            MissionLevel& L = out[i];
            L.m_levelNumber = j.at("level_number").get<int>();
            if (L.m_levelNumber != static_cast<int>(i) + 1)
                throw std::runtime_error("levels must be listed in order 1,2,3");
            L.m_title = Str(j, "title");
            L.m_missionType = ParseMissionType(Str(j, "mission_type"));
            L.m_maxBudget = Num(j, "max_budget_usd");
            L.m_maxBuildTimeSeconds = Num(j, "max_build_time_sec");
            L.m_minRequiredHealth = Num(j, "min_health");
            L.m_minRequiredStrength = Num(j, "min_strength");
            L.m_minRequiredThrust = Num(j, "min_thrust");
            L.m_minRequiredShielding = Num(j, "min_shielding");
            L.m_requiresSurfaceLandingGear = j.at("requires_surface_landing_gear").get<bool>();
            L.m_minOpticsResolution = j.at("min_optics_resolution").get<int>();
            L.m_requiredPowerWatts = Num(j, "required_power_watts");
            L.m_minThermalToleranceCelsius = Num(j, "min_thermal_tolerance_celsius");
            const json& pr = j.at("planet_report");
            L.m_planetReport.planetTitle = Str(pr, "planet_title");
            L.m_planetReport.telemetryModeText = Str(pr, "telemetry_mode_text");
            L.m_planetReport.coordinatesText = Str(pr, "coordinates_text");
            L.m_planetReport.rangeText = Str(pr, "range_text");
            L.m_planetReport.environmentType = Str(pr, "environment_type");
            L.m_planetReport.surfaceTempText = Str(pr, "surface_temp_text");
            L.m_planetReport.atmosphereText = Str(pr, "atmosphere_text");
            L.m_planetReport.briefingObjective = Str(pr, "briefing_objective");
        }
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("levels file '" + path + "': " + e.what());
    } catch (const std::exception& e) {
        throw std::runtime_error("levels file '" + path + "' is missing or has a bad key: " + e.what());
    }
    return out;
}

int MissionLevel::GetLevelNumber() const { return m_levelNumber; }
const std::string& MissionLevel::GetTitle() const { return m_title; }
MissionType MissionLevel::GetMissionType() const { return m_missionType; }
const PlanetReport& MissionLevel::GetPlanetReport() const { return m_planetReport; }
float MissionLevel::GetMaxBudget() const { return m_maxBudget; }
float MissionLevel::GetMaxBuildTimeSeconds() const { return m_maxBuildTimeSeconds; }
float MissionLevel::GetMinRequiredHealth() const { return m_minRequiredHealth; }
float MissionLevel::GetMinRequiredStrength() const { return m_minRequiredStrength; }
float MissionLevel::GetMinRequiredThrust() const { return m_minRequiredThrust; }
float MissionLevel::GetMinRequiredShielding() const { return m_minRequiredShielding; }
bool MissionLevel::GetRequiresSurfaceLandingGear() const { return m_requiresSurfaceLandingGear; }
int MissionLevel::GetMinOpticsResolution() const { return m_minOpticsResolution; }
float MissionLevel::GetRequiredPowerWatts() const { return m_requiredPowerWatts; }
float MissionLevel::GetMinThermalToleranceCelsius() const { return m_minThermalToleranceCelsius; }
