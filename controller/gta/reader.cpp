#include "reader.hpp"
#include <cmath>

namespace ctl::gta {

static bool read_ped_visibility(Mem& mem, std::uintptr_t ped, bool& out_visible) {
    constexpr std::uintptr_t kCandidates[] = { 0x145C, 0x147C, 0x142C };
    uint8_t flag{};
    for (auto off : kCandidates) {
        if (mem.read(ped + off, flag)) {
            // invisible when flag == 36 || 0 || 4
            out_visible = !(flag == 36 || flag == 0 || flag == 4);
            return true;
        }
    }
    return false;
}

void read_state(Mem& mem, const Offsets& off, Snapshot& snap) {
    uint32_t idx = 0; auto& w = snap.begin_write(idx);
    // 1) matrix
    uintptr_t cv_ptr{};
    if (off.CViewport && mem.read(off.CViewport, cv_ptr) && cv_ptr) {
        mem.read_buf(cv_ptr + 0x250, w.viewproj, sizeof(w.viewproj));
    }

    // 2) ped list/count
    w.entity_count = 0;
    // local ped via CPedFactory
    w.local_ped = 0;
    if (off.CPed_Factory) {
        uintptr_t fac{}; if (mem.read(off.CPed_Factory, fac) && fac) {
            uintptr_t lp{}; mem.read(fac + 0x8, lp);
            w.local_ped = lp;
            if (w.local_ped) {
                uintptr_t pModel{}; if (mem.read(w.local_ped + 0x30, pModel) && pModel) {
                    mem.read(pModel + 0x50, w.local_pos);
                }
            }
        }
    }
    if (off.ReplayInterface) {
        // off.ReplayInterface should be address of a global pointer; deref once
        uintptr_t ri_ptr{}; mem.read(off.ReplayInterface, ri_ptr);
        uintptr_t ri = ri_ptr;
        static int dbg_times = 0;
        uintptr_t ped_face{}; uintptr_t list{}; int count{};
        if (ri) {
            // Peds
            uintptr_t pface{}; uintptr_t plist{}; int pcount{};
            mem.read(ri + 0x18, pface);
            if (pface) {
                mem.read(pface + 0x100, plist);
                mem.read(pface + 0x110, pcount);
            }
            if (dbg_times < 5) {
                wprintf(L"[ctl] RI=%p *RI=%p pface=%p plist=%p pcnt=%d\n",
                    (void*)off.ReplayInterface, (void*)ri, (void*)pface, (void*)plist, pcount);
                dbg_times++;
            }
            // populate from peds if any
            if (plist && pcount > 0) {
                int maxn = (int)(sizeof(w.entities)/sizeof(w.entities[0]));
                int n = pcount < maxn ? pcount : maxn;
                for (int i=0;i<n;++i) {
                    uintptr_t inst{};
                    mem.read(plist + 0x10ull * i, inst);
                    if (!inst) continue;
                    uintptr_t pModel{}; if (!mem.read(inst + 0x30, pModel) || !pModel) continue;
                    ctl::Vec3 pos{}; mem.read(pModel + 0x50, pos);
                    auto& ent = w.entities[w.entity_count];
                    ent.pos = pos;
                    ent.inst = inst;
                    ent.alive = true;
                    bool vis{};
                    if (read_ped_visibility(mem, inst, vis)) ent.visible = vis; else ent.visible = false;
                    w.entity_count++;
                    if (w.entity_count >= maxn) break;
                }
            }
        } else if (dbg_times < 5) {
            wprintf(L"[ctl] RI=%p *RI=%p (invalid)\n", (void*)off.ReplayInterface, (void*)ri);
            dbg_times++;
        }
    }
    snap.end_write(idx);
}

} // namespace ctl::gta
