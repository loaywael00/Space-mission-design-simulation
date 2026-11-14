// Owner: E1
#include "ShipComponent.h"
#include <cmath>
#include <cstdio>
#include <utility>

CraftStats CraftStats::operator+(const CraftStats& o) const {
    CraftStats r;
    r.health = health + o.health;
    r.strength = strength + o.strength;
    r.thrust = thrust + o.thrust;
    r.shielding = shielding + o.shielding;
    r.mass = mass + o.mass;
    r.totalCost = totalCost + o.totalCost;
    r.totalBuildTimeSec = totalBuildTimeSec + o.totalBuildTimeSec;
    return r;
}

CraftStats CraftStats::operator-(const CraftStats& o) const {
    CraftStats r;
    r.health = health - o.health;
    r.strength = strength - o.strength;
    r.thrust = thrust - o.thrust;
    r.shielding = shielding - o.shielding;
    r.mass = mass - o.mass;
    r.totalCost = totalCost - o.totalCost;
    r.totalBuildTimeSec = totalBuildTimeSec - o.totalBuildTimeSec;
    return r;
}

std::string FormatDuration(float seconds) {
    long total = std::lround(seconds);
    if (total < 0) total = 0;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%ldm %02lds", total / 60, total % 60);
    return buf;
}

ShipComponent::ShipComponent(std::string id, std::string name, ComponentSlot slot, int tier,
                             CraftStats stats, std::string modelKey, std::string iconKey,
                             std::string description)
    : m_id(std::move(id)), m_name(std::move(name)), m_slotType(slot), m_tier(tier),
      m_statModifiers(stats), m_modelKey(std::move(modelKey)), m_iconKey(std::move(iconKey)),
      m_description(std::move(description)) {}

const std::string& ShipComponent::GetId() const { return m_id; }
const std::string& ShipComponent::GetName() const { return m_name; }
ComponentSlot ShipComponent::GetSlotType() const { return m_slotType; }
int ShipComponent::GetTier() const { return m_tier; }
const CraftStats& ShipComponent::GetStats() const { return m_statModifiers; }
const std::string& ShipComponent::GetModelKey() const { return m_modelKey; }
const std::string& ShipComponent::GetIconKey() const { return m_iconKey; }
const std::string& ShipComponent::GetDescription() const { return m_description; }
std::string ShipComponent::GetFormattedBuildTime() const {
    return FormatDuration(m_statModifiers.totalBuildTimeSec);
}
