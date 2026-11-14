# Spec issues found while implementing, and how the team resolved them

| # | Where | Problem | Resolution |
|---|-------|---------|-----------|
| 1 | Tasks E3-2 | Placeholder boxes are "6 vertices" (a box has 8) and all "centered on the origin". Centered boxes hide inside the chassis occlusion pass, so Optics (0.6 cube) would be invisible and TC-V1 could never pass. | 8 vertices, 12 triangles. Each box keeps its spec size but is offset to its own socket (shield plate / mobility / chassis / power rear / optics middle / payload front). `--centered` gives the literal spec behaviour. |
| 2 | E1-6 | Says the data-only counts "are recorded in FIX 12". FIX 12 only has the full-evaluator counts (5/2/1). | Derived independently (`tools/balance_check.py`): **81 / 24 / 2** data-only passes (3x3x1x3x3, 3x2x1x2x2, 1x2x1x1x1). Asserted in `test_data.cpp`. |
| 3 | E3-5 | Text says "stage 2 (index 2) shake". Index 2 is Stage 3. | Implemented per the timeline and A3: shake and health drop belong to Stage 3 (index 2). |
| 4 | E3-5 / v1 FILE 12 | Unclear whether stage highlights accumulate. | Each window shows only its own parts (overrides cleared, then set). Easy to change in `LaunchDirector::ApplyHighlight`. |
| 5 | E4-4 | "Rotate-left and rotate-right call AddOrbitDegrees(90 * dt)" gives both the same sign. | Left = -90*dt, right = +90*dt (matches the A / D keys). |
| 6 | v1 FILE 7 | `BeginMode3D` centres the scene on the **screen** (960,540), not on the viewport rectangle's centre (955,420), so the model sits about 120 px low in the viewport. | Kept to the letter of the spec. If it looks off, change `kTargetY` in `Viewport3D.cpp` (about +1.1 units compensates). |
| 7 | v1 Step A | "40 x 40 grid lines" is ambiguous. | Lines at -20..+20 (41 per direction, 40 cells), symmetric about the origin, major line every 5. |
| 8 | v1 FILE 8 | Category tabs are described as "icon tabs", but no tab icons exist (the 15 icons are per part). The "search/filter bar" has no behaviour spec. | Tabs use 3-letter labels (ALL, MOB, OPT, PAY, PWR, SHD). The filter strip shows the active filter and part count; no text search was built. |
| 9 | FIX 12 (A) | Level 3 has exactly one winner with zero thrust margin (90.0 vs 90). | Unchanged (design owner's call). The evaluator uses a 1e-4 epsilon so float rounding cannot break the exact-on-target build. Any mass or thrust edit to the catalog can make Level 3 unwinnable; `test_evaluator` will fail loudly if so. |
| 10 | Data | No build in the shipped data fails Stage 1 on time alone, or Stage 2 on power alone. | Those message branches are covered with synthetic levels (patched JSON) in `test_evaluator.cpp`. |
| 11 | E4-5 | Debrief shows all four stages, but the animation stops at the first failure. | Debrief shows all four (the evaluator always computes all four, TC-4 expects "Stages 3 and 4 PASS"). |
| 12 | Contract | `ScreenAction::LAUNCH` is declared but the HUD returns a bool. | Enum value kept (frozen), unused. |
| 13 | Assets | `JetBrainsMono-Regular.ttf` cannot be generated. | **Not included.** The game exits at startup with `Missing or invalid font file: assets/fonts/JetBrainsMono-Regular.ttf` until you add it (free, OFL licence). |

Small additive changes (no frozen signature touched): `FormatDuration()` in ShipComponent.h; getters on the derived components;
`GetOrthoFovY`/`WrapDegrees` on Viewport3D; layout/getter helpers on UI classes; `include/UIDraw.h`; GameManager read-only accessors;
entering the hangar eases the camera back to 45 degrees; catalog and planet report stay on screen (locked) during launch.
