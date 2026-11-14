#pragma once
// Owner: E2 (integration lead). Tiny assertion harness shared by all test programs. Header-only.
#include <cmath>
#include <cstdio>
#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

struct TestRegistry {
    int passed = 0, failed = 0;
    std::string current;
};
inline TestRegistry& Reg() { static TestRegistry r; return r; }

#define CHECK(cond)                                                                          \
    do {                                                                                     \
        if (cond) { Reg().passed++; }                                                        \
        else { Reg().failed++; std::printf("  FAIL [%s] %s:%d  %s\n", Reg().current.c_str(), \
                                           __FILE__, __LINE__, #cond); }                     \
    } while (0)
#define CHECK_NEAR(a, b, tol) CHECK(std::fabs(static_cast<double>(a) - static_cast<double>(b)) <= (tol))
#define CHECK_EQ_STR(a, b)                                                                   \
    do {                                                                                     \
        std::string _a = (a), _b = (b);                                                      \
        if (_a == _b) { Reg().passed++; }                                                    \
        else { Reg().failed++; std::printf("  FAIL [%s] %s:%d  \"%s\" != \"%s\"\n",          \
                                           Reg().current.c_str(), __FILE__, __LINE__,        \
                                           _a.c_str(), _b.c_str()); }                        \
    } while (0)
#define CHECK_THROWS(expr)                                                                   \
    do {                                                                                     \
        bool _t = false;                                                                     \
        try { expr; } catch (const std::exception&) { _t = true; }                           \
        CHECK(_t);                                                                           \
    } while (0)

inline void RunTest(const char* name, const std::function<void()>& fn) {
    Reg().current = name;
    int before = Reg().failed;
    try { fn(); } catch (const std::exception& e) {
        Reg().failed++;
        std::printf("  FAIL [%s] unexpected exception: %s\n", name, e.what());
    }
    std::printf("%s %s\n", Reg().failed == before ? "PASS" : "FAIL", name);
}
inline int Finish(const char* suite) {
    std::printf("== %s: %d checks passed, %d failed ==\n", suite, Reg().passed, Reg().failed);
    return Reg().failed == 0 ? 0 : 1;
}
