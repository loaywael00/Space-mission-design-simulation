#pragma once
// Owner: E1. Frozen contract (Section 3.2).
#include <string>
#include "ConfigAndTheme.h"

class MissionLevel;   // forward declaration only

struct CraftStats {
    float health = 0.0f;
    float strength = 0.0f;
    float thrust = 0.0f;
    float shielding = 0.0f;
    float mass = 0.0f;
    float totalCost = 0.0f;
    float totalBuildTimeSec = 0.0f;
    CraftStats operator+(const CraftStats& o) const;   // field by field
    CraftStats operator-(const CraftStats& o) const;   // field by field
};

// Additive helper (not in the frozen list): "2m 30s" formatting for any seconds value.
std::string FormatDuration(float seconds);

class ShipComponent {
public:
    ShipComponent(std::string id, std::string name, ComponentSlot slot, int tier,
                  CraftStats stats, std::string modelKey, std::string iconKey,
                  std::string description);
    virtual ~ShipComponent() = default;

    const std::string& GetId() const;
    const std::string& GetName() const;
    ComponentSlot GetSlotType() const;
    int GetTier() const;
    const CraftStats& GetStats() const;
    const std::string& GetModelKey() const;
    const std::string& GetIconKey() const;
    const std::string& GetDescription() const;
    std::string GetFormattedBuildTime() const;   // "2m 30s", "1m 00s"

    virtual bool MeetsMissionRequirement(const MissionLevel& level) const = 0;

protected:
    std::string m_id;
    std::string m_name;
    ComponentSlot m_slotType;
    int m_tier;
    CraftStats m_statModifiers;
    std::string m_modelKey;
    std::string m_iconKey;
    std::string m_description;
};
