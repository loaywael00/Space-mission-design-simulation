#pragma once
// Owner: E4. Internal 2D drawing helpers shared by the UI classes (additive; not part of the frozen contract).
// Everything here is Draw-side EXCEPT PointInRect, which UI Update() methods use for hit-testing.
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>
#include "raylib.h"
#include "AssetManager.h"
#include "ConfigAndTheme.h"

inline Color WithAlpha(Color c, unsigned char a) { return Color{c.r, c.g, c.b, a}; }

inline bool PointInRect(Vector2 p, Rectangle r) { return CheckCollisionPointRec(p, r); }

inline void UiText(const std::string& s, float x, float y, float size, Color c) {
    DrawTextEx(AssetManager::GetInstance().GetUIFont(), s.c_str(), Vector2{x, y}, size, 1.0f, c);
}
inline Vector2 UiMeasure(const std::string& s, float size) {
    return MeasureTextEx(AssetManager::GetInstance().GetUIFont(), s.c_str(), size, 1.0f);
}
inline void UiTextCentered(const std::string& s, Rectangle r, float size, Color c) {
    Vector2 m = UiMeasure(s, size);
    UiText(s, r.x + (r.width - m.x) * 0.5f, r.y + (r.height - m.y) * 0.5f, size, c);
}
inline void UiTextRight(const std::string& s, float rightX, float y, float size, Color c) {
    Vector2 m = UiMeasure(s, size);
    UiText(s, rightX - m.x, y, size, c);
}

inline void DrawPanel(Rectangle r, Color border, float thickness = 2.0f) {
    DrawRectangleRec(r, COLOR_PANEL_BG);
    DrawRectangleLinesEx(r, thickness, border);
}

// Gold corner brackets (used by the launch button frame).
inline void DrawCornerBrackets(Rectangle r, float len, float thick, Color c) {
    const float x0 = r.x, y0 = r.y, x1 = r.x + r.width, y1 = r.y + r.height;
    DrawLineEx({x0, y0}, {x0 + len, y0}, thick, c); DrawLineEx({x0, y0}, {x0, y0 + len}, thick, c);
    DrawLineEx({x1, y0}, {x1 - len, y0}, thick, c); DrawLineEx({x1, y0}, {x1, y0 + len}, thick, c);
    DrawLineEx({x0, y1}, {x0 + len, y1}, thick, c); DrawLineEx({x0, y1}, {x0, y1 - len}, thick, c);
    DrawLineEx({x1, y1}, {x1 - len, y1}, thick, c); DrawLineEx({x1, y1}, {x1, y1 - len}, thick, c);
}

inline void DrawButtonBox(Rectangle r, const std::string& label, float size, bool enabled, bool hover) {
    DrawRectangleRec(r, hover && enabled ? WithAlpha(COLOR_BORDER_CYAN, 60) : COLOR_PANEL_BG);
    DrawRectangleLinesEx(r, 2.0f, enabled ? (hover ? COLOR_BORDER_CYAN : COLOR_BORDER_GOLD) : COLOR_GRID_MAJOR);
    UiTextCentered(label, r, size, enabled ? COLOR_TEXT_WHITE : COLOR_TEXT_GRAY);
}

inline std::string FormatMoney(float usd) {
    long v = std::lround(usd);
    bool neg = v < 0;
    if (neg) v = -v;
    std::string digits = std::to_string(v), out;
    for (size_t i = 0; i < digits.size(); ++i) {
        out += digits[i];
        size_t left = digits.size() - 1 - i;
        if (left > 0 && left % 3 == 0) out += ',';
    }
    return std::string(neg ? "-$" : "$") + out;
}

inline std::string FormatNumber(float v, int decimals) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.*f", decimals, static_cast<double>(v));
    return buf;
}

inline float Clamp01(float v) { return std::min(1.0f, std::max(0.0f, v)); }

// Greedy word wrap using the real font metrics.
inline std::vector<std::string> WrapText(const std::string& text, float maxWidth, float size) {
    std::vector<std::string> lines;
    std::string line, word;
    auto flushWord = [&]() {
        if (word.empty()) return;
        std::string candidate = line.empty() ? word : line + " " + word;
        if (!line.empty() && UiMeasure(candidate, size).x > maxWidth) { lines.push_back(line); line = word; }
        else line = candidate;
        word.clear();
    };
    for (char ch : text) {
        if (ch == ' ') flushWord();
        else word += ch;
    }
    flushWord();
    if (!line.empty()) lines.push_back(line);
    return lines;
}

inline float DrawWrapped(const std::string& text, Rectangle area, float size, float lineGap, Color c) {
    float y = area.y;
    for (const std::string& l : WrapText(text, area.width, size)) {
        UiText(l, area.x, y, size, c);
        y += size + lineGap;
    }
    return y;
}
