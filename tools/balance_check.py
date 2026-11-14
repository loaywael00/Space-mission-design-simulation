#!/usr/bin/env python3
"""QA: independent re-implementation of the rules (no shared code with the C++), run from project root.
Prints the numbers every C++ test asserts against."""
import itertools, json, math

parts = json.load(open("data/parts_catalog.json"))
levels = json.load(open("data/levels_config.json"))["levels"]
base = parts["base_chassis"]
by_slot = {}
for p in parts["parts"]:
    by_slot.setdefault(p["slot"], []).append(p)
SLOTS = ["MOBILITY", "OPTICS", "PAYLOAD", "POWER", "SHIELDING"]

def totals(build):
    s = {k: base["stats"][k] for k in ("health", "strength", "thrust", "shielding")}
    mass, cost, time = base["mass_kg"], 0, 0
    for p in build:
        for k in s: s[k] += p["stats"][k]
        mass += p["mass_kg"]; cost += p["cost_usd"]; time += p["build_time_sec"]
    s["raw_thrust"] = s["thrust"]
    s["thrust"] = s["raw_thrust"] * (250.0 / max(250.0, mass))
    return s, mass, cost, time

def data_checks(build, lv):
    m, o, pl, pw, sh = build
    return dict(
        mobility=(not lv["requires_surface_landing_gear"]) or m["can_land_on_surface"],
        optics=o["optical_resolution_rating"] >= lv["min_optics_resolution"],
        payload=pl["supported_mission_type"] == lv["mission_type"],
        power=pw["power_output_watts"] >= lv["required_power_watts"],
        shield=sh["max_thermal_tolerance_celsius"] >= lv["min_thermal_tolerance_celsius"])

def evaluate(build, lv):
    s, mass, cost, time = totals(build)
    d = data_checks(build, lv)
    st1 = cost <= lv["max_budget_usd"] and time <= lv["max_build_time_sec"]
    st2 = s["thrust"] >= lv["min_thrust"] - 1e-4 and d["power"]
    st3 = (s["health"] >= lv["min_health"] and s["strength"] >= lv["min_strength"] and
           s["shielding"] >= lv["min_shielding"] and d["mobility"] and d["shield"])
    st4 = d["optics"] and d["payload"]
    ok = st1 and st2 and st3 and st4
    score = round(50 + 25 * (1 - cost / lv["max_budget_usd"]) + 25 * (1 - time / lv["max_build_time_sec"])) if ok else 0
    return ok, score

builds = list(itertools.product(*[by_slot[s] for s in SLOTS]))
print("total builds:", len(builds))
for lv in levels:
    wins = [b for b in builds if evaluate(b, lv)[0]]
    data_only = sum(1 for b in builds if all(data_checks(b, lv).values()))
    print("Level %d: full-evaluator wins=%d  data-only passes=%d" % (lv["level_number"], len(wins), data_only))
    if len(wins) <= 5:
        for b in wins:
            print("   win:", [p["id"] for p in b], "score", evaluate(b, lv)[1])
ids = lambda *x: [next(p for p in parts["parts"] if p["id"] == i) for i in x]
refs = {1: ids("mobility_t1","optics_t1","payload_t1","power_t1","shield_t1"),
        2: ids("mobility_t2","optics_t2","payload_t2","power_t2","shield_t2"),
        3: ids("mobility_t3","optics_t2","payload_t3","power_t3","shield_t3")}
for n, b in refs.items():
    s, mass, cost, time = totals(b)
    print("L%d ref: score=%s mass=%s thrust=%.2f cost=%s time=%s health=%s str=%s shd=%s" %
          (n, evaluate(b, levels[n-1])[1], mass, s["thrust"], cost, time, s["health"], s["strength"], s["shielding"]))
t3 = ids("mobility_t3","optics_t3","payload_t3","power_t3","shield_t3")
s, mass, cost, time = totals(t3)
print("all-T3: mass=%s thrust=%.2f cost=%s time=%s health=%s str=%s shd=%s" % (mass, s["thrust"], cost, time, s["health"], s["strength"], s["shielding"]))
