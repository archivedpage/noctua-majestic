#include "controller.hpp"
#include <cstdio>

namespace ctl::runtime {

static Controller g_controller;
Controller& instance() { return g_controller; }

static void ensure_console()
{
    static bool once = false; if (once) return; once = true;
    AllocConsole();
    FILE* f=nullptr; freopen_s(&f, "CONOUT$", "w", stdout); freopen_s(&f, "CONOUT$", "w", stderr);
    SetConsoleTitleW(L"noctua controller logs");
}

bool Controller::init(const wchar_t* proc) {
    ensure_console();
    DWORD pid = ctl::find_pid_by_name(proc);
    if (!pid) { wprintf(L"[ctl] pid not found for %ls\n", proc); return false; }
    if (!drv_.is_valid()) { wprintf(L"[ctl] driver handle invalid err=%lu\n", GetLastError()); return false; }
    if (!drv_.attach(pid)) { wprintf(L"[ctl] attach failed pid=%lu err=%lu\n", pid, GetLastError()); return false; }
    mem_ = std::make_unique<ctl::Mem>(drv_);
    if (!ctl::resolve_offsets(*mem_, off_)) {
        wprintf(L"[ctl] resolve_offsets partial/failed\n");
    }
    wprintf(L"[off] ReplayInterface=0x%p CViewport=0x%p CNetMgr=0x%p\n", (void*)off_.ReplayInterface, (void*)off_.CViewport, (void*)off_.CNetworkPlayerMgr);
    // features
    fm_.add(std::make_unique<ctl::features::EspBoxes>());
    return true;
}

DWORD WINAPI Controller::reader_thread(LPVOID param) {
    auto self = reinterpret_cast<Controller*>(param);
    unsigned tick=0;
    while (self->run_.load(std::memory_order_relaxed)) {
        if (self->mem_) {
            ctl::gta::read_state(*self->mem_, self->off_, self->snap_);
            if ((tick++ % 30)==0) {
                const auto& s = self->snap_.read();
                wprintf(L"[ctl] ped_count=%d  M[0]=%.3f M[5]=%.3f\n", s.entity_count, s.viewproj[0], s.viewproj[5]);
                if (s.entity_count>0) {
                    wprintf(L"[ctl] first ped pos: %.2f %.2f %.2f\n", s.entities[0].pos.x, s.entities[0].pos.y, s.entities[0].pos.z);
                }
            }
        }
        Sleep(3);
    }
    return 0;
}

void Controller::start() {
    if (run_.exchange(true)) return;
    thread_ = CreateThread(nullptr, 0, &Controller::reader_thread, this, 0, nullptr);
}

void Controller::stop() {
    if (!run_.exchange(false)) return;
    if (thread_) { WaitForSingleObject(thread_, 3000); CloseHandle(thread_); thread_ = nullptr; }
}

void Controller::frame(ImDrawList* dl, const ImVec2& screen) {
    const auto& s = snap_.read();
    fm_.update(s);
    fm_.render(dl, screen);
}


} // namespace ctl::runtime
