#pragma once
#include <vector>
#include <memory>
#include "../features/feature.hpp"

namespace ctl::features {

class FeatureManager {
public:
    void add(std::unique_ptr<IFeature> f) { feats_.push_back(std::move(f)); }
    void update(const ctl::GameState& s) {
        for (auto& f : feats_) f->update(s);
    }
    void render(ImDrawList* dl, const ImVec2& screen) {
        for (auto& f : feats_) f->render(dl, screen);
    }
private:
    std::vector<std::unique_ptr<IFeature>> feats_;
};

} // namespace ctl::features