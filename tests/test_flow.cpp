// Owner: E2. Integration tests TC-6..TC-10 and the campaign flow (TC-8), driven through the real GameManager
// and the real UI with injected mouse clicks (headless raylib stand-in).
#include <string>
#include "GameManager.h"
#include "stub_hooks.h"
#include "test_util.h"

namespace {
Vector2 Center(Rectangle r) { return {r.x + r.width / 2, r.y + r.height / 2}; }

// One simulated frame: optional click, update, then clear per-frame input.
void Frame(GameManager& g, float dt = 0.25f) { g.Update(dt); StubResetFrame(); GetStubInput().down = false; }
void ClickFrame(GameManager& g, Rectangle r) { Vector2 c = Center(r); StubClick(c.x, c.y); Frame(g); }

void EquipViaUi(GameManager& g, const char* id) {
    const ShipComponent* p = g.GetDatabase().FindById(id);
    ClickFrame(g, UIComponentCatalog::TabRect(static_cast<int>(p->GetSlotType()) + 1));
    ClickFrame(g, UIComponentCatalog::BuyRect(p->GetTier() - 1));
}
void EquipBuild(GameManager& g, std::initializer_list<const char*> ids) { for (const char* id : ids) EquipViaUi(g, id); StubMove(0, 0); Frame(g); }

const std::initializer_list<const char*> L1 = {"mobility_t1", "optics_t1", "payload_t1", "power_t1", "shield_t1"};
const std::initializer_list<const char*> L2 = {"mobility_t2", "optics_t2", "payload_t2", "power_t2", "shield_t2"};
const std::initializer_list<const char*> L3 = {"mobility_t3", "optics_t2", "payload_t3", "power_t3", "shield_t3"};
const std::initializer_list<const char*> T3 = {"mobility_t3", "optics_t3", "payload_t3", "power_t3", "shield_t3"};

// Launch and run the 0.25 s ticks until the debrief opens. Returns the elapsed launch seconds.
float LaunchAndWait(GameManager& g) {
    ClickFrame(g, RECT_LAUNCH_BUTTON);
    if (g.GetState() != GameState::LAUNCH_EVALUATION) return -1.0f;
    float t = 0;
    while (g.GetState() == GameState::LAUNCH_EVALUATION && t < 30.0f) { Frame(g); t += 0.25f; }
    return t;
}
void ToHangar(GameManager& g) {
    ClickFrame(g, UIScreens::MenuButtonRect());
    ClickFrame(g, UIScreens::BriefingButtonRect());
}
void Init(GameManager& g) { g.Init(); StubResetFrame(); }
}  // namespace

int main() {
    RunTest("startup: menu state, 3 levels, empty craft", [] {
        GameManager g; Init(g);
        CHECK(g.GetState() == GameState::MAIN_MENU && g.GetLevelIndex() == 0);
        CHECK(!g.GetCraft().AreAllMandatorySlotsFilled());
        g.Draw();   // every state must draw without throwing
        g.Shutdown();
    });

    RunTest("Init / Shutdown / Init again is safe", [] {
        GameManager g;
        Init(g); g.Shutdown(); Init(g);
        CHECK(g.GetState() == GameState::MAIN_MENU);
        g.Shutdown();
    });

    RunTest("TC-6 one slot empty: launch dimmed, tooltip, click does nothing", [] {
        GameManager g; Init(g); ToHangar(g);
        CHECK(g.GetState() == GameState::HANGAR_ASSEMBLY);
        EquipBuild(g, {"mobility_t1", "optics_t1", "payload_t1", "power_t1"});   // no shielding
        StubMove(Center(RECT_LAUNCH_BUTTON).x, Center(RECT_LAUNCH_BUTTON).y);
        g.Update(0.016f); g.Draw();
        CHECK(StubTextDrawn("INSTALL ALL 5 SUBSYSTEMS TO LAUNCH"));
        StubResetFrame();
        ClickFrame(g, RECT_LAUNCH_BUTTON);
        CHECK(g.GetState() == GameState::HANGAR_ASSEMBLY);
        g.Shutdown();
    });

    RunTest("TC-1 + TC-8 campaign: three wins, scores 65/57/54, menu restart is clean", [] {
        GameManager g; Init(g);
        ClickFrame(g, UIScreens::MenuButtonRect());
        CHECK(g.GetState() == GameState::LEVEL_BRIEFING);
        ClickFrame(g, UIScreens::BriefingButtonRect());
        CHECK(g.GetState() == GameState::HANGAR_ASSEMBLY);
        EquipBuild(g, L1);
        CHECK(g.GetCraft().AreAllMandatorySlotsFilled());
        float t = LaunchAndWait(g);
        CHECK_NEAR(t, 12.0, 0.01);
        CHECK(g.GetState() == GameState::MISSION_DEBRIEF && g.GetLastReport().overallMissionSuccess);
        CHECK(g.GetLevelScores()[0] == 65);
        g.Draw();
        CHECK(StubTextDrawn("PROCEED TO NEXT MISSION"));
        ClickFrame(g, UIScreens::DebriefButtonRect());
        CHECK(g.GetState() == GameState::LEVEL_BRIEFING && g.GetLevelIndex() == 1);
        CHECK(!g.GetCraft().GetEquippedComponent(ComponentSlot::POWER));     // craft reset between levels
        ClickFrame(g, UIScreens::BriefingButtonRect());
        EquipBuild(g, L2);
        CHECK_NEAR(LaunchAndWait(g), 12.0, 0.01);
        CHECK(g.GetLevelScores()[1] == 57);
        ClickFrame(g, UIScreens::DebriefButtonRect());
        ClickFrame(g, UIScreens::BriefingButtonRect());
        EquipBuild(g, L3);
        CHECK_NEAR(LaunchAndWait(g), 12.0, 0.01);
        CHECK(g.GetLevelScores()[2] == 54);
        g.Draw();
        CHECK(StubTextDrawn("VIEW CAMPAIGN RESULTS") && !StubTextDrawn("PROCEED"));   // no PROCEED on level 3 (FIX 10)
        ClickFrame(g, UIScreens::DebriefButtonRect());
        CHECK(g.GetState() == GameState::GAME_COMPLETE);
        g.Draw();
        CHECK(StubTextDrawn("MISSION CAMPAIGN COMPLETE") && StubTextDrawn("65") && StubTextDrawn("57") && StubTextDrawn("54"));
        ClickFrame(g, UIScreens::CompleteButtonRect());
        CHECK(g.GetState() == GameState::MAIN_MENU && g.GetLevelIndex() == 0);
        CHECK(!g.GetCraft().GetEquippedComponent(ComponentSlot::MOBILITY));
        ClickFrame(g, UIScreens::MenuButtonRect());
        CHECK(g.GetState() == GameState::LEVEL_BRIEFING && g.GetLevelIndex() == 0);
        CHECK(g.GetLevelScores()[0] == 0 && g.GetLevelScores()[1] == 0 && g.GetLevelScores()[2] == 0);   // fresh run
        g.Shutdown();
    });

    RunTest("TC-7 + TC-9 Level 1 all Tier 3 fails at 3.0 s, redesign keeps parts, then wins with 65", [] {
        GameManager g; Init(g); ToHangar(g);
        EquipBuild(g, T3);
        float t = LaunchAndWait(g);
        CHECK_NEAR(t, 3.0, 0.01);
        CHECK(g.GetState() == GameState::MISSION_DEBRIEF && !g.GetLastReport().overallMissionSuccess);
        CHECK_EQ_STR(g.GetLastReport().stages[0].failureReason, "Budget exceeded! NASA funding cap violated.");
        CHECK(g.GetLevelScores()[0] == 0);
        g.Draw();
        CHECK(StubTextDrawn("RETURN TO HANGAR & REDESIGN") && StubTextDrawn("MISSION FAILED"));
        ClickFrame(g, UIScreens::DebriefButtonRect());
        CHECK(g.GetState() == GameState::HANGAR_ASSEMBLY && g.GetLevelIndex() == 0);
        CHECK(g.GetCraft().AreAllMandatorySlotsFilled());                       // all T3 parts still equipped
        CHECK(g.GetCraft().GetEquippedComponent(ComponentSlot::POWER)->GetTier() == 3);
        EquipBuild(g, L1);                                                       // swap every slot to Tier 1
        CHECK_NEAR(LaunchAndWait(g), 12.0, 0.01);
        CHECK(g.GetLastReport().overallMissionSuccess && g.GetLevelScores()[0] == 65);
        g.Shutdown();
    });

    RunTest("TC-4 Level 3 all Tier 3 fails on thrust at 6.0 s", [] {
        GameManager g; Init(g); ToHangar(g);
        // reach level 3 the honest way: win levels 1 and 2 first
        EquipBuild(g, L1); LaunchAndWait(g); ClickFrame(g, UIScreens::DebriefButtonRect()); ClickFrame(g, UIScreens::BriefingButtonRect());
        EquipBuild(g, L2); LaunchAndWait(g); ClickFrame(g, UIScreens::DebriefButtonRect()); ClickFrame(g, UIScreens::BriefingButtonRect());
        CHECK(g.GetLevelIndex() == 2);
        EquipBuild(g, T3);
        CHECK_NEAR(g.GetCraft().GetCurrentStats().thrust, 80.8, 0.1);
        float t = LaunchAndWait(g);
        CHECK_NEAR(t, 6.0, 0.01);
        const auto& r = g.GetLastReport();
        CHECK(r.stages[0].passed && !r.stages[1].passed && r.stages[2].passed && r.stages[3].passed);
        CHECK(g.GetLevelScores()[2] == 0);
        g.Shutdown();
    });

    RunTest("TC-5 Level 3 with optics rating 1: fails stage 4, debrief at 12.0 s", [] {
        GameManager g; Init(g); ToHangar(g);
        EquipBuild(g, L1); LaunchAndWait(g); ClickFrame(g, UIScreens::DebriefButtonRect()); ClickFrame(g, UIScreens::BriefingButtonRect());
        EquipBuild(g, L2); LaunchAndWait(g); ClickFrame(g, UIScreens::DebriefButtonRect()); ClickFrame(g, UIScreens::BriefingButtonRect());
        EquipBuild(g, {"mobility_t3", "optics_t1", "payload_t3", "power_t3", "shield_t3"});
        CHECK_NEAR(LaunchAndWait(g), 12.0, 0.01);
        const auto& r = g.GetLastReport();
        CHECK(r.stages[0].passed && r.stages[1].passed && r.stages[2].passed && !r.stages[3].passed);
        CHECK_EQ_STR(r.stages[3].failureReason, "Incompatible scientific payload equipped for this mission objective!");
        g.Shutdown();
    });

    RunTest("TC-10 Level 2 with the Level 1 build fails stage 2 at 6.0 s (thrust message)", [] {
        GameManager g; Init(g); ToHangar(g);
        EquipBuild(g, L1); LaunchAndWait(g); ClickFrame(g, UIScreens::DebriefButtonRect()); ClickFrame(g, UIScreens::BriefingButtonRect());
        EquipBuild(g, L1);
        CHECK_NEAR(LaunchAndWait(g), 6.0, 0.01);
        CHECK_EQ_STR(g.GetLastReport().stages[1].failureReason,
                     "Insufficient thrust-to-weight ratio! Craft failed to achieve target trajectory.");
        g.Shutdown();
    });

    RunTest("catalog is locked during launch: clicks cannot change the craft", [] {
        GameManager g; Init(g); ToHangar(g);
        EquipBuild(g, L1);
        ClickFrame(g, RECT_LAUNCH_BUTTON);
        CHECK(g.GetState() == GameState::LAUNCH_EVALUATION);
        ClickFrame(g, UIComponentCatalog::TabRect(4));
        ClickFrame(g, UIComponentCatalog::BuyRect(2));      // would equip power_t3 if unlocked
        CHECK(g.GetCraft().GetEquippedComponent(ComponentSlot::POWER)->GetTier() == 1);
        CHECK(g.GetCraft().GetGhostComponent() == nullptr);
        g.Draw();
        CHECK(StubTextDrawn("Stage 1 of 4: Budget & Launch Window"));
        g.Shutdown();
    });

    return Finish("test_flow");
}
