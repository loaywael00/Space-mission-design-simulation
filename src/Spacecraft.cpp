// Owner: E2
#include "Spacecraft.h"
#include <algorithm>

namespace {
int Index(ComponentSlot s) { return static_cast<int>(s); }
bool ValidSlot(ComponentSlot s) { return Index(s) >= 0 && Index(s) < 5; }
}  // namespace

float Spacecraft::EffectiveThrust(float rawThrust, float totalMass) {
    // v1 FILE 5 / v2 FIX 1: EffectiveThrust = Raw x (250 / max(250, Mass))
    return rawThrust * (250.0f / std::max(250.0f, totalMass));
}

void Spacecraft::Initialize(const CraftStats& baseChassisStats) {
    m_baseChassisStats = baseChassisStats;
    ResetToBareChassis();
}

void Spacecraft::ResetToBareChassis() {
    m_equippedSlots.fill(nullptr);
    m_ghostPreviewComponent = nullptr;
    RecalculateStats();
}

void Spacecraft::EquipComponent(ShipComponent* part) {
    if (!part || !ValidSlot(part->GetSlotType())) return;
    m_equippedSlots[Index(part->GetSlotType())] = part;
    RecalculateStats();
}

void Spacecraft::UnequipSlot(ComponentSlot slot) {
    if (!ValidSlot(slot)) return;
    m_equippedSlots[Index(slot)] = nullptr;
    RecalculateStats();
}

void Spacecraft::SetGhostPreviewComponent(ShipComponent* hoveredPart) {
    m_ghostPreviewComponent = hoveredPart;
    RecalculatePreview();
}

bool Spacecraft::AreAllMandatorySlotsFilled() const {
    for (const ShipComponent* p : m_equippedSlots)
        if (!p) return false;
    return true;
}

ShipComponent* Spacecraft::GetEquippedComponent(ComponentSlot slot) const {
    return ValidSlot(slot) ? m_equippedSlots[Index(slot)] : nullptr;
}
ShipComponent* Spacecraft::GetGhostComponent() const { return m_ghostPreviewComponent; }
const CraftStats& Spacecraft::GetCurrentStats() const { return m_currentTotalStats; }
const CraftStats& Spacecraft::GetPreviewStats() const { return m_previewTotalStats; }

void Spacecraft::RecalculateStats() {
    CraftStats total = m_baseChassisStats;
    for (const ShipComponent* p : m_equippedSlots)
        if (p) total = total + p->GetStats();
    m_rawThrust = total.thrust;
    total.thrust = EffectiveThrust(m_rawThrust, total.mass);
    m_currentTotalStats = total;
    RecalculatePreview();
}

void Spacecraft::RecalculatePreview() {
    if (!m_ghostPreviewComponent) {
        m_previewTotalStats = m_currentTotalStats;
        m_previewRawThrust = m_rawThrust;
        return;
    }
    // Start from the current totals but with RAW thrust, swap the slot's part, then re-apply the penalty.
    CraftStats p = m_currentTotalStats;
    p.thrust = m_rawThrust;
    const ShipComponent* old = GetEquippedComponent(m_ghostPreviewComponent->GetSlotType());
    if (old) p = p - old->GetStats();
    p = p + m_ghostPreviewComponent->GetStats();
    m_previewRawThrust = p.thrust;
    p.thrust = EffectiveThrust(m_previewRawThrust, p.mass);
    m_previewTotalStats = p;
}
