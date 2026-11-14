// Owner: E2. Tasks E2-1 .. E2-3 and test cases TC-1..TC-5, TC-7, TC-10 (rules level), plus balance.
#include <array>
#include <fstream>
#include <iterator>
#include <vector>
#include "ComponentDatabase.h"
#include "MissionEvaluator.h"
#include "MissionLevel.h"
#include "Spacecraft.h"
#include "test_util.h"

namespace {
ComponentDatabase g_db;
std::array<MissionLevel, 3> g_levels;

void Build(Spacecraft& c, const char* mob, const char* opt, const char* pay, const char* pwr, const char* shd) {
    c.ResetToBareChassis();
    for (const char* id : {mob, opt, pay, pwr, shd}) {
        ShipComponent* p = g_db.FindById(id);
        if (!p) throw std::runtime_error(std::string("missing part ") + id);
        c.EquipComponent(p);
    }
}
// Loads the real levels file with one literal text substitution applied (synthetic edge-case levels).
std::array<MissionLevel, 3> LoadPatchedLevels(const std::string& find, const std::string& replace) {
    std::ifstream in("data/levels_config.json");
    std::string text((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    size_t pos = text.find(find);
    if (pos == std::string::npos) throw std::runtime_error("patch target not found: " + find);
    text.replace(pos, find.size(), replace);
    std::ofstream out("/tmp/patched_levels.json");
    out << text;
    out.close();
    return MissionLevel::LoadAllFromJson("/tmp/patched_levels.json");
}
void L1Ref(Spacecraft& c) { Build(c, "mobility_t1", "optics_t1", "payload_t1", "power_t1", "shield_t1"); }
void L2Ref(Spacecraft& c) { Build(c, "mobility_t2", "optics_t2", "payload_t2", "power_t2", "shield_t2"); }
void L3Ref(Spacecraft& c) { Build(c, "mobility_t3", "optics_t2", "payload_t3", "power_t3", "shield_t3"); }
void AllT3(Spacecraft& c) { Build(c, "mobility_t3", "optics_t3", "payload_t3", "power_t3", "shield_t3"); }
}  // namespace

int main() {
    g_db.LoadFromJson("data/parts_catalog.json");
    g_levels = MissionLevel::LoadAllFromJson("data/levels_config.json");
    Spacecraft craft;
    craft.Initialize(g_db.GetBaseChassisStats());

    RunTest("E2-1 bare chassis and fill state", [&] {
        craft.ResetToBareChassis();
        CHECK_NEAR(craft.GetCurrentStats().mass, 100.0f, 1e-3);
        CHECK_NEAR(craft.GetCurrentStats().health, 30.0f, 1e-3);
        CHECK(!craft.AreAllMandatorySlotsFilled());
        CHECK(craft.GetEquippedComponent(ComponentSlot::POWER) == nullptr);
        craft.EquipComponent(g_db.FindById("power_t1"));
        CHECK(!craft.AreAllMandatorySlotsFilled());
        craft.UnequipSlot(ComponentSlot::POWER);
        CHECK_NEAR(craft.GetCurrentStats().mass, 100.0f, 1e-3);
    });

    RunTest("E2-1 all Tier 3: mass 263, effective thrust 80.8", [&] {
        AllT3(craft);
        CHECK(craft.AreAllMandatorySlotsFilled());
        CHECK_NEAR(craft.GetCurrentStats().mass, 263.0f, 0.01);
        CHECK_NEAR(craft.GetCurrentStats().thrust, 80.8f, 0.1);
        CHECK_NEAR(craft.GetCurrentStats().health, 210.0f, 1e-3);
        CHECK_NEAR(craft.GetCurrentStats().strength, 200.0f, 1e-3);
        CHECK_NEAR(craft.GetCurrentStats().shielding, 140.0f, 1e-3);
        CHECK_NEAR(craft.GetCurrentStats().totalCost, 9500.0f, 1e-3);
        CHECK_NEAR(craft.GetCurrentStats().totalBuildTimeSec, 1080.0f, 1e-3);
    });

    RunTest("E2-1 Level 3 reference: effective thrust exactly 90", [&] {
        L3Ref(craft);
        CHECK_NEAR(craft.GetCurrentStats().thrust, 90.0f, 1e-4);
        CHECK_NEAR(craft.GetCurrentStats().mass, 248.0f, 1e-3);
    });

    RunTest("E2-1 equip replaces same-slot part", [&] {
        craft.ResetToBareChassis();
        craft.EquipComponent(g_db.FindById("power_t1"));
        craft.EquipComponent(g_db.FindById("power_t3"));
        CHECK(craft.GetEquippedComponent(ComponentSlot::POWER) == g_db.FindById("power_t3"));
        CHECK_NEAR(craft.GetCurrentStats().mass, 130.0f, 1e-3);
    });

    RunTest("E2-1 ghost preview stats", [&] {
        L1Ref(craft);                                   // current: mass 195, raw thrust 50
        CHECK(craft.GetGhostComponent() == nullptr);
        CHECK_NEAR(craft.GetPreviewStats().thrust, craft.GetCurrentStats().thrust, 1e-6);
        ShipComponent* hover = g_db.FindById("power_t3");   // swap Li-Ion for RTG
        craft.SetGhostPreviewComponent(hover);
        // raw = 50 - 45 + 125 = 130 ; mass = 195 - 25 + 30 = 200 -> effective 130
        CHECK_NEAR(craft.GetPreviewStats().thrust, 130.0f, 1e-3);
        CHECK_NEAR(craft.GetPreviewStats().mass, 200.0f, 1e-3);
        CHECK_NEAR(craft.GetPreviewStats().totalCost, 2300.0f - 500.0f + 2200.0f, 1e-3);
        CHECK_NEAR(craft.GetCurrentStats().thrust, 50.0f, 1e-3);   // current unchanged by preview
        // hovering the part already equipped changes nothing
        craft.SetGhostPreviewComponent(g_db.FindById("power_t1"));
        CHECK_NEAR(craft.GetPreviewStats().thrust, 50.0f, 1e-3);
        // heavy hover applies the mass penalty to the preview only
        Spacecraft t3;
        t3.Initialize(g_db.GetBaseChassisStats());
        for (const char* id : {"mobility_t3", "optics_t2", "payload_t3", "power_t3"}) t3.EquipComponent(g_db.FindById(id));
        t3.SetGhostPreviewComponent(g_db.FindById("shield_t3"));   // empty slot -> just adds
        CHECK_NEAR(t3.GetPreviewStats().thrust, 90.0f, 1e-3);
        t3.EquipComponent(g_db.FindById("optics_t3"));
        t3.SetGhostPreviewComponent(g_db.FindById("shield_t3"));
        CHECK_NEAR(t3.GetPreviewStats().mass, 263.0f, 1e-3);
        CHECK_NEAR(t3.GetPreviewStats().thrust, 80.8f, 0.1);
        t3.SetGhostPreviewComponent(nullptr);
        CHECK_NEAR(t3.GetPreviewStats().mass, t3.GetCurrentStats().mass, 1e-6);
        // equipping while a ghost is set refreshes the preview
        craft.SetGhostPreviewComponent(g_db.FindById("power_t3"));
        craft.EquipComponent(g_db.FindById("power_t3"));
        CHECK_NEAR(craft.GetPreviewStats().thrust, craft.GetCurrentStats().thrust, 1e-3);
        craft.ResetToBareChassis();
        CHECK(craft.GetGhostComponent() == nullptr);
    });

    RunTest("TC-1 Level 1 reference wins, score 65", [&] {
        L1Ref(craft);
        auto r = MissionEvaluator::Evaluate(craft, g_levels[0]);
        for (auto& s : r.stages) CHECK(s.passed && s.failureReason.empty());
        CHECK(r.overallMissionSuccess);
        CHECK(r.finalEfficiencyScore == 65);
        CHECK_EQ_STR(r.stages[0].stageName, "Budget & Launch Window");
        CHECK_EQ_STR(r.stages[1].stageName, "Propulsion & Power");
        CHECK_EQ_STR(r.stages[2].stageName, "Environmental Survival");
        CHECK_EQ_STR(r.stages[3].stageName, "Scientific Sample");
        CHECK_NEAR(craft.GetCurrentStats().health, 75, 1e-3);
        CHECK_NEAR(craft.GetCurrentStats().strength, 45, 1e-3);
        CHECK_NEAR(craft.GetCurrentStats().thrust, 50, 1e-3);
        CHECK_NEAR(craft.GetCurrentStats().shielding, 25, 1e-3);
        CHECK_NEAR(craft.GetCurrentStats().totalCost, 2300, 1e-3);
        CHECK_NEAR(craft.GetCurrentStats().totalBuildTimeSec, 315, 1e-3);
    });

    RunTest("TC-2 Level 2 reference wins, score 57", [&] {
        L2Ref(craft);
        auto r = MissionEvaluator::Evaluate(craft, g_levels[1]);
        CHECK(r.overallMissionSuccess);
        CHECK(r.finalEfficiencyScore == 57);
        CHECK_NEAR(craft.GetCurrentStats().thrust, 78.9f, 0.1);
    });

    RunTest("TC-3 Level 3 reference wins, score 54", [&] {
        L3Ref(craft);
        auto r = MissionEvaluator::Evaluate(craft, g_levels[2]);
        for (auto& s : r.stages) CHECK(s.passed);
        CHECK(r.overallMissionSuccess);
        CHECK(r.finalEfficiencyScore == 54);
    });

    RunTest("TC-4 Level 3 all Tier 3: thrust failure only", [&] {
        AllT3(craft);
        auto r = MissionEvaluator::Evaluate(craft, g_levels[2]);
        CHECK(r.stages[0].passed);   // 9500 <= 9800, 1080 <= 1110
        CHECK(!r.stages[1].passed);
        CHECK_EQ_STR(r.stages[1].failureReason, "Insufficient thrust-to-weight ratio! Craft failed to achieve target trajectory.");
        CHECK(r.stages[1].testedSlot == ComponentSlot::POWER);
        CHECK(r.stages[2].passed);
        CHECK(r.stages[3].passed);
        CHECK(!r.overallMissionSuccess);
        CHECK(r.finalEfficiencyScore == 0);
    });

    RunTest("TC-5 Level 3 with optics rating 1: stage 4 fails", [&] {
        Build(craft, "mobility_t3", "optics_t1", "payload_t3", "power_t3", "shield_t3");
        auto r = MissionEvaluator::Evaluate(craft, g_levels[2]);
        CHECK(r.stages[0].passed && r.stages[1].passed && r.stages[2].passed);
        CHECK(!r.stages[3].passed);
        CHECK_EQ_STR(r.stages[3].failureReason, "Incompatible scientific payload equipped for this mission objective!");
        CHECK(r.stages[3].testedSlot == ComponentSlot::PAYLOAD);
        CHECK(!r.overallMissionSuccess);
    });

    RunTest("TC-7 Level 1 all Tier 3: budget failure", [&] {
        AllT3(craft);
        auto r = MissionEvaluator::Evaluate(craft, g_levels[0]);
        CHECK(!r.stages[0].passed);
        CHECK_EQ_STR(r.stages[0].failureReason, "Budget exceeded! NASA funding cap violated.");
        CHECK(!r.overallMissionSuccess);
    });

    RunTest("TC-10 Level 2 with the Level 1 build: thrust message wins over power", [&] {
        L1Ref(craft);
        auto r = MissionEvaluator::Evaluate(craft, g_levels[1]);
        CHECK(r.stages[0].passed);
        CHECK(!r.stages[1].passed);   // thrust 50 < 75 AND power 150 < 250
        CHECK_EQ_STR(r.stages[1].failureReason, "Insufficient thrust-to-weight ratio! Craft failed to achieve target trajectory.");
    });

    RunTest("power-only failure shows the power message", [&] {
        // Synthetic Level 1: demand 250 W (the L1 ref has 150 W) but only 10 thrust, so thrust passes.
        auto lv = LoadPatchedLevels("\"required_power_watts\": 100", "\"required_power_watts\": 250");
        auto lv2 = LoadPatchedLevels("\"min_thrust\": 40", "\"min_thrust\": 10");
        L1Ref(craft);
        auto r = MissionEvaluator::Evaluate(craft, lv[0]);     // thrust 50 >= 40, power 150 < 250
        CHECK(!r.stages[1].passed);
        CHECK_EQ_STR(r.stages[1].failureReason, "Insufficient power output for mission demand!");
        CHECK(r.stages[1].testedSlot == ComponentSlot::POWER);
        CHECK(!r.overallMissionSuccess);
        CHECK(MissionEvaluator::Evaluate(craft, lv2[0]).stages[1].passed);   // control: normal power, low thrust bar
    });

    RunTest("stage 1 time-only failure shows the launch-window message", [&] {
        // The shipped data has no build that busts only the clock, so shrink Level 1's window to 100 s.
        auto lv = LoadPatchedLevels("\"max_build_time_sec\": 420", "\"max_build_time_sec\": 100");
        L1Ref(craft);                                          // cost 2300 <= 3500, time 315 > 100
        auto r = MissionEvaluator::Evaluate(craft, lv[0]);
        CHECK(!r.stages[0].passed);
        CHECK_EQ_STR(r.stages[0].failureReason, "Craft finished after the planetary launch window closed!");
        // when both fail, the budget message takes priority
        auto lv2 = LoadPatchedLevels("\"max_budget_usd\": 3500", "\"max_budget_usd\": 1000");
        CHECK_EQ_STR(MissionEvaluator::Evaluate(craft, lv2[0]).stages[0].failureReason,
                     "Budget exceeded! NASA funding cap violated.");
    });

    RunTest("all four stages are always evaluated (no short-circuit)", [&] {
        // Level 2 with the Level 1 build fails stages 2, 3 and 4 independently.
        L1Ref(craft);
        auto r = MissionEvaluator::Evaluate(craft, g_levels[1]);
        CHECK(r.stages[0].passed);
        CHECK(!r.stages[1].passed && !r.stages[2].passed && !r.stages[3].passed);
        CHECK_EQ_STR(r.stages[2].failureReason, "Structural or thermal failure! Hull/Suspension collapsed under planetary conditions.");
    });

    RunTest("empty slots never pass requirement checks", [&] {
        craft.ResetToBareChassis();
        auto r = MissionEvaluator::Evaluate(craft, g_levels[0]);
        CHECK(!r.stages[1].passed && !r.stages[2].passed && !r.stages[3].passed);
        CHECK(!r.overallMissionSuccess);
        CHECK(r.finalEfficiencyScore == 0);
    });

    // E2-3: all 243 builds per level through the evaluator.
    RunTest("E2-3 balance: 243 builds per level -> 5 / 2 / 1 winners", [&] {
        const char* slots[5][3] = {
            {"mobility_t1", "mobility_t2", "mobility_t3"}, {"optics_t1", "optics_t2", "optics_t3"},
            {"payload_t1", "payload_t2", "payload_t3"},    {"power_t1", "power_t2", "power_t3"},
            {"shield_t1", "shield_t2", "shield_t3"}};
        int wins[3] = {0, 0, 0};
        std::vector<std::string> l3winner;
        int total = 0;
        for (int a = 0; a < 3; ++a) for (int b = 0; b < 3; ++b) for (int c = 0; c < 3; ++c)
        for (int d = 0; d < 3; ++d) for (int e = 0; e < 3; ++e) {
            Build(craft, slots[0][a], slots[1][b], slots[2][c], slots[3][d], slots[4][e]);
            ++total;
            for (int lv = 0; lv < 3; ++lv)
                if (MissionEvaluator::Evaluate(craft, g_levels[lv]).overallMissionSuccess) {
                    ++wins[lv];
                    if (lv == 2) l3winner = {slots[0][a], slots[1][b], slots[2][c], slots[3][d], slots[4][e]};
                }
        }
        CHECK(total == 243);
        CHECK(wins[0] == 5);
        CHECK(wins[1] == 2);
        CHECK(wins[2] == 1);
        CHECK(l3winner.size() == 5 && l3winner[0] == "mobility_t3" && l3winner[1] == "optics_t2" &&
              l3winner[2] == "payload_t3" && l3winner[3] == "power_t3" && l3winner[4] == "shield_t3");
    });

    return Finish("test_evaluator");
}
