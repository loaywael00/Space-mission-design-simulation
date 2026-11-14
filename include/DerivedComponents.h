#pragma once
// Owner: E1. Frozen contract (Section 3.3). Rules: v2 FIX 3.
#include "ShipComponent.h"

class MobilityComponent : public ShipComponent {
public:
    MobilityComponent(std::string id, std::string name, int tier, CraftStats stats,
                      std::string modelKey, std::string iconKey, std::string description,
                      bool canLandOnSurface);
    bool CanLandOnSurface() const;
    bool MeetsMissionRequirement(const MissionLevel& level) const override;
private:
    bool m_canLandOnSurface;
};

class OpticsComponent : public ShipComponent {
public:
    OpticsComponent(std::string id, std::string name, int tier, CraftStats stats,
                    std::string modelKey, std::string iconKey, std::string description,
                    int opticalResolutionRating);
    int GetOpticalResolutionRating() const;
    bool MeetsMissionRequirement(const MissionLevel& level) const override;
private:
    int m_opticalResolutionRating;
};

class PayloadComponent : public ShipComponent {
public:
    PayloadComponent(std::string id, std::string name, int tier, CraftStats stats,
                     std::string modelKey, std::string iconKey, std::string description,
                     MissionType supportedMissionCapability);
    MissionType GetSupportedMissionCapability() const;
    bool MeetsMissionRequirement(const MissionLevel& level) const override;
private:
    MissionType m_supportedMissionCapability;
};

class PowerComponent : public ShipComponent {
public:
    PowerComponent(std::string id, std::string name, int tier, CraftStats stats,
                   std::string modelKey, std::string iconKey, std::string description,
                   float powerOutputWatts);
    float GetPowerOutputWatts() const;
    bool MeetsMissionRequirement(const MissionLevel& level) const override;
private:
    float m_powerOutputWatts;
};

class ShieldingComponent : public ShipComponent {
public:
    ShieldingComponent(std::string id, std::string name, int tier, CraftStats stats,
                       std::string modelKey, std::string iconKey, std::string description,
                       float maxThermalToleranceCelsius);
    float GetMaxThermalToleranceCelsius() const;
    bool MeetsMissionRequirement(const MissionLevel& level) const override;
private:
    float m_maxThermalToleranceCelsius;
};
