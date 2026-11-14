// Owner: E3. Task E3-5 (+ TC-V2 timeline expectations) against a recording fake viewport.
#include <array>
#include "LaunchDirector.h"
#include "test_util.h"

struct FakeViewportLog {
    Color chassis; std::array<Color, 5> slot; std::array<bool, 5> overridden; float autoRotate;
    int shakeCalls; float shakeSeconds, shakeMagnitude; int clearCalls;
};
extern FakeViewportLog g_fakeViewport;

namespace {
bool Same(Color a, Color b) { return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a; }
bool IsGreen(Color c) { return Same(c, COLOR_STATUS_NOMINAL); }
bool IsRed(Color c) { return Same(c, COLOR_STATUS_WARNING); }

MissionEvaluationReport MakeReport(bool p0, bool p1, bool p2, bool p3) {
    MissionEvaluationReport r;
    bool ok[4] = {p0, p1, p2, p3};
    r.overallMissionSuccess = p0 && p1 && p2 && p3;
    for (int i = 0; i < 4; ++i) { r.stages[i].passed = ok[i]; r.stages[i].stageName = "S" + std::to_string(i + 1); }
    return r;
}
// Steps in exact-binary 0.25 s ticks so times land precisely on 3/6/9/12.
float RunUntilFinished(LaunchDirector& d, Viewport3D& v, float maxSeconds = 20.0f) {
    float t = 0;
    while (!d.IsFinished() && t < maxSeconds) { d.Update(0.25f, v); t += 0.25f; }
    return d.GetElapsedSeconds();
}
}  // namespace

int main() {
    Viewport3D vp;

    RunTest("all-pass report finishes at 12.0 s", [&] {
        LaunchDirector d;
        CHECK(d.GetCurrentStageIndex() == -1);
        CHECK(!d.IsRunning() && !d.IsFinished());
        d.Start(MakeReport(true, true, true, true));
        CHECK(d.IsRunning());
        d.Update(0.25f, vp);
        CHECK_NEAR(d.GetElapsedSeconds(), 0.25, 1e-6);
        CHECK(d.GetCurrentStageIndex() == 0);
        float t = RunUntilFinished(d, vp);
        CHECK_NEAR(t, 12.0, 1e-5);
        CHECK(d.IsFinished() && !d.IsRunning());
        CHECK(d.GetCurrentStageIndex() == 3);
        CHECK_NEAR(g_fakeViewport.autoRotate, 0.0, 1e-6);          // auto-rotate stopped
        CHECK(!g_fakeViewport.overridden[0] && !g_fakeViewport.overridden[3]);   // overrides cleared
        CHECK(Same(g_fakeViewport.chassis, COLOR_CAD_WIRE_GOLD));
        CHECK(g_fakeViewport.shakeCalls == 0);
    });

    RunTest("failure at stage 2 finishes at 6.0 s", [&] {
        LaunchDirector d;
        d.Start(MakeReport(true, false, true, true));
        CHECK_NEAR(RunUntilFinished(d, vp), 6.0, 1e-5);
        CHECK(d.GetCurrentStageIndex() == 1);
    });

    RunTest("failure at stage 1 finishes at 3.0 s", [&] {
        LaunchDirector d;
        d.Start(MakeReport(false, true, true, true));
        CHECK_NEAR(RunUntilFinished(d, vp), 3.0, 1e-5);
        CHECK(d.GetCurrentStageIndex() == 0);
    });

    RunTest("failure at stage 3 finishes at 9.0 s; shake at 6 s; health drops", [&] {
        LaunchDirector d;
        g_fakeViewport.shakeCalls = 0;
        d.Start(MakeReport(true, true, false, true));
        CHECK_NEAR(d.GetAnimatedHealth(200.0f), 200.0, 1e-4);
        for (int i = 0; i < 23; ++i) d.Update(0.25f, vp);          // t = 5.75
        CHECK(g_fakeViewport.shakeCalls == 0);
        CHECK_NEAR(d.GetAnimatedHealth(200.0f), 200.0, 1e-4);
        d.Update(0.25f, vp);                                       // t = 6.0
        CHECK(g_fakeViewport.shakeCalls == 1);
        CHECK_NEAR(g_fakeViewport.shakeSeconds, 0.5, 1e-6);
        CHECK_NEAR(g_fakeViewport.shakeMagnitude, 0.1, 1e-6);
        d.Update(0.25f, vp);                                       // t = 6.25 -> 25% of the 1 s drop
        CHECK_NEAR(d.GetAnimatedHealth(200.0f), 150.0, 1e-3);
        d.Update(0.25f, vp); d.Update(0.25f, vp);                  // t = 6.75
        CHECK_NEAR(d.GetAnimatedHealth(200.0f), 50.0, 1e-3);
        CHECK_NEAR(RunUntilFinished(d, vp), 9.0, 1e-5);
        CHECK_NEAR(d.GetAnimatedHealth(200.0f), 0.0, 1e-4);        // stays at 0 for the debrief
        CHECK(g_fakeViewport.shakeCalls == 1);                     // fired exactly once
    });

    RunTest("stage 3 failing after an earlier failure never shakes", [&] {
        LaunchDirector d;
        g_fakeViewport.shakeCalls = 0;
        d.Start(MakeReport(true, false, false, true));             // first failure is stage 2 -> ends at 6.0
        RunUntilFinished(d, vp);
        CHECK(g_fakeViewport.shakeCalls == 0);
        CHECK_NEAR(d.GetAnimatedHealth(200.0f), 200.0, 1e-4);
    });

    RunTest("TC-V2 stage highlight timeline (all-pass = all green)", [&] {
        LaunchDirector d;
        d.Start(MakeReport(true, true, true, true));
        d.Update(0.25f, vp);                                       // t=0.25  stage 1
        CHECK(IsGreen(g_fakeViewport.chassis));
        CHECK(!g_fakeViewport.overridden[3]);
        CHECK_NEAR(g_fakeViewport.autoRotate, 25.0, 1e-6);
        for (int i = 0; i < 12; ++i) d.Update(0.25f, vp);          // t=3.25 stage 2: only POWER
        CHECK(d.GetCurrentStageIndex() == 1);
        CHECK(g_fakeViewport.overridden[3] && IsGreen(g_fakeViewport.slot[3]));
        CHECK(!g_fakeViewport.overridden[0] && !g_fakeViewport.overridden[4]);
        CHECK(Same(g_fakeViewport.chassis, COLOR_CAD_WIRE_GOLD));
        for (int i = 0; i < 12; ++i) d.Update(0.25f, vp);          // t=6.25 stage 3: MOBILITY + SHIELDING
        CHECK(d.GetCurrentStageIndex() == 2);
        CHECK(g_fakeViewport.overridden[0] && g_fakeViewport.overridden[4]);
        CHECK(!g_fakeViewport.overridden[3]);
        for (int i = 0; i < 12; ++i) d.Update(0.25f, vp);          // t=9.25 stage 4: PAYLOAD + OPTICS
        CHECK(d.GetCurrentStageIndex() == 3);
        CHECK(g_fakeViewport.overridden[2] && g_fakeViewport.overridden[1]);
        CHECK(!g_fakeViewport.overridden[0] && !g_fakeViewport.overridden[4]);
    });

    RunTest("failed stage colours red", [&] {
        LaunchDirector d;
        d.Start(MakeReport(false, true, true, true));
        d.Update(0.25f, vp);
        CHECK(IsRed(g_fakeViewport.chassis));
        d.Start(MakeReport(true, false, true, true));
        for (int i = 0; i < 13; ++i) d.Update(0.25f, vp);
        CHECK(IsRed(g_fakeViewport.slot[3]));
    });

    RunTest("Update is inert before Start and after finish", [&] {
        LaunchDirector d;
        d.Update(1.0f, vp);
        CHECK_NEAR(d.GetElapsedSeconds(), 0.0, 1e-6);
        d.Start(MakeReport(true, true, true, true));
        RunUntilFinished(d, vp);
        float t = d.GetElapsedSeconds();
        d.Update(1.0f, vp);
        CHECK_NEAR(d.GetElapsedSeconds(), t, 1e-6);
        d.Start(MakeReport(true, true, true, true));               // restart resets everything
        CHECK(d.IsRunning() && !d.IsFinished());
        CHECK_NEAR(d.GetElapsedSeconds(), 0.0, 1e-6);
    });

    return Finish("test_launch");
}
