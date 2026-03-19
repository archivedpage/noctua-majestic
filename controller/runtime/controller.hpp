#pragma once
#include <Windows.h>
#include <atomic>
#include <memory>
#include <imgui.h>
#include "../driver.hpp"
#include "../mem.hpp"
#include "../process.hpp"
#include "../offsets.hpp"
#include "../gta/reader.hpp"
#include "snapshot.hpp"
#include "feature_manager.hpp"
#include "../features/esp.hpp"

namespace ctl::runtime {

class Controller {
public:
    bool init(const wchar_t* proc = L"GTA5.exe");
    void start();
    void stop();
    void frame(ImDrawList* dl, const ImVec2& screen);

private:
    static DWORD WINAPI reader_thread(LPVOID param);

    std::atomic<bool> run_{false};
    HANDLE thread_{nullptr};

    driver::Driver drv_{};
    std::unique_ptr<ctl::Mem> mem_{};
    ctl::Offsets off_{};
    ctl::Snapshot snap_{};
    ctl::features::FeatureManager fm_{};
};

// global singleton
Controller& instance();

} // namespace ctl::runtime