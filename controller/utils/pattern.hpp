#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace ctl::utils {

struct Pattern {
    std::vector<uint8_t> bytes;
    std::vector<bool>    mask; // false = wildcard
};

inline Pattern parse_ida_pattern(const std::string& s) {
    Pattern p{};
    p.bytes.reserve(s.size()/3 + 1);
    p.mask.reserve(s.size()/3 + 1);
    for (size_t i = 0; i < s.size();) {
        while (i < s.size() && s[i] == ' ') ++i;
        if (i >= s.size()) break;
        if (s[i] == '?') {
            ++i; if (i < s.size() && s[i] == '?') ++i;
            p.bytes.push_back(0);
            p.mask.push_back(false);
        } else {
            auto hex = [](char c)->int{ if (c>='0'&&c<='9') return c-'0'; if(c>='A'&&c<='F') return 10+c-'A'; if(c>='a'&&c<='f') return 10+c-'a'; return -1; };
            if (i+1 >= s.size()) break;
            int hi = hex(s[i]); int lo = hex(s[i+1]);
            if (hi<0 || lo<0) break;
            p.bytes.push_back(uint8_t((hi<<4)|lo));
            p.mask.push_back(true);
            i += 2;
        }
        while (i < s.size() && s[i] == ' ') ++i;
    }
    return p;
}

inline const uint8_t* scan_buf(const uint8_t* buf, size_t len, const Pattern& p) {
    if (p.bytes.empty() || len < p.bytes.size()) return nullptr;
    const size_t n = p.bytes.size();
    for (size_t i = 0; i + n <= len; ++i) {
        size_t j = 0;
        for (; j < n; ++j) {
            if (p.mask[j] && buf[i+j] != p.bytes[j]) break;
        }
        if (j == n) return buf + i;
    }
    return nullptr;
}

} // namespace ctl::utils
