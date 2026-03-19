#pragma once
#include <cstdint>
#include <string>
#include "mem.hpp"
#include "utils/pattern.hpp"

namespace ctl {

struct Offsets {
    uintptr_t ReplayInterface = 0;
    uintptr_t CPed_Factory = 0;
    uintptr_t CViewport = 0;
    uintptr_t camGameplayDirector = 0;
    uintptr_t CNetworkPlayerMgr = 0;
    uintptr_t AimCPedPointer = 0;
};

bool resolve_offsets(Mem& mem, Offsets& out);

} // namespace ctl
