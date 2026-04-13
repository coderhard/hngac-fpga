#pragma once

#include <cstdint>

#include "hngac_types.hpp"

namespace hngac::fpga {

bool hngac_authorize(
    const PolicyRule policy[kMaxPolicyRules],
    std::uint16_t rule_count,
    const AuthorizationRequest& request);

}  // namespace hngac::fpga
