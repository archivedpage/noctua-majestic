#pragma once
#include <imgui.h>
#include "../runtime/snapshot.hpp"

namespace ctl::features {

struct IFeature {
    virtual ~IFeature() = default;
    virtual void update(const ctl::GameState& s) = 0;
    virtual void render(ImDrawList* dl, const ImVec2& screen) = 0;
};

} // namespace ctl::features
