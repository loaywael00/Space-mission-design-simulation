// Owner: E3. Tasks E3-1, E3-3, E3-4 against the headless raylib stand-in (no GPU): asset loading,
// camera maths, orbit wrap, reset easing, and the exact two-pass draw-call structure of v2 FIX 9.
#include <cmath>
#include <fstream>
#include "AssetManager.h"
#include "ComponentDatabase.h"
#include "Spacecraft.h"
#include "Viewport3D.h"
#include "stub_hooks.h"
#include "test_util.h"

int main() {
    ComponentDatabase db;
    db.LoadFromJson("data/parts_catalog.json");
    auto keys = [&] {
        std::vector<std::string> k{db.GetBaseChassisModelKey()};
        for (auto* p : db.GetAllParts()) k.push_back(p->GetModelKey());
        return k;
    }();
    std::vector<std::string> icons;
    for (auto* p : db.GetAllParts()) icons.push_back(p->GetIconKey());

    RunTest("E3-1 AssetManager loads all 16 models + 15 icons + font, unloads clean", [&] {
        AssetManager& am = AssetManager::GetInstance();
        am.LoadAllAssets(keys, icons);
        CHECK(am.GetModel("base_chassis").meshCount > 0);
        CHECK(am.GetIcon("power_t3_rtg_nuclear_generator").id != 0);
        CHECK(am.GetUIFont().baseSize == 24);
        CHECK_THROWS(am.GetModel("not_a_key"));
        CHECK_THROWS(am.GetIcon("not_a_key"));
        am.UnloadAllAssets();
        CHECK_THROWS(am.GetModel("base_chassis"));
        CHECK_THROWS(am.GetUIFont());
        am.LoadAllAssets(keys, icons);        // reload after unload works
        CHECK(am.GetModel("base_chassis").meshCount > 0);
    });

    RunTest("E3-1 AssetManager throws naming the missing file", [&] {
        AssetManager& am = AssetManager::GetInstance();
        std::string msg;
        try { am.LoadAllAssets({"base_chassis", "ghost_part"}, {}); } catch (const std::exception& e) { msg = e.what(); }
        CHECK(msg.find("assets/models/ghost_part.obj") != std::string::npos);
        msg.clear();
        try { am.LoadAllAssets({"base_chassis"}, {"no_icon"}); } catch (const std::exception& e) { msg = e.what(); }
        CHECK(msg.find("assets/icons/no_icon.png") != std::string::npos);
        CHECK_THROWS(am.GetModel("base_chassis"));      // failed load leaves nothing half-loaded
        am.LoadAllAssets(keys, icons);
    });

    RunTest("E3-3 camera: starts at 45 deg, wraps through 0 and 360", [&] {
        Viewport3D v; v.Init();
        CHECK_NEAR(v.GetOrbitAngleDeg(), 45.0, 1e-5);
        v.AddOrbitDegrees(-50.0f);
        CHECK_NEAR(v.GetOrbitAngleDeg(), 355.0, 1e-4);
        v.AddOrbitDegrees(10.0f);
        CHECK_NEAR(v.GetOrbitAngleDeg(), 5.0, 1e-4);
        v.AddOrbitDegrees(720.0f + 355.0f);
        CHECK_NEAR(v.GetOrbitAngleDeg(), 0.0, 1e-3);
        CHECK_NEAR(Viewport3D::WrapDegrees(-0.0f), 0.0, 1e-6);
        CHECK(Viewport3D::WrapDegrees(-1e-9f) < 360.0f && Viewport3D::WrapDegrees(-1e-9f) >= 0.0f);
        CHECK_NEAR(Viewport3D::WrapDegrees(-90.0f), 270.0, 1e-4);
    });

    RunTest("E3-3 keyboard orbit: A subtracts, D adds 90 deg/s", [&] {
        Viewport3D v; v.Init(); StubResetFrame();
        GetStubInput().keys[68] = true;   // D
        v.Update(0.5f);
        CHECK_NEAR(v.GetOrbitAngleDeg(), 90.0, 1e-3);
        GetStubInput().keys[68] = false; GetStubInput().keys[65] = true;   // A
        v.Update(1.0f);
        CHECK_NEAR(v.GetOrbitAngleDeg(), 0.0, 1e-3);
        GetStubInput().keys[65] = false;
    });

    RunTest("E3-3 mouse drag: 0.4 deg per pixel, only inside the viewport", [&] {
        Viewport3D v; v.Init(); StubResetFrame();
        StubClick(900, 300);                      // press inside the viewport (not on rotate controls)
        GetStubInput().delta = {50, 0};
        v.Update(1.0f / 60);
        CHECK_NEAR(v.GetOrbitAngleDeg(), 45.0 + 20.0, 1e-3);
        StubResetFrame(); GetStubInput().down = false; v.Update(1.0f / 60);   // release
        StubClick(100, 300);                      // press OUTSIDE (over the catalog) -> no drag
        GetStubInput().delta = {50, 0};
        v.Update(1.0f / 60);
        CHECK_NEAR(v.GetOrbitAngleDeg(), 65.0, 1e-3);
        StubResetFrame(); GetStubInput().down = false;
        StubClick(900, 760);                      // press on the on-screen rotate controls -> no drag
        GetStubInput().delta = {50, 0};
        v.Update(1.0f / 60);
        CHECK_NEAR(v.GetOrbitAngleDeg(), 65.0, 1e-3);
        StubResetFrame(); GetStubInput().down = false; v.Update(1.0f / 60);
    });

    RunTest("E3-3 reset eases to 45 deg over 0.5 s by the shortest way", [&] {
        Viewport3D v; v.Init(); StubResetFrame();
        v.AddOrbitDegrees(-40.0f);                // 5 deg: shortest way to 45 is +40, not -320
        v.StartResetToHomeAngle();
        v.Update(0.25f);
        float mid = v.GetOrbitAngleDeg();
        CHECK(mid > 5.0f && mid < 45.0f);
        CHECK_NEAR(mid, 25.0, 1.0);               // smoothstep midpoint
        v.Update(0.25f);
        CHECK_NEAR(v.GetOrbitAngleDeg(), 45.0, 1e-3);
        v.AddOrbitDegrees(-170.0f);               // 235 deg: shortest way is -190 -> +170 (via 360)
        v.StartResetToHomeAngle();
        v.Update(0.25f);
        v.Update(0.25f);
        CHECK_NEAR(v.GetOrbitAngleDeg(), 45.0, 1e-3);
    });

    RunTest("E3-3 auto-rotate runs at the set rate and stops at 0", [&] {
        Viewport3D v; v.Init(); StubResetFrame();
        v.SetAutoRotateDegPerSec(25.0f);
        v.Update(1.0f);
        CHECK_NEAR(v.GetOrbitAngleDeg(), 70.0, 1e-3);
        v.SetAutoRotateDegPerSec(0.0f);
        v.Update(1.0f);
        CHECK_NEAR(v.GetOrbitAngleDeg(), 70.0, 1e-3);
    });

    RunTest("E3-3 zoom is clamped to [6, 16] and ignored outside the viewport", [&] {
        Viewport3D v; v.Init(); StubResetFrame();
        CHECK_NEAR(v.GetOrthoFovY(), 10.0, 1e-6);
        GetStubInput().mouse = {900, 300};
        GetStubInput().wheel = 100.0f; v.Update(0.016f);
        CHECK_NEAR(v.GetOrthoFovY(), 6.0, 1e-6);
        GetStubInput().wheel = -100.0f; v.Update(0.016f);
        CHECK_NEAR(v.GetOrthoFovY(), 16.0, 1e-6);
        GetStubInput().mouse = {100, 300}; GetStubInput().wheel = 4.0f; v.Update(0.016f);   // over the catalog
        CHECK_NEAR(v.GetOrthoFovY(), 16.0, 1e-6);
        StubResetFrame();
    });

    RunTest("E3-4 two-pass render structure (v2 FIX 9)", [&] {
        Spacecraft craft; craft.Initialize(db.GetBaseChassisStats());
        Viewport3D v; v.Init(); StubResetFrame();
        v.Draw(craft);                            // base chassis only
        CHECK(GetStubInput().modelDraws == 1 && GetStubInput().wireDraws == 1);
        CHECK(GetStubInput().depthMaskOff == 1 && GetStubInput().depthMaskOn == 1);   // wires never write depth
        for (const char* id : {"mobility_t1", "optics_t1", "payload_t1", "power_t1", "shield_t1"})
            craft.EquipComponent(db.FindById(id));
        StubResetFrame(); v.Draw(craft);          // chassis + 5 parts, each fill + wire
        CHECK(GetStubInput().modelDraws == 6 && GetStubInput().wireDraws == 6);
        CHECK(GetStubInput().depthMaskOff == GetStubInput().depthMaskOn);             // always balanced
        // ghost over an equipped slot REPLACES that part: 5 chassis+parts passes - 1 skipped + ghost (fill+wire)
        craft.SetGhostPreviewComponent(db.FindById("power_t3"));
        StubResetFrame(); v.Draw(craft);
        CHECK(GetStubInput().modelDraws == 5 + 1 && GetStubInput().wireDraws == 5 + 1);
        CHECK(GetStubInput().depthMaskOff == GetStubInput().depthMaskOn);
        // ghost in an EMPTY slot just adds
        craft.UnequipSlot(ComponentSlot::OPTICS);
        craft.SetGhostPreviewComponent(db.FindById("optics_t2"));
        StubResetFrame(); v.Draw(craft);
        CHECK(GetStubInput().modelDraws == 5 + 1 && GetStubInput().wireDraws == 5 + 1);
    });

    RunTest("E3-4 all 15 parts draw when equipped (TC-V1 draw-call level)", [&] {
        Viewport3D v; v.Init();
        for (ShipComponent* p : db.GetAllParts()) {
            Spacecraft craft; craft.Initialize(db.GetBaseChassisStats());
            craft.EquipComponent(p);
            StubResetFrame(); v.Draw(craft);
            CHECK(GetStubInput().modelDraws == 2 && GetStubInput().wireDraws == 2);
        }
    });

    RunTest("E3-4 shake settles back to the home camera", [&] {
        Viewport3D v; v.Init(); StubResetFrame();
        v.SetChassisColor(COLOR_STATUS_WARNING);
        v.SetSlotOverride(ComponentSlot::POWER, COLOR_STATUS_NOMINAL);
        v.TriggerShake(0.5f, 0.1f);
        for (int i = 0; i < 40; ++i) v.Update(1.0f / 60);   // 0.67 s > 0.5 s shake
        v.ClearSlotOverrides();
        CHECK_NEAR(v.GetOrbitAngleDeg(), 45.0, 1e-5);       // shake never moves the orbit angle
    });

    AssetManager::GetInstance().UnloadAllAssets();
    return Finish("test_render");
}
