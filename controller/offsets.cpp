#include "offsets.hpp"
#include <vector>
#include <algorithm>
#include <Windows.h>

#include "utils/signature.hpp"

namespace ctl {
using namespace ctl::utils;

static bool read_pe_headers(Mem& mem, uintptr_t base, IMAGE_DOS_HEADER& dos, IMAGE_NT_HEADERS64& nt) {
    if (!mem.read(base, dos) || dos.e_magic != IMAGE_DOS_SIGNATURE) return false;
    if (!mem.read(base + dos.e_lfanew, nt) || nt.Signature != IMAGE_NT_SIGNATURE) return false;
    return true;
}

static bool get_text_section(Mem& mem, uintptr_t base, uintptr_t& text_va, uint32_t& text_sz) {
    IMAGE_DOS_HEADER dos{}; IMAGE_NT_HEADERS64 nt{};
    if (!read_pe_headers(mem, base, dos, nt)) return false;
    const auto nsects = nt.FileHeader.NumberOfSections;
    std::vector<IMAGE_SECTION_HEADER> sh(nsects);
    auto sect_off = base + dos.e_lfanew + sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) + nt.FileHeader.SizeOfOptionalHeader;
    if (!mem.read_buf(sect_off, sh.data(), sizeof(IMAGE_SECTION_HEADER) * nsects)) return false;
    for (unsigned i=0;i<nsects;++i) {
        if (memcmp(sh[i].Name, ".text", 5)==0) {
            text_va = base + sh[i].VirtualAddress;
            text_sz = sh[i].Misc.VirtualSize ? sh[i].Misc.VirtualSize : sh[i].SizeOfRawData;
            return true;
        }
    }
    text_va = base; text_sz = nt.OptionalHeader.SizeOfImage; // fallback
    return true;
}

static uintptr_t resolve_rip(uintptr_t instr, int disp_off) {
    return 0; // caller computes
}

static bool scan_one(Mem& mem, uintptr_t text, uint32_t text_sz, const char* ida, uintptr_t& out, int rip_mode /*0=none,1=call,2=mov/lea*/) {
    auto pat = parse_ida_pattern(ida);
    const size_t chunk = 512*1024;
    std::vector<uint8_t> buf(chunk + pat.bytes.size());
    for (uintptr_t pos=0; pos<text_sz; pos+=chunk) {
        size_t to_read = std::min<size_t>(chunk + pat.bytes.size(), text_sz - pos);
        if (!mem.read_buf(text + pos, buf.data(), to_read)) return false;
        if (auto p = scan_buf(buf.data(), to_read, pat)) {
            uintptr_t match = text + pos + (p - buf.data());
            if (rip_mode == 1) { // call rel32: E8 rel
                int32_t rel{}; mem.read(match + 1, rel);
                out = (match + 5) + rel; return true;
            } else if (rip_mode == 2) { // 48 8B/8D 0D/05/15 disp32
                int32_t d{}; mem.read(match + 3, d);
                out = (match + 7) + d; return true;
            } else {
                out = match; return true;
            }
        }
        if (to_read < chunk) break;
    }
    return false;
}

bool resolve_offsets(Mem& mem, Offsets& out) {
    uintptr_t base{}; if (!mem.image_base(base) || !base) return false;
    // Use remote signature scanner identical to original implementation
    utils::signature s(mem, "GTA5.exe");

    out.ReplayInterface = s.scan("E8 ? ? ? ? 40 02 FB 40 0A F0 40 3A FB 72 E1 40 84 F6 75 72 81 7D ? ? ? ? ? 75 69 81 7D ? ? ? ? ? 75 60 8B 05 ? ? ? ? 39 45 BC").add(1).rip().add(0x84).add(3).rip().as<uintptr_t>();
    out.CPed_Factory   = s.scan("75 17 48 8B 0D ? ? ? ? 8B C3").add(2).add(3).rip();
    out.CViewport      = s.scan("48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD").add(3).rip();
    out.camGameplayDirector = s.scan("48 8B 05 ? ? ? ? 38 98 ? ? ? ? 8A C3").add(3).rip();
    out.CNetworkPlayerMgr   = s.scan("48 8B 0D ? ? ? ? E8 ? ? ? ? 48 85 C0 74 37").add(3).rip();
    out.AimCPedPointer      = s.scan("E8 ? ? ? ? B1 01 48 81 C4").add(1).rip().add(0x293).add(3).rip();

    // Sanity: ReplayInterface should deref to non-null
    uintptr_t ri_ptr{}; if (!mem.read(out.ReplayInterface, ri_ptr) || !ri_ptr) {
        // allow other offsets even if RI failed
        return out.CPed_Factory || out.CViewport || out.camGameplayDirector || out.CNetworkPlayerMgr || out.AimCPedPointer;
    }

    return out.ReplayInterface || out.CPed_Factory || out.CViewport || out.camGameplayDirector || out.CNetworkPlayerMgr || out.AimCPedPointer;
}

} // namespace ctl
