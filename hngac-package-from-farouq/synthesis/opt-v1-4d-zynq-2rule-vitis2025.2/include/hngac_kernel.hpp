#pragma once

#include <cstdint>

#include "hngac_types.hpp"

// File scope, not in hngac::fpga: Vitis 2025.2 cosim breaks on namespaced tops.
bool hngac_authorize(
    const hngac::fpga::PolicyRule policy[hngac::fpga::kMaxPolicyRules],
    std::uint16_t rule_count,
    const hngac::fpga::AuthorizationRequest& request);
