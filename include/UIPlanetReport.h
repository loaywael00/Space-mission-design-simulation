#pragma once
// Owner: E4. Frozen contract (Section 3.14).
#include "Viewport3D.h"
#include "PlanetReport.h"

class UIPlanetReport {
public:
    void Update(Viewport3D& viewport, bool locked);   // rotate and reset buttons
    void Draw(const PlanetReport& report) const;

    // Layout inside RECT_ROTATION_CONTROLS (additive, used by tests)
    static Rectangle RotateLeftRect();
    static Rectangle ResetRect();
    static Rectangle RotateRightRect();
    static constexpr float kRotateDegPerSec = 90.0f;
private:
    bool m_locked = false;
    int m_hover = 0;       // 0 none, 1 left, 2 reset, 3 right
    bool m_held = false;   // the hovered button is currently held down
};
