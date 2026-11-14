// Owner: E2
#include "GameManager.h"
#include <vector>
#include "AssetManager.h"

void GameManager::Init() {
    m_db.LoadFromJson("data/parts_catalog.json");

    std::vector<std::string> modelKeys{m_db.GetBaseChassisModelKey()};
    std::vector<std::string> iconKeys;
    for (const ShipComponent* p : m_db.GetAllParts()) {
        modelKeys.push_back(p->GetModelKey());
        iconKeys.push_back(p->GetIconKey());
    }
    AssetManager::GetInstance().LoadAllAssets(modelKeys, iconKeys);
    m_assetsLoaded = true;

    m_levels = MissionLevel::LoadAllFromJson("data/levels_config.json");
    m_craft.Initialize(m_db.GetBaseChassisStats());
    m_viewport.Init();
    m_state = GameState::MAIN_MENU;
    m_levelIndex = 0;
    m_levelScores = {0, 0, 0};
}

void GameManager::Update(float dt) {
    const MissionLevel& level = m_levels[static_cast<size_t>(m_levelIndex)];
    switch (m_state) {
        case GameState::MAIN_MENU:
            if (m_screens.UpdateMainMenu() == ScreenAction::START_MISSION) {
                m_craft.ResetToBareChassis();
                m_levelIndex = 0;
                m_levelScores = {0, 0, 0};
                m_state = GameState::LEVEL_BRIEFING;
            }
            break;

        case GameState::LEVEL_BRIEFING:
            if (m_screens.UpdateBriefing(level) == ScreenAction::ENTER_HANGAR) {
                m_viewport.StartResetToHomeAngle();
                m_state = GameState::HANGAR_ASSEMBLY;
            }
            break;

        case GameState::HANGAR_ASSEMBLY: {
            m_viewport.Update(dt);
            m_catalog.Update(m_craft, m_db, false);
            m_planetReport.Update(m_viewport, false);
            if (m_telemetry.Update(m_craft, false)) {
                m_craft.SetGhostPreviewComponent(nullptr);
                MissionEvaluationReport report = MissionEvaluator::Evaluate(m_craft, level);
                m_launch.Start(report);
                m_state = GameState::LAUNCH_EVALUATION;
            }
            break;
        }

        case GameState::LAUNCH_EVALUATION:
            m_viewport.Update(dt);
            m_catalog.Update(m_craft, m_db, true);          // locked: clears the ghost, no buying
            m_planetReport.Update(m_viewport, true);
            m_telemetry.Update(m_craft, true);
            m_launch.Update(dt, m_viewport);
            if (m_launch.IsFinished()) {
                m_lastReport = m_launch.GetReport();
                if (m_lastReport.overallMissionSuccess)
                    m_levelScores[static_cast<size_t>(m_levelIndex)] = m_lastReport.finalEfficiencyScore;
                m_state = GameState::MISSION_DEBRIEF;
            }
            break;

        case GameState::MISSION_DEBRIEF:
            switch (m_screens.UpdateDebrief(m_lastReport, m_levelIndex)) {
                case ScreenAction::PROCEED:
                    if (m_levelIndex >= 2) {
                        m_state = GameState::GAME_COMPLETE;
                    } else {
                        ++m_levelIndex;
                        m_craft.ResetToBareChassis();
                        m_state = GameState::LEVEL_BRIEFING;
                    }
                    break;
                case ScreenAction::RETURN_TO_HANGAR:
                    m_state = GameState::HANGAR_ASSEMBLY;    // parts stay equipped
                    break;
                default: break;
            }
            break;

        case GameState::GAME_COMPLETE:
            if (m_screens.UpdateComplete() == ScreenAction::RETURN_TO_MENU) {
                m_levelIndex = 0;
                m_craft.ResetToBareChassis();
                m_state = GameState::MAIN_MENU;
            }
            break;
    }
}

void GameManager::Draw() const {
    const MissionLevel& level = m_levels[static_cast<size_t>(m_levelIndex)];
    switch (m_state) {
        case GameState::MAIN_MENU: m_screens.DrawMainMenu(); break;
        case GameState::LEVEL_BRIEFING: m_screens.DrawBriefing(level); break;
        case GameState::HANGAR_ASSEMBLY:
            m_viewport.Draw(m_craft);
            m_catalog.Draw(m_craft);
            m_telemetry.Draw(m_craft, level, nullptr, false);
            m_planetReport.Draw(level.GetPlanetReport());
            break;
        case GameState::LAUNCH_EVALUATION:
            m_viewport.Draw(m_craft);
            m_catalog.Draw(m_craft);
            m_telemetry.Draw(m_craft, level, &m_launch, true);
            m_planetReport.Draw(level.GetPlanetReport());
            m_screens.DrawLaunchBanner(m_launch);
            break;
        case GameState::MISSION_DEBRIEF: m_screens.DrawDebrief(m_lastReport, m_levelIndex); break;
        case GameState::GAME_COMPLETE: m_screens.DrawComplete(m_levelScores); break;
    }
}

void GameManager::Shutdown() {
    if (m_assetsLoaded) AssetManager::GetInstance().UnloadAllAssets();
    m_assetsLoaded = false;
}
