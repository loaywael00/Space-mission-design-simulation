// Owner: E1. Tasks E1-1 .. E1-6. Run from the project root (paths are relative to it).
#include <array>
#include <cstdio>
#include <fstream>
#include <vector>
#include "ComponentDatabase.h"
#include "DerivedComponents.h"
#include "MissionLevel.h"
#include "test_util.h"

namespace {
const char* kParts = "data/parts_catalog.json";
const char* kLevels = "data/levels_config.json";

bool FileExists(const std::string& p) { std::ifstream f(p); return static_cast<bool>(f); }

void WriteTemp(const std::string& path, const std::string& text) {
    std::ofstream f(path);
    f << text;
}

// Data-only requirement checks (no stat thresholds) for one level.
int CountDataOnlyPasses(const ComponentDatabase& db, const MissionLevel& level) {
    int count = 0;
    auto mob = db.GetPartsBySlot(ComponentSlot::MOBILITY);
    auto opt = db.GetPartsBySlot(ComponentSlot::OPTICS);
    auto pay = db.GetPartsBySlot(ComponentSlot::PAYLOAD);
    auto pwr = db.GetPartsBySlot(ComponentSlot::POWER);
    auto shd = db.GetPartsBySlot(ComponentSlot::SHIELDING);
    for (auto* m : mob) for (auto* o : opt) for (auto* p : pay) for (auto* w : pwr) for (auto* s : shd)
        if (m->MeetsMissionRequirement(level) && o->MeetsMissionRequirement(level) &&
            p->MeetsMissionRequirement(level) && w->MeetsMissionRequirement(level) &&
            s->MeetsMissionRequirement(level)) ++count;
    return count;
}
}  // namespace

int main() {
    RunTest("E1-1 CraftStats operator+/-", [] {
        CraftStats a{1, 2, 3, 4, 5, 6, 7}, b{1, 2, 3, 4, 5, 6, 7};
        CraftStats s = a + b;
        CHECK_NEAR(s.health, 2, 1e-6); CHECK_NEAR(s.strength, 4, 1e-6); CHECK_NEAR(s.thrust, 6, 1e-6);
        CHECK_NEAR(s.shielding, 8, 1e-6); CHECK_NEAR(s.mass, 10, 1e-6);
        CHECK_NEAR(s.totalCost, 12, 1e-6); CHECK_NEAR(s.totalBuildTimeSec, 14, 1e-6);
        CraftStats d = s - a;
        CHECK_NEAR(d.health, 1, 1e-6); CHECK_NEAR(d.totalBuildTimeSec, 7, 1e-6);
    });

    RunTest("E1-1 build time formatting", [] {
        CHECK_EQ_STR(FormatDuration(150), "2m 30s");
        CHECK_EQ_STR(FormatDuration(60), "1m 00s");
        CHECK_EQ_STR(FormatDuration(0), "0m 00s");
        CHECK_EQ_STR(FormatDuration(315), "5m 15s");
        CHECK_EQ_STR(FormatDuration(1110), "18m 30s");
        CHECK_EQ_STR(FormatDuration(-5), "0m 00s");
    });

    ComponentDatabase db;
    db.LoadFromJson(kParts);
    auto levels = MissionLevel::LoadAllFromJson(kLevels);

    RunTest("E1-3 database contents", [&] {
        CHECK(db.GetAllParts().size() == 15);
        for (int s = 0; s < 5; ++s)
            CHECK(db.GetPartsBySlot(static_cast<ComponentSlot>(s)).size() == 3);
        ShipComponent* p = db.FindById("payload_t2");
        CHECK(p != nullptr);
        CHECK(dynamic_cast<PayloadComponent*>(p) != nullptr);
        CHECK_EQ_STR(p->GetName(), "Orbital Grapple Arm");
        CHECK(p->GetStats().mass == 50.0f);
        CHECK(p->GetStats().totalCost == 1500.0f);
        CHECK(p->GetStats().totalBuildTimeSec == 180.0f);
        CHECK_EQ_STR(p->GetFormattedBuildTime(), "3m 00s");
        CHECK(db.FindById("nope") == nullptr);
        CHECK_NEAR(db.GetBaseChassisStats().mass, 100.0f, 1e-6);
        CHECK_NEAR(db.GetBaseChassisStats().health, 30.0f, 1e-6);
        CHECK_NEAR(db.GetBaseChassisStats().strength, 15.0f, 1e-6);
        CHECK_NEAR(db.GetBaseChassisStats().totalCost, 0.0f, 1e-6);
        CHECK_EQ_STR(db.GetBaseChassisModelKey(), "base_chassis");
        // catalog order is slot-major, tier-minor
        auto all = db.GetAllParts();
        CHECK_EQ_STR(all.front()->GetId(), "mobility_t1");
        CHECK_EQ_STR(all.back()->GetId(), "shield_t3");
        // subclass attributes (v2 FIX 2)
        CHECK(dynamic_cast<MobilityComponent*>(db.FindById("mobility_t3"))->CanLandOnSurface());
        CHECK(!dynamic_cast<MobilityComponent*>(db.FindById("mobility_t2"))->CanLandOnSurface());
        CHECK(dynamic_cast<OpticsComponent*>(db.FindById("optics_t3"))->GetOpticalResolutionRating() == 3);
        CHECK(dynamic_cast<PowerComponent*>(db.FindById("power_t2"))->GetPowerOutputWatts() == 300.0f);
        CHECK(dynamic_cast<ShieldingComponent*>(db.FindById("shield_t3"))->GetMaxThermalToleranceCelsius() == 600.0f);
        CHECK(dynamic_cast<PayloadComponent*>(db.FindById("payload_t3"))->GetSupportedMissionCapability() ==
              MissionType::SURFACE_CORE_DRILL);
    });

    RunTest("E1-3 database error handling", [&] {
        ComponentDatabase bad;
        CHECK_THROWS(bad.LoadFromJson("data/does_not_exist.json"));
        WriteTemp("/tmp/bad_json.json", "{ not json");
        CHECK_THROWS(bad.LoadFromJson("/tmp/bad_json.json"));
        WriteTemp("/tmp/no_parts.json", "{\"schema_version\":2}");
        CHECK_THROWS(bad.LoadFromJson("/tmp/no_parts.json"));
        // unknown class / unknown slot / duplicate id / missing key
        const std::string chassis = "\"base_chassis\":{\"model_key\":\"base_chassis\",\"mass_kg\":100,\"cost_usd\":0,"
                                    "\"build_time_sec\":0,\"stats\":{\"health\":30,\"strength\":15,\"thrust\":0,\"shielding\":0}},";
        auto part = [](const std::string& id, const std::string& cls, const std::string& slot, const std::string& extra) {
            return "{\"id\":\"" + id + "\",\"class\":\"" + cls + "\",\"slot\":\"" + slot + "\",\"tier\":1,\"name\":\"n\","
                   "\"model_key\":\"m\",\"icon_key\":\"i\",\"description\":\"d\",\"cost_usd\":1,\"build_time_sec\":1,"
                   "\"mass_kg\":1,\"stats\":{\"health\":1,\"strength\":1,\"thrust\":1,\"shielding\":1}" + extra + "}";
        };
        auto file = [&](const std::string& partsArray) {
            WriteTemp("/tmp/case.json", "{\"schema_version\":2," + chassis + "\"parts\":[" + partsArray + "]}");
            return std::string("/tmp/case.json");
        };
        CHECK_THROWS(bad.LoadFromJson(file(part("a", "WarpDriveComponent", "POWER", ""))));
        CHECK_THROWS(bad.LoadFromJson(file(part("a", "PowerComponent", "ENGINES", ",\"power_output_watts\":1"))));
        CHECK_THROWS(bad.LoadFromJson(file(part("a", "PowerComponent", "MOBILITY", ",\"power_output_watts\":1"))));
        CHECK_THROWS(bad.LoadFromJson(file(part("a", "PowerComponent", "POWER", ""))));   // missing subclass key
        CHECK_THROWS(bad.LoadFromJson(file(part("a", "PowerComponent", "POWER", ",\"power_output_watts\":1") + "," +
                                           part("a", "PowerComponent", "POWER", ",\"power_output_watts\":1"))));
        // a failed load must not clobber a good database
        CHECK(db.GetAllParts().size() == 15);
        // and a valid minimal file loads
        bad.LoadFromJson(file(part("a", "PowerComponent", "POWER", ",\"power_output_watts\":1")));
        CHECK(bad.GetAllParts().size() == 1);
    });

    RunTest("E1-4 mission levels", [&] {
        CHECK_EQ_STR(levels[1].GetTitle(), "Phobos Orbital Rock Capture");
        CHECK(levels[2].GetRequiresSurfaceLandingGear());
        CHECK(!levels[0].GetRequiresSurfaceLandingGear());
        CHECK(levels[0].GetMaxBudget() == 3500.0f);
        CHECK(levels[0].GetLevelNumber() == 1 && levels[2].GetLevelNumber() == 3);
        CHECK(levels[0].GetMissionType() == MissionType::FLYBY_RECON);
        CHECK(levels[1].GetMissionType() == MissionType::ORBITAL_SAMPLE);
        CHECK(levels[2].GetMissionType() == MissionType::SURFACE_CORE_DRILL);
        CHECK_EQ_STR(levels[0].GetTitle(), "Mars Flyby Recon");
        CHECK_EQ_STR(levels[2].GetTitle(), "Mars Surface Core Sample");
        CHECK_EQ_STR(levels[0].GetPlanetReport().planetTitle, "MARS PLANET REPORT");
        CHECK_EQ_STR(levels[1].GetPlanetReport().planetTitle, "PHOBOS PLANET REPORT");
        CHECK_EQ_STR(levels[2].GetPlanetReport().telemetryModeText, "Mock Surface");
        CHECK(levels[1].GetMaxBuildTimeSeconds() == 840.0f);
        CHECK(levels[2].GetMinRequiredThrust() == 90.0f);
        CHECK(levels[2].GetRequiredPowerWatts() == 500.0f);
        CHECK(levels[2].GetMinThermalToleranceCelsius() == 500.0f);
        CHECK(levels[1].GetMinOpticsResolution() == 2);
        CHECK_THROWS(MissionLevel::LoadAllFromJson("data/nope.json"));
        WriteTemp("/tmp/two_levels.json", "{\"levels\":[{},{}]}");
        CHECK_THROWS(MissionLevel::LoadAllFromJson("/tmp/two_levels.json"));
    });

    RunTest("E1-2 MeetsMissionRequirement rules (true and false cases)", [&] {
        auto P = [&](const char* id) { return db.FindById(id); };
        const MissionLevel &L1 = levels[0], &L2 = levels[1], &L3 = levels[2];
        // Mobility: only matters when surface landing gear is required
        CHECK(P("mobility_t1")->MeetsMissionRequirement(L1));
        CHECK(P("mobility_t2")->MeetsMissionRequirement(L2));
        CHECK(!P("mobility_t1")->MeetsMissionRequirement(L3));
        CHECK(!P("mobility_t2")->MeetsMissionRequirement(L3));
        CHECK(P("mobility_t3")->MeetsMissionRequirement(L3));
        // Optics: rating >= minimum
        CHECK(P("optics_t1")->MeetsMissionRequirement(L1));
        CHECK(!P("optics_t1")->MeetsMissionRequirement(L2));
        CHECK(P("optics_t2")->MeetsMissionRequirement(L2));
        CHECK(P("optics_t3")->MeetsMissionRequirement(L3));
        // Payload: exact capability match
        CHECK(P("payload_t1")->MeetsMissionRequirement(L1));
        CHECK(!P("payload_t1")->MeetsMissionRequirement(L2));
        CHECK(P("payload_t2")->MeetsMissionRequirement(L2));
        CHECK(!P("payload_t2")->MeetsMissionRequirement(L3));
        CHECK(P("payload_t3")->MeetsMissionRequirement(L3));
        // Power: watts >= required
        CHECK(P("power_t1")->MeetsMissionRequirement(L1));      // 150 >= 100
        CHECK(!P("power_t1")->MeetsMissionRequirement(L2));     // 150 <  250
        CHECK(P("power_t2")->MeetsMissionRequirement(L2));      // 300 >= 250
        CHECK(!P("power_t2")->MeetsMissionRequirement(L3));     // 300 <  500
        CHECK(P("power_t3")->MeetsMissionRequirement(L3));      // 600 >= 500
        // Shielding: thermal >= required
        CHECK(P("shield_t1")->MeetsMissionRequirement(L1));     // 150 >= 100
        CHECK(!P("shield_t1")->MeetsMissionRequirement(L2));    // 150 <  250
        CHECK(P("shield_t2")->MeetsMissionRequirement(L2));     // 300 >= 250
        CHECK(!P("shield_t2")->MeetsMissionRequirement(L3));    // 300 <  500
        CHECK(P("shield_t3")->MeetsMissionRequirement(L3));     // 600 >= 500
    });

    // E1-5: every catalog key must have a model and icon on disk (run after tools/make_placeholder_assets.py).
    RunTest("E1-5 asset-key consistency", [&] {
        CHECK(FileExists("assets/models/" + db.GetBaseChassisModelKey() + ".obj"));
        for (ShipComponent* p : db.GetAllParts()) {
            CHECK(FileExists("assets/models/" + p->GetModelKey() + ".obj"));
            CHECK(FileExists("assets/icons/" + p->GetIconKey() + ".png"));
            CHECK_EQ_STR(p->GetModelKey(), p->GetIconKey());   // icons share the model key (v2 FIX 8)
        }
    });

    // E1-6: data-only balance. Counts re-derived by tools/balance_check.py (the spec's FIX 12 lists
    // only the full-evaluator counts 5/2/1; these data-only counts were missing and are recorded here).
    RunTest("E1-6 data-only balance counts (243 builds)", [&] {
        CHECK(CountDataOnlyPasses(db, levels[0]) == 81);   // 3 x 3 x 1 x 3 x 3
        CHECK(CountDataOnlyPasses(db, levels[1]) == 24);   // 3 x 2 x 1 x 2 x 2
        CHECK(CountDataOnlyPasses(db, levels[2]) == 2);    // 1 x 2 x 1 x 1 x 1
    });

    return Finish("test_data");
}
