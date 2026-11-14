#pragma once
// Owner: E1. Frozen contract (Section 3.4).
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "ShipComponent.h"

class ComponentDatabase {
public:
    // Throws std::runtime_error with a readable message on any failure.
    void LoadFromJson(const std::string& partsPath);
    ShipComponent* FindById(const std::string& id) const;        // nullptr if missing
    std::vector<ShipComponent*> GetPartsBySlot(ComponentSlot slot) const;
    std::vector<ShipComponent*> GetAllParts() const;             // catalog order
    const CraftStats& GetBaseChassisStats() const;
    const std::string& GetBaseChassisModelKey() const;           // "base_chassis"
private:
    std::vector<std::unique_ptr<ShipComponent>> m_parts;
    std::unordered_map<std::string, ShipComponent*> m_byId;
    CraftStats m_baseChassisStats;
    std::string m_baseChassisModelKey = "base_chassis";
};
