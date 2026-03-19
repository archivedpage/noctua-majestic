

 external overlay toolkit for majestic. using kernel mode driver with a
 DirectX 11 overlay injecting into dwm.exe.
  ---

  ## Building

  **Requirements:**
  - Visual Studio 2019+
  - Windows SDK & WDK (for the IOCTL driver)
  - DirectX 11 SDK

  ---

  ## Dependencies

  - [Dear ImGui](https://github.com/ocornut/imgui) — imgui
  - [FreeType](https://freetype.org/) — font
  - [nlohmann/json](https://github.com/nlohmann/json) — configuration serialization
  -  vtablehook utility (`thirdparty/vtablehook/`)
