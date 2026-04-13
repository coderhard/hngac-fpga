#pragma once

#include <cstdint>

#include "hngac_types.hpp"

namespace hngac::fpga {

bool hngac_authorize(
    const PolicyRule policy[kMaxPolicyRules],
    std::uint16_t rule_count,
    std::uint16_t subject_id,
    std::uint16_t object_id,
    const Bitmask256& required_attributes);

}  // namespace hngac::fpga
