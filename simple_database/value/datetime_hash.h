#pragma once
#include "datetime.h"

namespace std {
    template<>
    struct hash<datetime> {
        size_t operator()(const datetime& t) const noexcept {
            size_t h = std::hash<date>{}(t.dt);
            h ^= std::hash<unsigned int>{}(t.hr) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<unsigned int>{}(t.min) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<unsigned int>{}(t.sec) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
}