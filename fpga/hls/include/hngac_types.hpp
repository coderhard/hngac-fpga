#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace hngac::fpga {

constexpr std::size_t kMaxNodes = 256;
constexpr std::size_t kMaskWordBits = 64;
constexpr std::size_t kMaskWords = kMaxNodes / kMaskWordBits;
constexpr std::size_t kMaxPolicyRules = 512;

struct Bitmask256 {
    std::array<std::uint64_t, kMaskWords> words{};
};

struct PolicyRule {
    Bitmask256 subjects{};
    Bitmask256 objects{};
    Bitmask256 attributes{};
};

inline void set_bit(Bitmask256& mask, std::size_t index) {
    if (index >= kMaxNodes) {
        return;
    }
    const std::size_t word = index / kMaskWordBits;
    const std::size_t bit = index % kMaskWordBits;
    mask.words[word] |= (std::uint64_t{1} << bit);
}

inline bool test_bit(const Bitmask256& mask, std::size_t index) {
    if (index >= kMaxNodes) {
        return false;
    }
    const std::size_t word = index / kMaskWordBits;
    const std::size_t bit = index % kMaskWordBits;
    return (mask.words[word] & (std::uint64_t{1} << bit)) != 0;
}

inline bool contains_all(const Bitmask256& required, const Bitmask256& available) {
    for (std::size_t i = 0; i < kMaskWords; ++i) {
        if ((required.words[i] & available.words[i]) != required.words[i]) {
            return false;
        }
    }
    return true;
}

}  // namespace hngac::fpga
