#pragma once
#include <vector>
#include <atomic>
#include <cstdint>

namespace ctl {

struct Vec3 { float x{}, y{}, z{}; };

struct Entity {
    Vec3 pos{};
    std::uintptr_t inst{0};
    bool alive{false};
    bool visible{false};
};

struct GameState {
    // simple fixed-size snapshot for now
    Entity entities[64]{};
    int entity_count{0};
    float viewproj[16]{}; // CViewport+0x250
    std::uintptr_t local_ped{0};
    Vec3 local_pos{};
};

class Snapshot {
public:
    const GameState& read() const {
        return buffers_[ver_.load(std::memory_order_acquire) & 1];
    }
    GameState& begin_write(uint32_t& idx) {
        idx = ((ver_.load(std::memory_order_relaxed) & 1) ^ 1);
        return buffers_[idx];
    }
    void end_write(uint32_t idx) {
        ver_.store(idx, std::memory_order_release);
    }
private:
    std::atomic<uint32_t> ver_{0};
    GameState buffers_[2]{};
};

} // namespace ctl
