// Owner: E4. Tasks E4-2..E4-6: headless UI smoke tests. Input is injected through the stub, Draw runs
// against the no-op stub, and we assert on stored state plus the text that Draw emitted.
#include <array>
#include "AssetManager.h"
#include "ComponentDatabase.h"
#include "MissionEvaluator.h"
#include "Spacecraft.h"
#include "UIComponentCatalog.h"
#include "UIPlanetReport.h"
#include "UIScreens.h"
#include "UITelemetryHUD.h"
#include "stub_hooks.h"
#include "test_util.h"

namespace {
Vector2 Center(Rectangle r) { return {r.x + r.width / 2, r.y + r.height / 2}; }
void Click(Rectangle r) { Vector2 c = Center(r); StubClick(c.x, c.y); }
void Next() { StubResetFrame(); GetStubInput().down = false; }
bool Overlap(Rectangle a, Rectangle b) {
    return a.x < b.x + b.width && b.x < a.x + a.width && a.y < b.y + b.height && b.y < a.y + a.height;
}
bool Inside(Rectangle inner, Rectangle outer) {
    return inner.x >= outer.x && inner.y >= outer.y && inner.x + inner.width <= outer.x + outer.width &&
           inner.y + inner.height <= outer.y + outer.height;
}
}  // namespace

int main() {
    ComponentDatabase db; db.LoadFromJson("data/parts_catalog.json");
    auto levels = MissionLevel::LoadAllFromJson("data/levels_config.json");
    std::vector<std::string> mk{db.GetBaseChassisModelKey()}, ik;
    for (auto* p : db.GetAllParts()) { mk.push_back(p->GetModelKey()); ik.push_back(p->GetIconKey()); }
    AssetManager::GetInstance().LoadAllAssets(mk, ik);
    Spacecraft craft; craft.Initialize(db.GetBaseChassisStats());

    RunTest("E4-2 catalog: default tab shows all 15, five cards, hover sets ghost", [&] {
        UIComponentCatalog cat; StubResetFrame();
        StubMove(0, 0);
        cat.Update(craft, db, false);
        CHECK(cat.GetSelectedTab() == 0 && cat.GetFilteredCount() == 15 && cat.GetScrollOffset() == 0);
        CHECK(craft.GetGhostComponent() == nullptr);
        StubMove(Center(UIComponentCatalog::CardRect(2)).x, Center(UIComponentCatalog::CardRect(2)).y);
        cat.Update(craft, db, false);
        CHECK(craft.GetGhostComponent() == db.GetAllParts()[2]);
        cat.Draw(craft);
        CHECK(StubTextDrawn("MODULAR PARTS") && StubTextDrawn("Light Orbital Skids") && StubTextDrawn("Cost: $400"));
        CHECK(StubTextDrawn("Build Time: 1m 00s") && StubTextDrawn("Showing 1-5 of 15"));
        StubMove(0, 0); cat.Update(craft, db, false);
        CHECK(craft.GetGhostComponent() == nullptr);                  // leaving the cards clears it
    });

    RunTest("E4-2 catalog: Buy equips, button flips to Remove, Remove unequips", [&] {
        UIComponentCatalog cat; craft.ResetToBareChassis(); StubResetFrame();
        Click(UIComponentCatalog::BuyRect(0)); cat.Update(craft, db, false);
        CHECK(craft.GetEquippedComponent(ComponentSlot::MOBILITY) == db.FindById("mobility_t1"));
        Next(); cat.Update(craft, db, false);   // still hovering card 0
        cat.Draw(craft);
        CHECK(StubTextDrawn("Remove"));
        Next(); Click(UIComponentCatalog::PlusRect(0)); cat.Update(craft, db, false);   // "-" button
        CHECK(craft.GetEquippedComponent(ComponentSlot::MOBILITY) == nullptr);
        Next(); Click(UIComponentCatalog::CardRect(0)); cat.Update(craft, db, false);   // card body, not a button
        CHECK(craft.GetEquippedComponent(ComponentSlot::MOBILITY) == nullptr);
        Next(); Click(UIComponentCatalog::BuyRect(1)); cat.Update(craft, db, false);
        Next(); Click(UIComponentCatalog::BuyRect(0)); cat.Update(craft, db, false);   // replaces same slot
        CHECK(craft.GetEquippedComponent(ComponentSlot::MOBILITY) == db.FindById("mobility_t1"));
        CHECK(craft.GetEquippedComponent(ComponentSlot::MOBILITY)->GetTier() == 1);
        Next();
    });

    RunTest("E4-2 catalog: tabs filter, wheel scrolls one card per notch and clamps, X resets", [&] {
        UIComponentCatalog cat; craft.ResetToBareChassis(); StubResetFrame();
        for (int t = 1; t <= 5; ++t) {
            Click(UIComponentCatalog::TabRect(t)); cat.Update(craft, db, false); Next();
            CHECK(cat.GetSelectedTab() == t && cat.GetFilteredCount() == 3);
        }
        Click(UIComponentCatalog::ResetButtonRect()); cat.Update(craft, db, false); Next();
        CHECK(cat.GetSelectedTab() == 0 && cat.GetFilteredCount() == 15);
        StubMove(Center(RECT_CATALOG_MAIN_PANEL).x, Center(RECT_CATALOG_MAIN_PANEL).y);
        GetStubInput().wheel = -1.0f; cat.Update(craft, db, false); Next();
        CHECK(cat.GetScrollOffset() == 1);
        for (int i = 0; i < 20; ++i) { StubMove(Center(RECT_CATALOG_MAIN_PANEL).x, Center(RECT_CATALOG_MAIN_PANEL).y); GetStubInput().wheel = -1.0f; cat.Update(craft, db, false); Next(); }
        CHECK(cat.GetScrollOffset() == 10);                              // 15 - 5
        cat.Draw(craft);
        CHECK(StubTextDrawn("Showing 11-15 of 15"));
        GetStubInput().wheel = 1.0f; StubMove(Center(RECT_CATALOG_MAIN_PANEL).x, Center(RECT_CATALOG_MAIN_PANEL).y);
        cat.Update(craft, db, false); Next();
        CHECK(cat.GetScrollOffset() == 9);
        Click(UIComponentCatalog::TabRect(2)); cat.Update(craft, db, false); Next();   // changing tab resets scroll
        CHECK(cat.GetScrollOffset() == 0);
        // wheel outside the panel does nothing
        StubMove(1000, 400); GetStubInput().wheel = -1.0f; cat.Update(craft, db, false); Next();
        CHECK(cat.GetScrollOffset() == 0);
    });

    RunTest("E4-2 catalog: locked = no hover, no buy, no scroll, no tab change", [&] {
        UIComponentCatalog cat; craft.ResetToBareChassis(); StubResetFrame();
        StubMove(Center(UIComponentCatalog::CardRect(1)).x, Center(UIComponentCatalog::CardRect(1)).y);
        cat.Update(craft, db, false);
        CHECK(craft.GetGhostComponent() != nullptr);              // a ghost exists when the launch locks the UI
        cat.Update(craft, db, true);
        CHECK(craft.GetGhostComponent() == nullptr);              // locking clears it even with the mouse still on a card
        Click(UIComponentCatalog::BuyRect(0)); cat.Update(craft, db, true);
        CHECK(craft.GetEquippedComponent(ComponentSlot::MOBILITY) == nullptr);
        CHECK(craft.GetGhostComponent() == nullptr);
        Next(); Click(UIComponentCatalog::TabRect(3)); cat.Update(craft, db, true);
        CHECK(cat.GetSelectedTab() == 0);
        cat.Draw(craft);
        CHECK(StubTextDrawn("LOCKED"));
        Next();
    });

    RunTest("E4-3 HUD: Level 1 reference shows 75 / 45 / 50.0 / $2,300 / 5m 15s, launch enabled", [&] {
        craft.ResetToBareChassis();
        for (const char* id : {"mobility_t1", "optics_t1", "payload_t1", "power_t1", "shield_t1"}) craft.EquipComponent(db.FindById(id));
        UITelemetryHUD hud; StubResetFrame(); StubMove(0, 0);
        CHECK(!hud.Update(craft, false));
        CHECK(hud.IsLaunchEnabled());
        hud.Draw(craft, levels[0], nullptr, false);
        CHECK(StubTextDrawn("75") && StubTextDrawn("45") && StubTextDrawn("50.0"));
        CHECK(StubTextDrawn("$2,300") && StubTextDrawn("5m 15s") && StubTextDrawn("LAUNCH MISSION"));
        Next(); Click(RECT_LAUNCH_BUTTON);
        CHECK(hud.Update(craft, false));                                  // click returns true once
        Next(); StubMove(Center(RECT_LAUNCH_BUTTON).x, Center(RECT_LAUNCH_BUTTON).y);
        CHECK(!hud.Update(craft, false));                                 // hover alone does not
        Next();
    });

    RunTest("E4-3 HUD: incomplete craft dims launch, shows tooltip, click does nothing (TC-6)", [&] {
        craft.UnequipSlot(ComponentSlot::PAYLOAD);
        UITelemetryHUD hud; StubResetFrame(); Click(RECT_LAUNCH_BUTTON);
        CHECK(!hud.Update(craft, false));
        CHECK(!hud.IsLaunchEnabled());
        hud.Draw(craft, levels[0], nullptr, false);
        CHECK(StubTextDrawn("INSTALL ALL 5 SUBSYSTEMS TO LAUNCH"));
        Next(); StubMove(100, 100); hud.Update(craft, false); hud.Draw(craft, levels[0], nullptr, false);
        CHECK(!StubTextDrawn("INSTALL ALL 5"));                           // no tooltip when not hovering
        // locked also blocks a complete craft
        craft.EquipComponent(db.FindById("payload_t1"));
        Next(); Click(RECT_LAUNCH_BUTTON); CHECK(!hud.Update(craft, true));
        Next();
    });

    RunTest("E4-3 HUD: LED segments = ceil(value/max*12), clamped", [&] {
        CHECK(UITelemetryHUD::LitSegments(0, 250) == 0);
        CHECK(UITelemetryHUD::LitSegments(1, 250) == 1);
        CHECK(UITelemetryHUD::LitSegments(250, 250) == 12);
        CHECK(UITelemetryHUD::LitSegments(300, 250) == 12);
        CHECK(UITelemetryHUD::LitSegments(125, 250) == 6);
        CHECK(UITelemetryHUD::LitSegments(75, 250) == 4);       // 3.6 -> 4
        CHECK(UITelemetryHUD::LitSegments(50, 150) == 4);       // exactly 4.0 -> 4 (not 5)
        CHECK(UITelemetryHUD::LitSegments(-5, 250) == 0);
    });

    RunTest("E4-3 HUD: ghost preview and animated launch health draw without error", [&] {
        UITelemetryHUD hud; StubResetFrame(); StubMove(0, 0);
        craft.SetGhostPreviewComponent(db.FindById("power_t3"));
        hud.Update(craft, false); hud.Draw(craft, levels[0], nullptr, false);
        craft.SetGhostPreviewComponent(nullptr);
        MissionEvaluationReport r = MissionEvaluator::Evaluate(craft, levels[2]);   // fails stage 3 -> shake/health drop
        LaunchDirector ld; ld.Start(r);
        Viewport3D vp; vp.Init();
        for (int i = 0; i < 28; ++i) ld.Update(0.25f, vp);
        hud.Draw(craft, levels[2], &ld, true);
        CHECK(StubTextDrawn("MISSION IN PROGRESS"));
        Next();
    });

    RunTest("E4-4 planet report: Mars/Phobos titles, six fields", [&] {
        UIPlanetReport pr; Viewport3D vp; vp.Init(); StubResetFrame(); StubMove(0, 0);
        pr.Update(vp, false);
        pr.Draw(levels[0].GetPlanetReport());
        CHECK(StubTextDrawn("MARS PLANET REPORT") && StubTextDrawn("Telemetry: Mock Orbital"));
        CHECK(StubTextDrawn("Data: -60.37 deg +39.85 deg") && StubTextDrawn("Range: 69.39 - 62.05"));
        CHECK(StubTextDrawn("Planetary") && StubTextDrawn("Surface Temp: -60 C") && StubTextDrawn("Atmosphere: Thin CO2"));
        CHECK(StubTextDrawn("360 deg"));
        Next(); pr.Draw(levels[1].GetPlanetReport());
        CHECK(StubTextDrawn("PHOBOS PLANET REPORT") && StubTextDrawn("Orbital Debris Ring") && StubTextDrawn("None (airless)"));
        Next();
    });

    RunTest("E4-4 planet report: rotate buttons hold, reset click, locked disables", [&] {
        UIPlanetReport pr; Viewport3D vp; vp.Init(); StubResetFrame();
        Click(UIPlanetReport::RotateRightRect()); GetStubInput().down = true;
        pr.Update(vp, false);
        CHECK(vp.GetOrbitAngleDeg() > 45.0f);                    // right adds
        float after = vp.GetOrbitAngleDeg();
        CHECK_NEAR(after, 45.0 + 90.0 / 60.0, 1e-3);             // 90 deg/s * (1/60 s)
        Next(); Click(UIPlanetReport::RotateLeftRect()); GetStubInput().down = true; pr.Update(vp, false);
        CHECK_NEAR(vp.GetOrbitAngleDeg(), 45.0, 1e-3);           // left subtracts
        Next(); GetStubInput().down = true; StubMove(Center(UIPlanetReport::RotateLeftRect()).x, Center(UIPlanetReport::RotateLeftRect()).y);
        pr.Update(vp, false);                                    // held, no new press: still rotates
        CHECK_NEAR(vp.GetOrbitAngleDeg(), 45.0 - 1.5, 1e-3);
        Next(); Click(UIPlanetReport::ResetRect()); pr.Update(vp, false);
        vp.Update(0.5f);
        CHECK_NEAR(vp.GetOrbitAngleDeg(), 45.0, 1e-3);
        Next(); vp.AddOrbitDegrees(30.0f); Click(UIPlanetReport::RotateRightRect()); GetStubInput().down = true;
        float before = vp.GetOrbitAngleDeg(); pr.Update(vp, true);
        CHECK_NEAR(vp.GetOrbitAngleDeg(), before, 1e-6);         // locked
        Next();
    });

    RunTest("E4-5 screens: buttons return the right actions and show the right text", [&] {
        UIScreens s; StubResetFrame(); StubMove(0, 0);
        CHECK(s.UpdateMainMenu() == ScreenAction::NONE);
        s.DrawMainMenu();
        CHECK(StubTextDrawn("NASA SPACECRAFT DESIGN SIMULATION") && StubTextDrawn("START MISSION 1"));
        Next(); Click(UIScreens::MenuButtonRect()); CHECK(s.UpdateMainMenu() == ScreenAction::START_MISSION);
        Next(); Click(UIScreens::MenuButtonRect()); GetStubInput().pressed = false;
        CHECK(s.UpdateMainMenu() == ScreenAction::NONE);          // hover without press
        Next(); Click(UIScreens::BriefingButtonRect());
        CHECK(s.UpdateBriefing(levels[0]) == ScreenAction::ENTER_HANGAR);
        Next(); s.DrawBriefing(levels[1]);
        CHECK(StubTextDrawn("Phobos Orbital Rock Capture") && StubTextDrawn("ENTER ASSEMBLY HANGAR") &&
              StubTextDrawn("LEVEL 2 OF 3") && StubTextDrawn("$6,200") && StubTextDrawn("14m 00s"));
        Next(); s.DrawBriefing(levels[2]);
        CHECK(StubTextDrawn("REQUIRED"));                         // surface landing gear on level 3
        Next();
    });

    RunTest("E4-5 debrief: success / failure / level-3 labels and actions", [&] {
        UIScreens s;
        MissionEvaluationReport ok; ok.overallMissionSuccess = true; ok.finalEfficiencyScore = 65;
        MissionEvaluationReport bad;
        for (int i = 0; i < 4; ++i) { ok.stages[i].passed = true; ok.stages[i].stageName = "S"; bad.stages[i].passed = (i != 1); bad.stages[i].stageName = "S"; }
        bad.stages[1].failureReason = "Insufficient thrust-to-weight ratio! Craft failed to achieve target trajectory.";
        CHECK_EQ_STR(UIScreens::DebriefButtonLabel(ok, 0), "PROCEED TO NEXT MISSION");
        CHECK_EQ_STR(UIScreens::DebriefButtonLabel(ok, 1), "PROCEED TO NEXT MISSION");
        CHECK_EQ_STR(UIScreens::DebriefButtonLabel(ok, 2), "VIEW CAMPAIGN RESULTS");
        CHECK_EQ_STR(UIScreens::DebriefButtonLabel(bad, 0), "RETURN TO HANGAR & REDESIGN");
        StubResetFrame(); s.DrawDebrief(ok, 0);
        CHECK(StubTextDrawn("MISSION SUCCESS") && StubTextDrawn("EFFICIENCY SCORE: 65") && StubTextDrawn("PASS"));
        Next(); s.DrawDebrief(bad, 0);
        CHECK(StubTextDrawn("MISSION FAILED") && StubTextDrawn("FAIL") && StubTextDrawn("Insufficient thrust"));
        CHECK(!StubTextDrawn("EFFICIENCY SCORE"));
        Next(); Click(UIScreens::DebriefButtonRect());
        CHECK(s.UpdateDebrief(ok, 0) == ScreenAction::PROCEED);
        Next(); Click(UIScreens::DebriefButtonRect());
        CHECK(s.UpdateDebrief(bad, 0) == ScreenAction::RETURN_TO_HANGAR);
        Next();
    });

    RunTest("E4-5 complete screen and launch banner", [&] {
        UIScreens s; StubResetFrame(); s.DrawComplete({65, 57, 54});
        CHECK(StubTextDrawn("MISSION CAMPAIGN COMPLETE") && StubTextDrawn("65") && StubTextDrawn("57") &&
              StubTextDrawn("54") && StubTextDrawn("RETURN TO MAIN MENU") && StubTextDrawn("176"));
        Next(); Click(UIScreens::CompleteButtonRect()); CHECK(s.UpdateComplete() == ScreenAction::RETURN_TO_MENU);
        Next();
        LaunchDirector ld; Viewport3D vp; vp.Init();
        s.DrawLaunchBanner(ld); CHECK(StubTextDrawn("Stage") == false);   // nothing before Start
        MissionEvaluationReport r;
        for (int i = 0; i < 4; ++i) { r.stages[i].passed = true; r.stages[i].stageName = "N" + std::to_string(i + 1); }
        ld.Start(r); ld.Update(3.25f, vp);
        s.DrawLaunchBanner(ld);
        CHECK(StubTextDrawn("Stage 2 of 4: N2") && StubTextDrawn("PASS"));
        Next();
    });

    RunTest("E4-6 layout: panels do not overlap and everything sits on screen", [&] {
        Rectangle screen{0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
        std::array<Rectangle, 8> all = {RECT_CATALOG_SIDEBAR_TABS, RECT_CATALOG_MAIN_PANEL, RECT_VIEWPORT_3D_AREA,
            RECT_ROTATION_CONTROLS, RECT_PLANET_REPORT_PANEL, RECT_TELEMETRY_TOP_PANEL, RECT_STATS_BOTTOM_PANEL, RECT_LAUNCH_BUTTON};
        for (auto r : all) CHECK(Inside(r, screen));
        for (size_t i = 0; i < all.size(); ++i)
            for (size_t j = i + 1; j < all.size(); ++j) {
                bool allowed = (all[i].x == RECT_VIEWPORT_3D_AREA.x && all[j].x == RECT_ROTATION_CONTROLS.x) ;  // rotate overlay sits on the viewport
                if (!allowed) CHECK(!Overlap(all[i], all[j]));
            }
        CHECK(Inside(RECT_ROTATION_CONTROLS, RECT_VIEWPORT_3D_AREA));
        for (int t = 0; t < 6; ++t) CHECK(Inside(UIComponentCatalog::TabRect(t), RECT_CATALOG_SIDEBAR_TABS));
        for (int t = 0; t < 5; ++t) {
            CHECK(Inside(UIComponentCatalog::CardRect(t), RECT_CATALOG_MAIN_PANEL));
            CHECK(Inside(UIComponentCatalog::BuyRect(t), UIComponentCatalog::CardRect(t)));
            CHECK(Inside(UIComponentCatalog::PlusRect(t), UIComponentCatalog::CardRect(t)));
            CHECK(!Overlap(UIComponentCatalog::BuyRect(t), UIComponentCatalog::PlusRect(t)));
            if (t) CHECK(!Overlap(UIComponentCatalog::CardRect(t - 1), UIComponentCatalog::CardRect(t)));
        }
        CHECK(UIComponentCatalog::TabRect(0).width == 48 && UIComponentCatalog::TabRect(0).height == 64);
        CHECK(UIComponentCatalog::CardRect(0).width == 360 && UIComponentCatalog::CardRect(0).height == 145);
        CHECK(Inside(UIPlanetReport::RotateLeftRect(), RECT_ROTATION_CONTROLS));
        CHECK(Inside(UIPlanetReport::RotateRightRect(), RECT_ROTATION_CONTROLS));
        CHECK(Inside(UIPlanetReport::ResetRect(), RECT_ROTATION_CONTROLS));
        CHECK(!Overlap(UIPlanetReport::RotateLeftRect(), UIPlanetReport::ResetRect()));
        CHECK(!Overlap(UIPlanetReport::RotateRightRect(), UIPlanetReport::ResetRect()));
    });

    AssetManager::GetInstance().UnloadAllAssets();
    return Finish("test_ui_smoke");
}
