#pragma once
#include <cstdint>
#include "../mem.hpp"
#include "../offsets.hpp"
#include "../runtime/snapshot.hpp"

namespace ctl::gta {

// Fills snapshot with current entities (stub for now)
void read_state(Mem& mem, const Offsets& off, Snapshot& snap);

} // namespace ctl::gta
