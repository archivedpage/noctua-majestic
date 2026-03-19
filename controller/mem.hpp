#pragma once
#include <cstdint>
#include "driver.hpp"

namespace ctl {

class Mem {
public:
    explicit Mem(driver::Driver& drv) : drv_(drv) {}

    template<typename T>
    bool read(uintptr_t addr, T& out) const {
        return drv_.read_buf(addr, &out, sizeof(T));
    }

    bool read_buf(uintptr_t addr, void* buf, size_t sz) const {
        return drv_.read_buf(addr, buf, sz);
    }

    bool image_base(uintptr_t& out) const { return drv_.get_image_base(out); }

private:
    driver::Driver& drv_;
};

} // namespace ctl
