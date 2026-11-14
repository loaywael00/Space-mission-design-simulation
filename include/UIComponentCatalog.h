#pragma once
// Owner: E4. Frozen contract (Section 3.12). Public static layout helpers are additive (used by tests).
#include <vector>
#include "ComponentDatabase.h"
#include "Spacecraft.h"

class UIComponentCatalog {
public:
    // locked == true after launch: no hover, no buy, no scroll.
    void Update(Spacecraft& craft, const ComponentDatabase& db, bool locked);
    void Draw(const Spacecraft& craft) const;

    // Layout (screen pixels). Cards are 360 x 145, five visible; tabs are 48 x 64.
    static constexpr int kVisibleCards = 5;
    static constexpr int kTabCount = 6;
    static Rectangle TabRect(int tabIndex);        // 0 = All Parts, 1..5 = slots in enum order
    static Rectangle CardRect(int visibleIndex);   // 0..4
    static Rectangle BuyRect(int visibleIndex);    // 65 x 30
    static Rectangle PlusRect(int visibleIndex);   // 32 x 30
    static Rectangle ResetButtonRect();            // header "X"

    // Read-only state for tests
    int GetSelectedTab() const { return m_selectedTab; }
    int GetScrollOffset() const { return m_scrollOffsetIndex; }
    int GetFilteredCount() const { return static_cast<int>(m_filtered.size()); }
private:
    int m_selectedTab = 0;                 // 0 = ALL PARTS, 1..5 = ComponentSlot + 1
    int m_scrollOffsetIndex = 0;
    ShipComponent* m_currentlyHoveredPart = nullptr;
    std::vector<ShipComponent*> m_filtered;
    int m_hoverCard = -1;                  // visible index under the mouse, -1 none
    int m_hoverTab = -1;
    bool m_hoverReset = false;
    bool m_hoverBuy = false;               // mouse over the Buy/Remove or +/- button of the hovered card
    bool m_locked = false;
};
