#!/usr/bin/env bash
# Headless test runner (no raylib / cmake needed): compiles against tests/stubs/raylib_stub.cpp.
# Usage (from anywhere):  bash tests/run_tests.sh
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
OUT="$ROOT/build_test"; mkdir -p "$OUT"
CXX="${CXX:-g++}"
FLAGS="-std=c++17 -Wall -Wextra -g -fsanitize=address,undefined -Itests/stubs -Iinclude -Itests"
STUB="tests/stubs/raylib_stub.cpp"
LOGIC="src/ConfigAndTheme.cpp src/ShipComponent.cpp src/DerivedComponents.cpp src/ComponentDatabase.cpp src/MissionLevel.cpp src/PlanetReport.cpp src/Spacecraft.cpp src/MissionEvaluator.cpp"
RENDER="src/AssetManager.cpp src/Viewport3D.cpp src/LaunchDirector.cpp"
UI="src/UIComponentCatalog.cpp src/UITelemetryHUD.cpp src/UIPlanetReport.cpp src/UIScreens.cpp"

python3 tools/make_placeholder_assets.py >/dev/null
# Headless runs need a font file to exist; the real TTF is not in the repo, so stage a dummy in a scratch dir.
STAGE="$OUT/stage"; rm -rf "$STAGE"; mkdir -p "$STAGE/assets/fonts"
ln -s "$ROOT/data" "$STAGE/data"; ln -s "$ROOT/assets/models" "$STAGE/assets/models"; ln -s "$ROOT/assets/icons" "$STAGE/assets/icons"
if [ -f assets/fonts/JetBrainsMono-Regular.ttf ]; then cp assets/fonts/JetBrainsMono-Regular.ttf "$STAGE/assets/fonts/"; else : > "$STAGE/assets/fonts/JetBrainsMono-Regular.ttf"; fi

build_run() {  # name, sources..., (run inside STAGE so "data/..." and "assets/..." resolve)
  local name="$1"; shift
  $CXX $FLAGS "$@" $STUB -o "$OUT/$name"
  ( cd "$STAGE" && "$OUT/$name" )
}
FAIL=0
build_run test_data tests/test_data.cpp $LOGIC || FAIL=1
build_run test_evaluator tests/test_evaluator.cpp $LOGIC || FAIL=1
[ -f tests/test_launch.cpp ] && { build_run test_launch tests/test_launch.cpp tests/fake_viewport3d.cpp src/LaunchDirector.cpp $LOGIC || FAIL=1; }
[ -f tests/test_render.cpp ] && { build_run test_render tests/test_render.cpp $LOGIC $RENDER || FAIL=1; }
[ -f tests/test_ui_smoke.cpp ] && { build_run test_ui_smoke tests/test_ui_smoke.cpp $LOGIC $RENDER $UI || FAIL=1; }
[ -f tests/test_flow.cpp ] && { build_run test_flow tests/test_flow.cpp $LOGIC $RENDER $UI src/GameManager.cpp || FAIL=1; }
[ $FAIL -eq 0 ] && echo "ALL SUITES PASSED" || { echo "SOME SUITES FAILED"; exit 1; }
