// Owner: E1
#include "DerivedComponents.h"
#include <utility>
#include "MissionLevel.h"

#define BASE_ARGS std::move(id), std::move(name)

MobilityComponent::MobilityComponent(std::string id, std::string name, int tier, CraftStats stats,
                                     std::string modelKey, std::string iconKey,
                                     std::string description, bool canLandOnSurface)
    : ShipComponent(BASE_ARGS, ComponentSlot::MOBILITY, tier, stats, std::move(modelKey),
                    std::move(iconKey), std::move(description)),
      m_canLandOnSurface(canLandOnSurface) {}
bool MobilityComponent::CanLandOnSurface() const { return m_canLandOnSurface; }
bool MobilityComponent::MeetsMissionRequirement(const MissionLevel& level) const {
    if (!level.GetRequiresSurfaceLandingGear()) return true;
    return m_canLandOnSurface;
}

OpticsComponent::OpticsComponent(std::string id, std::string name, int tier, CraftStats stats,
                                 std::string modelKey, std::string iconKey,
                                 std::string description, int opticalResolutionRating)
    : ShipComponent(BASE_ARGS, ComponentSlot::OPTICS, tier, stats, std::move(modelKey),
                    std::move(iconKey), std::move(description)),
      m_opticalResolutionRating(opticalResolutionRating) {}
int OpticsComponent::GetOpticalResolutionRating() const { return m_opticalResolutionRating; }
bool OpticsComponent::MeetsMissionRequirement(const MissionLevel& level) const {
    return m_opticalResolutionRating >= level.GetMinOpticsResolution();
}

PayloadComponent::PayloadComponent(std::string id, std::string name, int tier, CraftStats stats,
                                   std::string modelKey, std::string iconKey,
                                   std::string description, MissionType cap)
    : ShipComponent(BASE_ARGS, ComponentSlot::PAYLOAD, tier, stats, std::move(modelKey),
                    std::move(iconKey), std::move(description)),
      m_supportedMissionCapability(cap) {}
MissionType PayloadComponent::GetSupportedMissionCapability() const { return m_supportedMissionCapability; }
bool PayloadComponent::MeetsMissionRequirement(const MissionLevel& level) const {
    return m_supportedMissionCapability == level.GetMissionType();
}

PowerComponent::PowerComponent(std::string id, std::string name, int tier, CraftStats stats,
                               std::string modelKey, std::string iconKey,
                               std::string description, float powerOutputWatts)
    : ShipComponent(BASE_ARGS, ComponentSlot::POWER, tier, stats, std::move(modelKey),
                    std::move(iconKey), std::move(description)),
      m_powerOutputWatts(powerOutputWatts) {}
float PowerComponent::GetPowerOutputWatts() const { return m_powerOutputWatts; }
bool PowerComponent::MeetsMissionRequirement(const MissionLevel& level) const {
    return m_powerOutputWatts >= level.GetRequiredPowerWatts();
}

ShieldingComponent::ShieldingComponent(std::string id, std::string name, int tier, CraftStats stats,
                                       std::string modelKey, std::string iconKey,
                                       std::string description, float maxThermal)
    : ShipComponent(BASE_ARGS, ComponentSlot::SHIELDING, tier, stats, std::move(modelKey),
                    std::move(iconKey), std::move(description)),
      m_maxThermalToleranceCelsius(maxThermal) {}
float ShieldingComponent::GetMaxThermalToleranceCelsius() const { return m_maxThermalToleranceCelsius; }
bool ShieldingComponent::MeetsMissionRequirement(const MissionLevel& level) const {
    return m_maxThermalToleranceCelsius >= level.GetMinThermalToleranceCelsius();
}
