#pragma once
#include "feature.hpp"
#include <cmath>
#include <cstdio>

namespace ctl::features {

struct EspSettings {
    // 0 = none, 1 = full box, 2 = corners
    static inline int   style = 1;
    static inline ImU32 color = IM_COL32(255, 255, 255, 255);
    static inline ImU32 distance_color = IM_COL32(220, 220, 220, 255);
    static inline float scale = 0.40f;
    static inline bool  visible_only = false;
    static inline bool  show_distance = false;
    static inline float render_distance = 300.f;
};

class EspBoxes : public IFeature {
public:
    void update(const ctl::GameState& s) override {
        state_ = &s;
    }
    static bool world_to_screen(const ctl::Vec3& v, ImVec2& o, const float m[16], const ImVec2& screen) {
        float x = v.x * m[0] + v.y * m[4] + v.z * m[8]  + m[12];
        float y = v.x * m[1] + v.y * m[5] + v.z * m[9]  + m[13];
        float w = v.x * m[3] + v.y * m[7] + v.z * m[11] + m[15];
        if (w < 0.001f) return false;
        float invw = 1.0f / w;
        x *= invw; y *= invw;
        float sx = (x * 0.5f + 0.5f) * screen.x;
        float sy = (1.0f - (y * 0.5f + 0.5f)) * screen.y;
        o = ImVec2{ sx, sy };
        return true;
    }
    void render(ImDrawList* dl, const ImVec2& screen) override {
        if (!state_) return;
        const float* M = state_->viewproj;
        for (int i=0;i<state_->entity_count;++i) {
            const auto& e = state_->entities[i];
            if (!e.alive) continue;
            if (EspSettings::visible_only && !e.visible) continue; // visible only
            // skip local player
            if (e.inst != 0 && state_->local_ped != 0 && e.inst == state_->local_ped) continue;
            // check render distance
            float dx = e.pos.x - state_->local_pos.x;
            float dy = e.pos.y - state_->local_pos.y;
            float dz = e.pos.z - state_->local_pos.z;
            float dist = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (EspSettings::render_distance < 300.f && dist > EspSettings::render_distance) continue;
            ImVec2 p, p2;
            ctl::Vec3 top = e.pos; top.z += 1.0f;
            if (!world_to_screen(e.pos, p, M, screen)) continue;
            if (!world_to_screen(top, p2, M, screen)) continue;
            float box_h = (p.y - p2.y) * 2.0f * EspSettings::scale;
            float box_w = box_h / 2.4f;
            ImU32 col = EspSettings::color;
            ImVec2 p0{ p.x - box_w, p.y - box_h };
            ImVec2 p1{ p.x + box_w, p.y + box_h };
            if (EspSettings::show_distance && state_->local_ped) {
                int meters = (int)std::lround(dist);
                char buf[32];
                sprintf_s(buf, "[%dm]", meters);
                ImVec2 ts = ImGui::CalcTextSize(buf);
                ImVec2 tp{ p.x - ts.x * 0.5f, p1.y + 3.f };
                const ImU32 outline = IM_COL32(0, 0, 0, 255);
                const ImU32 text_col = EspSettings::distance_color;
                // outline
                dl->AddText(ImVec2(tp.x - 1, tp.y - 1), outline, buf);
                dl->AddText(ImVec2(tp.x - 1, tp.y + 1), outline, buf);
                dl->AddText(ImVec2(tp.x + 1, tp.y - 1), outline, buf);
                dl->AddText(ImVec2(tp.x + 1, tp.y + 1), outline, buf);
                dl->AddText(ImVec2(tp.x - 1, tp.y), outline, buf);
                dl->AddText(ImVec2(tp.x + 1, tp.y), outline, buf);
                dl->AddText(ImVec2(tp.x, tp.y - 1), outline, buf);
                dl->AddText(ImVec2(tp.x, tp.y + 1), outline, buf);
                // main text
                dl->AddText(tp, text_col, buf);
            }
            if (EspSettings::style == 0) continue;
            if (box_h <= 1.f || box_w <= 1.f) continue;
            if (EspSettings::style == 2) {
                const float len = box_w * 0.35f;
                const float th = 1.f;
                const ImU32 outline = IM_COL32(0,0,0,255);
                // top-left
                dl->AddLine(ImVec2(p0.x-1, p0.y-1), ImVec2(p0.x + len+1, p0.y-1), outline, th);
                dl->AddLine(ImVec2(p0.x-1, p0.y+1), ImVec2(p0.x + len+1, p0.y+1), outline, th);
                dl->AddLine(ImVec2(p0.x-1, p0.y-1), ImVec2(p0.x-1, p0.y + len+1), outline, th);
                dl->AddLine(ImVec2(p0.x+1, p0.y-1), ImVec2(p0.x+1, p0.y + len+1), outline, th);
                // top-right
                dl->AddLine(ImVec2(p1.x-len-1, p0.y-1), ImVec2(p1.x+1, p0.y-1), outline, th);
                dl->AddLine(ImVec2(p1.x-len-1, p0.y+1), ImVec2(p1.x+1, p0.y+1), outline, th);
                dl->AddLine(ImVec2(p1.x-1, p0.y-1), ImVec2(p1.x-1, p0.y + len+1), outline, th);
                dl->AddLine(ImVec2(p1.x+1, p0.y-1), ImVec2(p1.x+1, p0.y + len+1), outline, th);
                // bottom-left
                dl->AddLine(ImVec2(p0.x-1, p1.y-1), ImVec2(p0.x + len+1, p1.y-1), outline, th);
                dl->AddLine(ImVec2(p0.x-1, p1.y+1), ImVec2(p0.x + len+1, p1.y+1), outline, th);
                dl->AddLine(ImVec2(p0.x-1, p1.y-len-1), ImVec2(p0.x-1, p1.y+1), outline, th);
                dl->AddLine(ImVec2(p0.x+1, p1.y-len-1), ImVec2(p0.x+1, p1.y+1), outline, th);
                // bottom-right
                dl->AddLine(ImVec2(p1.x-len-1, p1.y-1), ImVec2(p1.x+1, p1.y-1), outline, th);
                dl->AddLine(ImVec2(p1.x-len-1, p1.y+1), ImVec2(p1.x+1, p1.y+1), outline, th);
                dl->AddLine(ImVec2(p1.x-1, p1.y-len-1), ImVec2(p1.x-1, p1.y+1), outline, th);
                dl->AddLine(ImVec2(p1.x+1, p1.y-len-1), ImVec2(p1.x+1, p1.y+1), outline, th);
                // main color
                // top-left
                dl->AddLine(ImVec2(p0.x, p0.y), ImVec2(p0.x + len, p0.y), col, th);
                dl->AddLine(ImVec2(p0.x, p0.y), ImVec2(p0.x, p0.y + len), col, th);
                // top-right
                dl->AddLine(ImVec2(p1.x, p0.y), ImVec2(p1.x - len, p0.y), col, th);
                dl->AddLine(ImVec2(p1.x, p0.y), ImVec2(p1.x, p0.y + len), col, th);
                // bottom-left
                dl->AddLine(ImVec2(p0.x, p1.y), ImVec2(p0.x + len, p1.y), col, th);
                dl->AddLine(ImVec2(p0.x, p1.y), ImVec2(p0.x, p1.y - len), col, th);
                // bottom-right
                dl->AddLine(ImVec2(p1.x, p1.y), ImVec2(p1.x - len, p1.y), col, th);
                dl->AddLine(ImVec2(p1.x, p1.y), ImVec2(p1.x, p1.y - len), col, th);
            } else if (EspSettings::style == 1) {
                const ImU32 outline = IM_COL32(0,0,0,255);
                dl->AddRect(ImVec2(p0.x-1, p0.y-1), ImVec2(p1.x+1, p1.y+1), outline, 0.f, 0, 1.f);
                dl->AddRect(p0, p1, col, 0.f, 0, 1.f);
            }
        }
    }
private:
    const ctl::GameState* state_ = nullptr;
};

} // namespace ctl::features
