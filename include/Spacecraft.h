#pragma once
// Owner: E2. Frozen contract (Section 3.7).
#include <array>
#include <string>
#include "ShipComponent.h"

class Spacecraft {
public:
    void Initialize(const CraftStats& baseChassisStats);  // call once at startup
    void ResetToBareChassis();                             // clears slots and ghost
    void EquipComponent(ShipComponent* part);              // replaces same-slot part
    void UnequipSlot(ComponentSlot slot);
    void SetGhostPreviewComponent(ShipComponent* hoveredPart);  // nullptr clears
    bool AreAllMandatorySlotsFilled() const;               // all 5 slots

    ShipComponent* GetEquippedComponent(ComponentSlot slot) const;  // nullptr if empty
    ShipComponent* GetGhostComponent() const;
    const CraftStats& GetCurrentStats() const;   // thrust here is EFFECTIVE thrust
    const CraftStats& GetPreviewStats() const;   // thrust here is EFFECTIVE thrust
private:
    void RecalculateStats();      // also refreshes the preview
    void RecalculatePreview();
    static float EffectiveThrust(float rawThrust, float totalMass);

    std::string m_baseChassisModelKey = "base_chassis";
    CraftStats m_baseChassisStats;
    std::array<ShipComponent*, 5> m_equippedSlots {nullptr, nullptr, nullptr, nullptr, nullptr};
    ShipComponent* m_ghostPreviewComponent = nullptr;
    CraftStats m_currentTotalStats;
    CraftStats m_previewTotalStats;
    float m_rawThrust = 0.0f;
    float m_previewRawThrust = 0.0f;
};
