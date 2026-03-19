#include "driver.hpp"

namespace driver
{
    Driver::Driver() : handle_(create_handle())
    {
    }

    Driver::~Driver()
    {
        CloseHandle(handle_);
        std::wcout << L"Driver handle successfully destroyed.\n";
    }

    bool Driver::is_valid() const
    {
        return handle_ != INVALID_HANDLE_VALUE;
    }

    HANDLE Driver::create_handle()
    {
        auto try_open = [](LPCWSTR path)->HANDLE {
            return CreateFileW(
                path,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                nullptr,
                OPEN_EXISTING,
                0,
                nullptr);
        };

        HANDLE h = try_open(L"\\\\.\\xd");
        if (h == INVALID_HANDLE_VALUE)
        {
            // Fallback to native device path via GLOBALROOT
            h = try_open(L"\\\\?\\GLOBALROOT\\Device\\xd");
        }

        if (h == INVALID_HANDLE_VALUE)
        {
            std::wcerr << L"Failed creating Driver handle. Error: " << GetLastError() << '\n';
        }
        else
        {
            std::wcout << L"Created Driver handle.\n";
        }

        return h;
    }

    bool Driver::attach(const DWORD& process_id) const
    {
        Request driver_request{};
        driver_request.process_id_handle = reinterpret_cast<HANDLE>(process_id);

        return DeviceIoControl(handle_, control_codes::attach, &driver_request, sizeof(driver_request), &driver_request, sizeof(driver_request), nullptr, nullptr);
    }

    bool Driver::get_image_base(std::uintptr_t& out_base) const
    {
        Request driver_request{};
        if (!DeviceIoControl(handle_, control_codes::get_image_base, &driver_request, sizeof(driver_request), &driver_request, sizeof(driver_request), nullptr, nullptr))
            return false;
        out_base = reinterpret_cast<std::uintptr_t>(driver_request.target_address);
        return out_base != 0;
    }
}
