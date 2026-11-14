#pragma once
// Owner: E4. Frozen contract (Section 3.1). Constants live here only (R4).
#include "raylib.h"

// Window
constexpr int SCREEN_WIDTH  = 1920;
constexpr int SCREEN_HEIGHT = 1080;
constexpr int TARGET_FPS    = 60;

// Colors (defined once in src/ConfigAndTheme.cpp)
extern const Color COLOR_BG_DARK_SLATE;      // {24,32,38,255}
extern const Color COLOR_GRID_MINOR;         // {36,48,56,255}
extern const Color COLOR_GRID_MAJOR;         // {48,64,74,255}
extern const Color COLOR_CAD_WIRE_GOLD;      // {230,213,172,255}
extern const Color COLOR_GHOST_CYAN_WIRE;    // {0,229,255,255}
extern const Color COLOR_GHOST_CYAN_FILL;    // {0,180,216,75}
extern const Color COLOR_PANEL_BG;           // {20,28,34,235}
extern const Color COLOR_BORDER_GOLD;        // {168,149,102,255}
extern const Color COLOR_BORDER_CYAN;        // {56,182,204,255}
extern const Color COLOR_STATUS_NOMINAL;     // {76,209,124,255}
extern const Color COLOR_STATUS_WARNING;     // {235,77,75,255}
extern const Color COLOR_TEXT_WHITE;         // {245,245,245,255}
extern const Color COLOR_TEXT_GRAY;          // {170,175,180,255}
extern const Color COLOR_HEADER_TAN;         // {185,168,122,255}
extern const Color COLOR_BG_WHITE_MODE;      // {248,249,250,255} optional light theme
extern const Color COLOR_WIRE_BLACK_MODE;    // {25,30,36,255}    optional light theme

// Panel rectangles (1920 x 1080)
extern const Rectangle RECT_CATALOG_SIDEBAR_TABS; // {20,110,48,420}
extern const Rectangle RECT_CATALOG_MAIN_PANEL;   // {68,40,390,960}
extern const Rectangle RECT_VIEWPORT_3D_AREA;     // {460,40,990,760}
extern const Rectangle RECT_ROTATION_CONTROLS;    // {760,730,380,65}
extern const Rectangle RECT_PLANET_REPORT_PANEL;  // {630,825,650,215}
extern const Rectangle RECT_TELEMETRY_TOP_PANEL;  // {1470,40,410,430}
extern const Rectangle RECT_STATS_BOTTOM_PANEL;   // {1470,620,410,290}
extern const Rectangle RECT_LAUNCH_BUTTON;        // {1470,935,410,95}

enum class GameState { MAIN_MENU, LEVEL_BRIEFING, HANGAR_ASSEMBLY,
                       LAUNCH_EVALUATION, MISSION_DEBRIEF, GAME_COMPLETE };
enum class ComponentSlot { MOBILITY = 0, OPTICS = 1, PAYLOAD = 2,
                           POWER = 3, SHIELDING = 4, COUNT = 5 };
enum class MissionType { FLYBY_RECON = 1, ORBITAL_SAMPLE = 2, SURFACE_CORE_DRILL = 3 };
