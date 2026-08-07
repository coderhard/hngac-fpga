#pragma once

#include <cstdint>

#include "hngac_types.hpp"

// File-scope HLS top. Vitis 2025.2 co-sim has previously failed to resolve
// namespaced top-function stubs, so keep this wrapper at global scope.
bool hngac_authorize(
    const hngac::fpga::PolicyRule policy[hngac::fpga::kMaxPolicyRules],
    std::uint16_t rule_count,
    const hngac::fpga::AuthorizationRequest& request);
