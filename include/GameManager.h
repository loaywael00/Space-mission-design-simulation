#pragma once
// Owner: E2. Frozen contract (Section 3.16).
#include <array>
#include "ComponentDatabase.h"
#include "MissionLevel.h"
#include "Spacecraft.h"
#include "Viewport3D.h"
#include "LaunchDirector.h"
#include "UIComponentCatalog.h"
#include "UITelemetryHUD.h"
#include "UIPlanetReport.h"
#include "UIScreens.h"

class GameManager {
public:
    void Init();                  // load data, assets, levels; throws on failure
    void Update(float dt);
    void Draw() const;
    void Shutdown();              // unload assets

    // Read-only accessors for tests and tools (additive)
    GameState GetState() const { return m_state; }
    int GetLevelIndex() const { return m_levelIndex; }
    const std::array<int, 3>& GetLevelScores() const { return m_levelScores; }
    const Spacecraft& GetCraft() const { return m_craft; }
    const MissionEvaluationReport& GetLastReport() const { return m_lastReport; }
    const ComponentDatabase& GetDatabase() const { return m_db; }
private:
    // ORDER MATTERS: m_db must be declared before m_craft, because the
    // spacecraft stores raw pointers into m_db.
    ComponentDatabase m_db;
    std::array<MissionLevel, 3> m_levels;
    Spacecraft m_craft;
    Viewport3D m_viewport;
    LaunchDirector m_launch;
    UIComponentCatalog m_catalog;
    UITelemetryHUD m_telemetry;
    UIPlanetReport m_planetReport;
    UIScreens m_screens;
    GameState m_state = GameState::MAIN_MENU;
    int m_levelIndex = 0;
    MissionEvaluationReport m_lastReport;
    std::array<int, 3> m_levelScores {0, 0, 0};
    bool m_assetsLoaded = false;
};
