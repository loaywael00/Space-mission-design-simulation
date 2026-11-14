// Owner: E4
#include "UIComponentCatalog.h"
#include <algorithm>
#include "UIDraw.h"

namespace {
const char* kTabLabels[UIComponentCatalog::kTabCount] = {"ALL", "MOB", "OPT", "PAY", "PWR", "SHD"};
const char* kTabNames[UIComponentCatalog::kTabCount] = {"ALL PARTS", "MOBILITY", "OPTICS", "PAYLOAD", "POWER", "SHIELDING"};
}  // namespace

Rectangle UIComponentCatalog::TabRect(int i) {
    return Rectangle{RECT_CATALOG_SIDEBAR_TABS.x, RECT_CATALOG_SIDEBAR_TABS.y + static_cast<float>(i) * 71.0f, 48.0f, 64.0f};
}
Rectangle UIComponentCatalog::CardRect(int i) {
    return Rectangle{RECT_CATALOG_MAIN_PANEL.x + 15.0f, 135.0f + static_cast<float>(i) * 155.0f, 360.0f, 145.0f};
}
Rectangle UIComponentCatalog::BuyRect(int i) {
    Rectangle c = CardRect(i);
    return Rectangle{c.x + 247.0f, c.y + 105.0f, 65.0f, 30.0f};
}
Rectangle UIComponentCatalog::PlusRect(int i) {
    Rectangle c = CardRect(i);
    return Rectangle{c.x + 318.0f, c.y + 105.0f, 32.0f, 30.0f};
}
Rectangle UIComponentCatalog::ResetButtonRect() {
    return Rectangle{RECT_CATALOG_MAIN_PANEL.x + RECT_CATALOG_MAIN_PANEL.width - 40.0f, RECT_CATALOG_MAIN_PANEL.y + 10.0f, 30.0f, 30.0f};
}

void UIComponentCatalog::Update(Spacecraft& craft, const ComponentDatabase& db, bool locked) {
    m_locked = locked;
    m_hoverCard = -1; m_hoverTab = -1; m_hoverReset = false; m_hoverBuy = false;

    const Vector2 mouse = GetMousePosition();
    const bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (!locked) {
        // Header X resets the filter; tabs choose the filter.
        if (PointInRect(mouse, ResetButtonRect())) {
            m_hoverReset = true;
            if (clicked) { m_selectedTab = 0; m_scrollOffsetIndex = 0; }
        }
        for (int t = 0; t < kTabCount; ++t) {
            if (PointInRect(mouse, TabRect(t))) {
                m_hoverTab = t;
                if (clicked && t != m_selectedTab) { m_selectedTab = t; m_scrollOffsetIndex = 0; }
            }
        }
    }

    // Filtered list for the current tab (rebuilt each frame; cheap, avoids stale pointers).
    m_filtered = (m_selectedTab == 0) ? db.GetAllParts()
                                      : db.GetPartsBySlot(static_cast<ComponentSlot>(m_selectedTab - 1));
    const int maxScroll = std::max(0, static_cast<int>(m_filtered.size()) - kVisibleCards);

    if (!locked && PointInRect(mouse, RECT_CATALOG_MAIN_PANEL)) {
        const float wheel = GetMouseWheelMove();
        if (wheel > 0.0f) m_scrollOffsetIndex -= 1;       // one card per notch
        else if (wheel < 0.0f) m_scrollOffsetIndex += 1;
    }
    m_scrollOffsetIndex = std::clamp(m_scrollOffsetIndex, 0, maxScroll);

    // Hover and Buy / Remove.
    ShipComponent* hovered = nullptr;
    if (!locked) {
        for (int i = 0; i < kVisibleCards; ++i) {
            const int idx = m_scrollOffsetIndex + i;
            if (idx >= static_cast<int>(m_filtered.size())) break;
            if (PointInRect(mouse, CardRect(i))) {
                m_hoverCard = i;
                hovered = m_filtered[idx];
                m_hoverBuy = PointInRect(mouse, BuyRect(i)) || PointInRect(mouse, PlusRect(i));
                if (clicked && m_hoverBuy) {
                    const ComponentSlot slot = hovered->GetSlotType();
                    if (craft.GetEquippedComponent(slot) == hovered) craft.UnequipSlot(slot);
                    else craft.EquipComponent(hovered);
                }
                break;
            }
        }
    }
    m_currentlyHoveredPart = hovered;
    craft.SetGhostPreviewComponent(hovered);   // nullptr clears (also when locked)
}

void UIComponentCatalog::Draw(const Spacecraft& craft) const {
    const Rectangle P = RECT_CATALOG_MAIN_PANEL;
    DrawPanel(P, COLOR_BORDER_GOLD, 2.0f);

    // Header bar
    UiText("MODULAR PARTS", P.x + 16.0f, P.y + 12.0f, 24.0f, COLOR_HEADER_TAN);
    const Rectangle X = ResetButtonRect();
    DrawRectangleLinesEx(X, 2.0f, m_hoverReset ? COLOR_BORDER_CYAN : COLOR_BORDER_GOLD);
    UiTextCentered("X", X, 18.0f, COLOR_TEXT_WHITE);

    // Filter strip
    UiText(std::string("FILTER: ") + kTabNames[m_selectedTab], P.x + 16.0f, P.y + 62.0f, 16.0f, COLOR_TEXT_GRAY);
    UiTextRight(std::to_string(m_filtered.size()) + " PARTS", P.x + P.width - 16.0f, P.y + 62.0f, 16.0f, COLOR_TEXT_GRAY);

    // Tabs (docked on the left edge)
    for (int t = 0; t < kTabCount; ++t) {
        const Rectangle r = TabRect(t);
        const bool sel = (t == m_selectedTab);
        DrawRectangleRec(r, sel ? WithAlpha(COLOR_BORDER_GOLD, 90) : COLOR_PANEL_BG);
        DrawRectangleLinesEx(r, sel ? 3.0f : 2.0f, sel ? COLOR_BORDER_GOLD : (t == m_hoverTab ? COLOR_BORDER_CYAN : COLOR_GRID_MAJOR));
        UiTextCentered(kTabLabels[t], r, 14.0f, sel ? COLOR_TEXT_WHITE : COLOR_TEXT_GRAY);
    }

    // Cards
    for (int i = 0; i < kVisibleCards; ++i) {
        const int idx = m_scrollOffsetIndex + i;
        if (idx >= static_cast<int>(m_filtered.size())) break;
        const ShipComponent* part = m_filtered[idx];
        const Rectangle c = CardRect(i);
        const bool equipped = craft.GetEquippedComponent(part->GetSlotType()) == part;
        const bool hover = (i == m_hoverCard) && !m_locked;

        DrawRectangleRec(c, COLOR_BG_DARK_SLATE);
        DrawRectangleLinesEx(c, hover ? 3.0f : 2.0f,
                             hover ? COLOR_BORDER_GOLD : (equipped ? COLOR_STATUS_NOMINAL : COLOR_GRID_MAJOR));

        const Texture2D& icon = AssetManager::GetInstance().GetIcon(part->GetIconKey());
        DrawTexturePro(icon, Rectangle{0, 0, static_cast<float>(icon.width), static_cast<float>(icon.height)},
                       Rectangle{c.x + 10.0f, c.y + 17.5f, 110.0f, 110.0f}, Vector2{0, 0}, 0.0f, WHITE);

        UiText(part->GetName(), c.x + 130.0f, c.y + 12.0f, 20.0f, COLOR_TEXT_WHITE);
        UiText("Cost: " + FormatMoney(part->GetStats().totalCost), c.x + 130.0f, c.y + 44.0f, 16.0f, COLOR_TEXT_GRAY);
        UiText("Build Time: " + part->GetFormattedBuildTime(), c.x + 130.0f, c.y + 66.0f, 16.0f, COLOR_TEXT_GRAY);
        UiText("Tier " + std::to_string(part->GetTier()), c.x + 130.0f, c.y + 112.0f, 16.0f, COLOR_HEADER_TAN);

        const bool buyHot = hover && m_hoverBuy;
        DrawButtonBox(BuyRect(i), equipped ? "Remove" : "Buy", 16.0f, !m_locked, buyHot);
        DrawButtonBox(PlusRect(i), equipped ? "-" : "+", 18.0f, !m_locked, buyHot);
    }

    // Footer: scroll position
    const int total = static_cast<int>(m_filtered.size());
    const int first = total == 0 ? 0 : m_scrollOffsetIndex + 1;
    const int last = std::min(total, m_scrollOffsetIndex + kVisibleCards);
    UiText("Showing " + std::to_string(first) + "-" + std::to_string(last) + " of " + std::to_string(total),
           P.x + 16.0f, P.y + P.height - 50.0f, 16.0f, COLOR_TEXT_GRAY);
    if (m_locked) UiText("SYSTEMS LOCKED - MISSION IN PROGRESS", P.x + 16.0f, P.y + P.height - 28.0f, 16.0f, COLOR_STATUS_WARNING);
}
