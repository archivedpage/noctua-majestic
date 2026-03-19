#include <Windows.h>
#include <atlbase.h>

#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <dxgi.h>
#pragma comment(lib, "dxgi.lib")

#pragma comment( lib, "freetype64.lib" )

#include <undocumented.h>
#include <vtablehook.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include <gui.h>
#include <image.hpp>

// controller test includes
#include "..\\controller\\runtime\\controller.hpp"
#include "..\\controller\\features\\esp.hpp"
#include <atomic>

using namespace ImGui;
using namespace ui;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

decltype(&PresentDWM) fn_PresentDWM = nullptr;
decltype(&PresentMultiplaneOverlay) fn_PresentMultiplaneOverlay = nullptr;

static CComPtr<ID3D11Device>            g_Device = nullptr;
static CComPtr<ID3D11DeviceContext>     g_DeviceContext = nullptr;
static CComPtr<ID3D11RenderTargetView>  g_RTV = nullptr;
static CComPtr<IDXGISwapChainDWMLegacy> g_SwapChain = nullptr;

static HHOOK g_hMouseHook = nullptr;
static HHOOK g_hKeyboardHook = nullptr;

static ID3D11ShaderResourceView* avatar = nullptr;

static bool g_menu_open = true;
static std::atomic<bool> g_panic{ false };

static void draw(IDXGISwapChainDWMLegacy* pSwapChain);
static void setup_ui_once();
static void PanicShutdown();

static bool g_bools[100] = {};
static int  g_ints[100] = {};
static float g_floats[100] = {};
static float g_cols[100][4] = {};
static c_key g_keys[100] = {};
static std::vector<multi_select_item> g_items { "Head", "Body", "Chest", "Legs", "Arms" };
static bool g_pages_built = false;

static void build_pages()
{
    if (g_pages_built) return;

    for (int i = 0; i < 100; ++i) {
        g_cols[i][0] = 1.f;
        g_cols[i][1] = 1.f;
        g_cols[i][2] = 1.f;
        g_cols[i][3] = 1.f;
    }
    
    g_floats[0] = 300.f;

    add_page(0, [](){
        child("aimbot", []() {
            // todo
        });
    });

    add_page(1, [](){
        child("esp", []() {
            checkbox("enable", &g_bools[0]);
            combo("box", &g_ints[60], { "none", "full", "corners" });
            color_edit("box color", g_cols[0]);
            checkbox("name", &g_bools[3], 0, { g_cols[1] });
            combo("health", &g_ints[61], { "none", "hp only", "hp and armor", "adaptive" });
            checkbox("distance", &g_bools[8], 0, { g_cols[2] });
            slider_float("render distance", &g_floats[0], 1.f, 300.f, g_floats[0] >= 300.f ? "unlimited" : "%.0fm");
            checkbox("weapon", &g_bools[6]);
            checkbox("skeleton", &g_bools[9]);
            checkbox("viewline", &g_bools[10]);
            checkbox("visible only", &g_bools[11]);
        });
    });

    add_page(2, []() {
        BeginGroup();
        {
            child("menu", []() {
                binder("menu key", &g_keys[0]);
                binder("panic key", &g_keys[7]);
                binder("console key", &g_keys[1]);
                checkbox("watermark", &g_bools[17]);
                checkbox("hotkeys list", &g_bools[18]);
                checkbox("spectators list", &g_bools[19]);
                if (color_edit("menu col", menu_col)) { }
            });

            child("movement", []() {
                checkbox("bunnyhop", &g_bools[20], 0, { }, []() { });
                checkbox("auto strafe", &g_bools[21], 0, { }, []() { });
                checkbox("strafe assist", &g_bools[22]);
                checkbox("fast stop", &g_bools[23], 0, { }, 0, "fast stop description");
            });

            child("other", []() {
                checkbox("safe mode", &g_bools[50], 0, { }, 0, "safe mode description");
                checkbox("obs bypass", &g_bools[51], 0, { }, 0, "obs bypass description");
                checkbox("cheat communication", &g_bools[52], 0, { }, 0, "cheat communication description");
                checkbox("block luacmd calls", &g_bools[53], 0, { }, []() { }, "block luacmd calls description");
                checkbox("event listeners", &g_bools[54], 0, { }, 0, "event listeners description");
                checkbox("engine prediction", &g_bools[55], 0, { }, 0, "engine prediction description");
                checkbox("screengrab notify", &g_bools[56], 0, { }, 0, "screengrab notify description");
                checkbox("use spammer", &g_bools[57], 0, { }, 0, "use spammer description");
                checkbox("auto pistol", &g_bools[58], 0, { }, 0, "auto pistol description");
            });
        }
        EndGroup();

        SameLine();

        BeginGroup();
        {
            child("config", []() {
                combo("configs", &g_ints[20], { "select config", "rage", "legit", "semirage", "visuals" });
                if (button("load")) { notify::add("config successfully loaded", notify::notify_success); }
                if (button("save")) { notify::add("unable to save config", notify::notify_error); }
                if (button("cloud")) { }
                if (button("create")) { notify::add("config successfully created", notify::notify_success); }
                if (button("open config folder")) { }
            });

            child("camera", []() {
                checkbox("FOV changer", &g_bools[24], 0, { }, []() { });
                checkbox("zoom", &g_bools[25], &g_keys[2], { }, []() { });
                checkbox("thirdperson", &g_bools[26], &g_keys[3], { }, []() { });
                checkbox("freecam", &g_bools[27], &g_keys[4], { }, []() { });
                slider_float("aspect ratio", &g_floats[10], 0, 10, "%.2f");
            });

            child("pathfinder", []() {
                checkbox("enable##pathfinder", &g_bools[28]);
                binder("set destination", &g_keys[5]);
                binder("recalculate path", &g_keys[6]);
            });
        }
        EndGroup();
    });

    add_page(3, [](){
        static int selected_player = 0;
        static std::vector<const char*> players { "shye", "michael conors", "yui" };

        child("players", [&](){
            PushFont(fonts[font].get(13));
            static char search[32];
            InputTextWithHint("search", "type...", search, sizeof(search));
            PopFont();

            for (int i = 0; i < players.size(); ++i) {
                if (!strstr(players[i], search)) continue;
                BeginChild(players[i], { 0, 17 }, 0, ImGuiWindowFlags_NoBackground);
                {
                    GetWindowDrawList()->AddImageRounded(avatar, GetCurrentWindow()->DC.CursorPos, GetCurrentWindow()->DC.CursorPos + SCALE(17, 17), { 0, 0 }, { 1, 1 }, col(255, 255, 255, 1.f), SCALE(3));
                    Dummy(SCALE(17, 17));
                    SameLine(0, SCALE(8));
                    GetWindowDrawList()->AddText(GetCurrentWindow()->DC.CursorPos + ImVec2{ 0, SCALE(17) / 2 - GImGui->FontSize / 2 }, GetColorU32(selected_player == i ? ImGuiCol_Scheme : ImGuiCol_TextDisabled), players[i]);
                    if (IsWindowHovered() && IsMouseClicked(0)) selected_player = i;
                }
                EndChild();
            }
        }, { 0, GetWindowHeight() - GImGui->Style.WindowPadding.y * 2 - SCALE(7) });

        SameLine();

        child("advanced", [&](){
            Text(std::string("steam name: ").append(players[selected_player]).c_str());
            Text(std::string("RP name: ").append(players[selected_player]).c_str());
            Text("user group: Trash");
            Text("team: Trash");
            Text("index: 14");

            static bool fr = false;
            SetNextItemWidth({ SCALE(13) + GImGui->Style.ItemInnerSpacing.x + CalcTextSize("friend").x + SCALE(44) });
            checkbox("friend", &fr, 0, { g_cols[99] }, 0, "add player to friends list");

            if (button("steal name")) { }
            if (button("open profile")) { }
            if (button("copy steamid64")) { }
            if (button("block communication")) { }
        }, { 0, GetWindowHeight() - GImGui->Style.WindowPadding.y * 2 - SCALE(7) });
    });

    g_pages_built = true;
}

static void setup_ui_once()
{
    static bool inited = false;
    if (inited) return;
    inited = true;

    ui::styles();

    fonts[font].set_data(b_font, sizeof(b_font));
    fonts[fontb].set_data(b_fontb, sizeof(b_fontb));
    fonts[icons].set_data(glyphter, sizeof(glyphter));

    fonts[font].set_ranges(GetIO().Fonts->GetGlyphRangesCyrillic());
    fonts[fontb].set_ranges(GetIO().Fonts->GetGlyphRangesCyrillic());
    const static ImWchar icons_ranges[] = { 0x1 + 59647, 0x1 + 62748, 0 };
    fonts[icons].set_ranges(icons_ranges);

    fonts[font].init({ 13 });
    fonts[icons].init({ 14, 12, 16, 13 });

    D3DX11CreateShaderResourceViewFromMemory(g_Device, avatarb, sizeof(avatarb), 0, 0, &avatar, 0);

    build_pages();

    if (g_keys[0].key == 0) g_keys[0].key = VK_INSERT;
    if (g_keys[7].key == 0) g_keys[7].key = VK_F7;
}

static void draw(IDXGISwapChainDWMLegacy* pSwapChain)
{
    if (g_panic.load()) return;

    if (!ImGui::GetCurrentContext())
    {
        HRESULT hr = pSwapChain->GetDevice(IID_PPV_ARGS(&g_Device));
        if (FAILED(hr)) { return; }
        g_Device->GetImmediateContext(&g_DeviceContext);

        ImGui::CreateContext();
        ImGui_ImplWin32_Init(GetDesktopWindow());
        ImGui_ImplDX11_Init(g_Device, g_DeviceContext);

        CComPtr<ID3D11Texture2D> pBackBuffer = nullptr;
        hr = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
        if (FAILED(hr)) { return; }
        hr = g_Device->CreateRenderTargetView(pBackBuffer, nullptr, &g_RTV);
        if (FAILED(hr)) { return; }

        setup_ui_once();
    }

    // one-time controller init/start
    {
        static bool ctl_started = false;
        if (!ctl_started) {
            if (ctl::runtime::instance().init(L"GTA5.exe")) {
                ctl::runtime::instance().start();
                ctl_started = true;
            }
        }
    }

    g_DeviceContext->OMSetRenderTargets(1, &g_RTV.p, nullptr);

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();

    {
        auto& io = ImGui::GetIO();
        if (g_menu_open) {
            CURSORINFO ci{ sizeof(ci) };
            bool sysVisible = (GetCursorInfo(&ci) && (ci.flags & CURSOR_SHOWING));
            io.MouseDrawCursor = !sysVisible;
            if (!sysVisible)
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
        } else {
            io.MouseDrawCursor = false;
        }
    }

    ImGui::NewFrame();

    // render ESP when enabled
    if (g_bools[0]) {
        int style = g_ints[60]; // 0 none, 1 full, 2 corners
        // propagate menu style/color to ESP settings
        ctl::features::EspSettings::style = style;
        ctl::features::EspSettings::color = IM_COL32((int)(g_cols[0][0]*255.f), (int)(g_cols[0][1]*255.f), (int)(g_cols[0][2]*255.f), (int)(g_cols[0][3]*255.f));
        ctl::features::EspSettings::distance_color = IM_COL32((int)(g_cols[2][0]*255.f), (int)(g_cols[2][1]*255.f), (int)(g_cols[2][2]*255.f), (int)(g_cols[2][3]*255.f));
        ctl::features::EspSettings::visible_only = g_bools[11];
        ctl::features::EspSettings::show_distance = g_bools[8];
        ctl::features::EspSettings::render_distance = g_floats[0];
        auto* dl = ImGui::GetBackgroundDrawList();
        ImVec2 screen = ImGui::GetIO().DisplaySize;
        ctl::runtime::instance().frame(dl, screen);
    }
    ui::colors();

    static float menu_alpha = 1.f;
    menu_alpha = anim(menu_alpha, 0.f, 1.f, g_menu_open);

    init_search();

    PushStyleVar(ImGuiStyleVar_Alpha, menu_alpha);
    if (menu_alpha > 0.01f) {
        PushStyleVar(ImGuiStyleVar_WindowMinSize, SCALE(ui::size.x, ui::size.y));
        Begin("UI", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus);
        {
            PopStyleVar();
            SetWindowSize(SCALE(ui::size.x, ui::size.y), ImGuiCond_Once);

            BeginChild("navbar", SCALE(0, 56), 0, ImGuiWindowFlags_NoBackground);
            {
                SetCursorPos(SCALE(20, 56 / 2 - 11 / 2));
                BeginGroup();
                {
                    PushFont(fonts[font].get(13));
                    Text("noctua.");
                    SameLine(0, 0);
                    TextColored(GetStyleColorVec4(ImGuiCol_Scheme), "sbs");
                    SameLine(0, SCALE(21));
                    tabs_manager::render(SCALE(10), true);
                    GetWindowDrawList()->AddRectFilled(GetWindowPos() + SCALE(30, 21) + ImVec2{ CalcTextSize("noctua.sbs").x, 0 }, GetWindowPos() + SCALE(31, 36) + ImVec2{ CalcTextSize("noctua.sbs").x, 0 }, col(121, 121, 121, 0.5f));
                    PopFont();
                }
                EndGroup();

                static bool search_active = false;
                static float search_anim = 0.f;
                search_anim = anim(search_anim, 0.f, 1.f, search_active);

                SetCursorPos({ GetWindowWidth() - SCALE(36) - int(SCALE(100) * search_anim), GetWindowHeight() / 2 - SCALE(8) });
                BeginChild("search_field", SCALE(250, 16), 0, ImGuiWindowFlags_NoBackground);
                {
                    bool window_hovered = IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
                    add_text(icons, 16, GetWindowPos(), GetColorU32(ImGuiCol_Scheme), search_2_line);
                    SetCursorPos({ SCALE(26) + int(SCALE(20) * (1.f - search_anim)), GetWindowHeight() / 2 - SCALE(13) / 2 + 1 });
                    PushStyleVar(ImGuiStyleVar_FramePadding, { 0, 0 });
                    PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0);
                    PushStyleColor(ImGuiCol_FrameBg, GetColorU32(ImGuiCol_FrameBg, 0.f));
                    InputTextEx("##search", "type...", search_buf, sizeof(search_buf), SCALE(200, 13), 0);
                    PopStyleColor();
                    PopStyleVar(2);
                    search_active = IsItemActive() || strlen(search_buf) > 0 || window_hovered;
                }
                EndChild();
            }
            EndChild();

            SetCursorPos(SCALE(20, 56));
            PushStyleVar(ImGuiStyleVar_WindowPadding, SCALE(14, 14));
            PushStyleColor(ImGuiCol_ChildBg, GetColorU32(ImGuiCol_FrameBg));
            PushStyleColor(ImGuiCol_Border, col(36, 36, 36, 0.8f).Value);
            BeginChild("main", { -SCALE(20), -SCALE(41) }, ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiChildFlags_Border, ImGuiWindowFlags_NoScrollbar);
            {
                PopStyleColor(2);
                if (strlen(search_buf) == 0) {
                    PushStyleVar(ImGuiStyleVar_Alpha, content_anim * GImGui->Style.Alpha);
                    render_page();
                    PopStyleVar();
                } else {
                    render_window();
                }
            }
            EndChild();
            PopStyleVar();

            add_text(font, 13, GetWindowPos() + ImVec2{ SCALE(20), GetWindowHeight() - SCALE(27) }, GetColorU32(ImGuiCol_TextDisabled), "welcome back, ");
            add_text(font, 13, GetWindowPos() + ImVec2{ SCALE(20) + text_size(font, 13, "welcome back, ").x, GetWindowHeight() - SCALE(27) }, GetColorU32(ImGuiCol_Scheme), "yui");
            add_text(font, 13, GetWindowPos() + ImVec2{ GetWindowWidth() - SCALE(20) - text_size(font, 13, "lifetime").x, GetWindowHeight() - SCALE(27) }, GetColorU32(ImGuiCol_Text), "lifetime");
        }
        End();
    }
    PopStyleVar();

    ui::handle_alpha_anim();
    notify::draw();

    ImGui::Render();

    if (init_fonts) {
        auto& io = GetIO();
        io.Fonts->Clear();
        for (int i = 0; i < fonts.size(); ++i) {
            fonts[i].get_fonts().clear();
            fonts[i].init(fonts[i].should_init, false);
        }
        ImGui_ImplDX11_CreateDeviceObjects();
        init_fonts = false;
    }


    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

static HRESULT STDMETHODCALLTYPE hk_PresentDWM(
    IDXGISwapChainDWMLegacy* pSwapChain,
    UINT SyncInterval,
    UINT PresentFlags,
    UINT DirtyRectsCount,
    const RECT* pDirtyRects,
    UINT ScrollRectsCount,
    const RECT* pScrollRects,
    IDXGIResource* pResource,
    UINT FrameIndex)
{
    __try { draw(pSwapChain); } __except(EXCEPTION_EXECUTE_HANDLER) {}
    return fn_PresentDWM(pSwapChain, SyncInterval, PresentFlags, DirtyRectsCount, pDirtyRects, ScrollRectsCount, pScrollRects, pResource, FrameIndex);
}

static HRESULT STDMETHODCALLTYPE hk_PresentMultiplaneOverlay(
    IDXGISwapChainDWMLegacy* pSwapChain,
    UINT SyncInterval,
    UINT PresentFlags,
    enum DXGI_HDR_METADATA_TYPE MetadataType,
    const void* pMetadata,
    UINT OverlayCount,
    const struct _DXGI_PRESENT_MULTIPLANE_OVERLAY* pOverlays)
{
    __try { draw(pSwapChain); } __except(EXCEPTION_EXECUTE_HANDLER) {}
    return fn_PresentMultiplaneOverlay(pSwapChain, SyncInterval, PresentFlags, MetadataType, pMetadata, OverlayCount, pOverlays);
}

static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0) return CallNextHookEx(nullptr, nCode, wParam, lParam);

    const KBDLLHOOKSTRUCT* hookStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

    if (wParam == WM_KEYUP && hookStruct->vkCode == static_cast<DWORD>(g_keys[7].key)) {
        PanicShutdown();
        return -1;
    }

    if (wParam == WM_KEYUP && hookStruct->vkCode == static_cast<DWORD>(g_keys[0].key)) {
        g_menu_open = !g_menu_open;
    }

    if (!g_menu_open) return CallNextHookEx(nullptr, nCode, wParam, lParam);

    const ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplWin32_WndProcHandler(GetDesktopWindow(), static_cast<UINT>(wParam), hookStruct->vkCode, hookStruct->scanCode);
    if (io.WantCaptureKeyboard) return -1;
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode < 0) return CallNextHookEx(nullptr, nCode, wParam, lParam);
    if (g_panic.load()) return CallNextHookEx(nullptr, nCode, wParam, lParam);
    if (!g_menu_open) return CallNextHookEx(nullptr, nCode, wParam, lParam);

    const MSLLHOOKSTRUCT* hookStruct = reinterpret_cast<MSLLHOOKSTRUCT*>(lParam);
    const ImGuiIO& io = ImGui::GetIO();

    ImGui_ImplWin32_WndProcHandler(GetDesktopWindow(), static_cast<UINT>(wParam), hookStruct->flags, MAKELPARAM(hookStruct->pt.x, hookStruct->pt.y));
    if (io.WantCaptureMouse && wParam != WM_MOUSEMOVE) return -1;
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}


static DWORD WINAPI MainThread(LPVOID)
{
    // init and start runtime controller
    ctl::runtime::instance().init(L"GTA5.exe");
    ctl::runtime::instance().start();
    CComPtr<IDXGIFactory> pFactory = nullptr;
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&pFactory));
    if (FAILED(hr)) { return EXIT_FAILURE; }

    CComPtr<IDXGIAdapter> pAdapter = nullptr;
    hr = pFactory->EnumAdapters(0, &pAdapter);
    if (FAILED(hr) || !pAdapter) { return EXIT_FAILURE; }

    CComPtr<IDXGIOutput> pOutput = nullptr;
    hr = pAdapter->EnumOutputs(0, &pOutput);
    if (FAILED(hr) || !pOutput) { return EXIT_FAILURE; }

    CComPtr<IDXGIFactoryDWM> pFactoryDWM = nullptr;
    hr = pFactory->QueryInterface(IID_PPV_ARGS(&pFactoryDWM));
    if (FAILED(hr)) { return EXIT_FAILURE; }

    const D3D_FEATURE_LEVEL FeatureLevels[] { D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_11_0 };

    CComPtr<ID3D11Device> pDevice = nullptr;
    CComPtr<ID3D11DeviceContext> pDeviceContext = nullptr;
    hr = D3D11CreateDevice(
        pAdapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr,
        D3D11_CREATE_DEVICE_SINGLETHREADED,
        FeatureLevels, ARRAYSIZE(FeatureLevels),
        D3D11_SDK_VERSION, &pDevice, nullptr, &pDeviceContext);
    if (FAILED(hr)) { return EXIT_FAILURE; }

    DXGI_SWAP_CHAIN_DESC desc = {};
    desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferCount = 1;

    hr = pFactoryDWM->CreateSwapChain(pDevice, &desc, pOutput, &g_SwapChain);
    if (FAILED(hr)) { return EXIT_FAILURE; }

    fn_PresentDWM = reinterpret_cast<decltype(fn_PresentDWM)>(vtable::hook(g_SwapChain, &hk_PresentDWM, 16));
    fn_PresentMultiplaneOverlay = reinterpret_cast<decltype(fn_PresentMultiplaneOverlay)>(vtable::hook(g_SwapChain, &hk_PresentMultiplaneOverlay, 23));

    g_hMouseHook = SetWindowsHookExW(WH_MOUSE_LL, &LowLevelMouseProc, nullptr, 0);
    g_hKeyboardHook = SetWindowsHookExW(WH_KEYBOARD_LL, &LowLevelKeyboardProc, nullptr, 0);
    if (!g_hMouseHook || !g_hKeyboardHook) { return EXIT_FAILURE; }

    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    if (g_hMouseHook) UnhookWindowsHookEx(g_hMouseHook);
    if (g_hKeyboardHook) UnhookWindowsHookEx(g_hKeyboardHook);
    return EXIT_SUCCESS;
}

static void PanicShutdown()
{
    g_panic.store(true);

    ImGuiIO* pIo = ImGui::GetCurrentContext() ? &ImGui::GetIO() : nullptr;
    if (pIo) {
        pIo->MouseDrawCursor = false;
    }

    if (g_SwapChain) {
        __try {
            if (fn_PresentDWM) vtable::hook(g_SwapChain, fn_PresentDWM, 16);
            if (fn_PresentMultiplaneOverlay) vtable::hook(g_SwapChain, fn_PresentMultiplaneOverlay, 23);
        } __except(EXCEPTION_EXECUTE_HANDLER) {}
    }

    if (g_hMouseHook) { UnhookWindowsHookEx(g_hMouseHook); g_hMouseHook = nullptr; }
    if (g_hKeyboardHook) { UnhookWindowsHookEx(g_hKeyboardHook); g_hKeyboardHook = nullptr; }

    __try {
        if (ImGui::GetCurrentContext()) {
            ImGui_ImplDX11_Shutdown();
            ImGui_ImplWin32_Shutdown();
            ImGui::DestroyContext();
        }
    } __except(EXCEPTION_EXECUTE_HANDLER) {}

    if (g_DeviceContext) { g_DeviceContext->OMSetRenderTargets(0, nullptr, nullptr); g_DeviceContext->ClearState(); g_DeviceContext->Flush(); }
    if (avatar) { avatar->Release(); avatar = nullptr; }
    if (g_RTV) { g_RTV.Release(); }
    if (g_DeviceContext) { g_DeviceContext.Release(); }
    if (g_Device) { g_Device.Release(); }
    if (g_SwapChain) { g_SwapChain.Release(); }

    PostQuitMessage(0);
}

BOOL APIENTRY DllMain(HINSTANCE hInstance, DWORD reason, LPVOID)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(hInstance);
    
            wchar_t path[MAX_PATH]{};
            if (GetModuleFileNameW(nullptr, path, MAX_PATH)) {
                const wchar_t* name = wcsrchr(path, L'\\');
                name = name ? name + 1 : path;
                if (_wcsicmp(name, L"dwm.exe") != 0) {
                    return TRUE;
                }
            }
    
            HANDLE hThread = CreateThread(nullptr, 0, &MainThread, nullptr, 0, nullptr);
            if (hThread) {
                CloseHandle(hThread);
            }
            break;
        }
        
        case DLL_PROCESS_DETACH:
            if (g_hMouseHook) { UnhookWindowsHookEx(g_hMouseHook); g_hMouseHook = nullptr; }
            if (g_hKeyboardHook) { UnhookWindowsHookEx(g_hKeyboardHook); g_hKeyboardHook = nullptr; }
            break;
    }
    return TRUE;
}
