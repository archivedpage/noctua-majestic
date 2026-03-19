#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <Windows.h>
#include "mem.hpp"
#include "utils/pattern.hpp"

namespace ctl::utils {

class signature {
    ctl::Mem& mem_;
    uintptr_t base_ = 0;
    size_t size_ = 0;
    std::unique_ptr<uint8_t[]> data_;
    uintptr_t temp_ = 0; // virtual address in target
public:
    explicit signature(ctl::Mem& mem, const std::string& module_name)
        : mem_(mem) {
        // Get main image base (GTA5.exe) via driver
        uintptr_t base = 0;
        if (!mem_.image_base(base) || !base) return;
        base_ = base;
        // Read PE headers to get SizeOfImage
        IMAGE_DOS_HEADER dos{}; IMAGE_NT_HEADERS64 nt{};
        if (!mem_.read(base_, dos) || dos.e_magic != IMAGE_DOS_SIGNATURE) return;
        if (!mem_.read(base_ + dos.e_lfanew, nt) || nt.Signature != IMAGE_NT_SIGNATURE) return;
        size_ = nt.OptionalHeader.SizeOfImage;
        data_ = std::make_unique<uint8_t[]>(size_);
        // Read whole image in chunks
        const size_t chunk = 512 * 1024;
        for (size_t off = 0; off < size_;) {
            size_t to_read = (off + chunk <= size_) ? chunk : (size_ - off);
            mem_.read_buf(base_ + off, data_.get() + off, to_read);
            off += to_read;
        }
        temp_ = base_;
    }

    signature& scan(const std::string& ida_pattern) {
        temp_ = 0;
        auto pat = parse_ida_pattern(ida_pattern);
        if (!data_ || pat.bytes.empty()) return *this;
        if (auto p = scan_buf(data_.get(), size_, pat)) {
            size_t off = size_t(p - data_.get());
            temp_ = base_ + off;
        }
        return *this;
    }

    signature& add(int value) { temp_ += value; return *this; }
    signature& sub(int value) { temp_ -= value; return *this; }

    signature& rip() {
        // read 32-bit rel from target memory at temp_
        int32_t rel = 0;
        if (mem_.read(temp_, rel)) {
            temp_ = temp_ + rel + 4;
        }
        return *this;
    }

    template<typename T>
    T as() const { return (T)temp_; }

    operator uintptr_t() const { return temp_; }
};

} // namespace ctl::utils
