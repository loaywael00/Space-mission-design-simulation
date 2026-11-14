# NASA Craft Builder (C++17 / raylib 5.0)

Implements the whole task breakdown (E1 data, E2 rules/flow, E3 3D, E4 UI/build) against the frozen contract.

## Run the game (needs cmake, a C++17 compiler, internet for the first configure)
1. Download `JetBrainsMono-Regular.ttf` into `assets/fonts/`.
2. Download nlohmann/json **v3.11.3** `json.hpp` and overwrite `include/third_party/json.hpp`
   (the file there is a small offline stand-in with the same API subset).
3. `python3 tools/make_placeholder_assets.py`   (placeholder models and icons; real art replaces files of the same name)
4. `cmake -S . -B build && cmake --build build && cmake --build build --target run`

Controls: mouse for everything; A / D or drag to orbit; wheel zooms over the viewport and scrolls the catalog over the catalog.

## Tests (no GPU, no raylib, no cmake needed)
`bash tests/run_tests.sh` compiles every source with `-Wall -Wextra -fsanitize=address,undefined` against
`tests/stubs/` (a headless raylib stand-in, tests only) and runs six suites, about 545 checks:
data layer, evaluator + 243-build balance, LaunchDirector timeline, camera/asset/draw-call structure,
UI (simulated clicks), and the full campaign through the real GameManager.
`tools/balance_check.py` is an independent Python re-implementation used to cross-check the numbers.
With CMake: `cmake -S . -B build -DBUILD_TESTS=ON` then `bash tests/run_tests.sh` stages the working directory.

See `docs/SPEC_ISSUES.md` for spec problems found and how they were resolved.
