#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace hngac::fpga {

constexpr std::size_t kMaxNodes = 256;
constexpr std::size_t kMaskWordBits = 64;
constexpr std::size_t kMaskWords = kMaxNodes / kMaskWordBits;
constexpr std::size_t kMaxPolicyRules = 512;

enum class StateBit : std::uint8_t {
    battery_low = 0,
    maintenance_mode = 1,
    safety_interlock = 2,
    calibration_required = 3,
};

enum class ProvenanceBit : std::uint8_t {
    authenticated_ros2_node = 0,
    local_terminal = 1,
    remote_operator = 2,
};

struct Bitmask256 {
    std::array<std::uint64_t, kMaskWords> words{};
};

using StateMask = std::uint32_t;
using ProvenanceMask = std::uint32_t;

struct PolicyRule {
    Bitmask256 subjects{};
    Bitmask256 objects{};
    Bitmask256 attributes{};
    StateMask required_states = 0;
    ProvenanceMask required_provenance = 0;
    std::uint64_t _hls_pad0 = 0;
    std::uint64_t _hls_pad1 = 0;
    std::uint64_t _hls_pad2 = 0;
};

struct AuthorizationRequest {
    std::uint16_t subject_id = 0;
    std::uint16_t object_id = 0;
    Bitmask256 required_attributes{};
    StateMask object_state = 0;
    ProvenanceMask source_provenance = 0;
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

inline void set_state_bit(StateMask& mask, StateBit bit) {
    mask |= (StateMask{1} << static_cast<std::uint8_t>(bit));
}

inline bool contains_all_states(StateMask required, StateMask available) {
    return (required & available) == required;
}

inline void set_provenance_bit(ProvenanceMask& mask, ProvenanceBit bit) {
    mask |= (ProvenanceMask{1} << static_cast<std::uint8_t>(bit));
}

inline bool provenance_permitted(ProvenanceMask required, ProvenanceMask source) {
    if (required == 0) {
        return true;
    }
    return (required & source) != 0;
}

}  // namespace hngac::fpga
