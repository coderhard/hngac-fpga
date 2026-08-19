#include "hngac_kernel.hpp"

using namespace hngac::fpga;

// File scope, not in hngac::fpga: Vitis 2025.2 cosim breaks on namespaced tops.
bool hngac_authorize(
    const PolicyRule policy[kMaxPolicyRules],
    std::uint16_t rule_count,
    const AuthorizationRequest& request) {
#pragma HLS INTERFACE s_axilite port=return
#pragma HLS INTERFACE s_axilite port=rule_count
#pragma HLS INTERFACE s_axilite port=request
#pragma HLS INTERFACE bram port=policy

    if (request.subject_id >= kMaxNodes || request.object_id >= kMaxNodes) {
        return false;
    }

    const std::uint16_t bounded_rule_count =
        rule_count > kMaxPolicyRules ? static_cast<std::uint16_t>(kMaxPolicyRules) : rule_count;

    // Pre-compute word indices and bit masks
    const std::size_t subj_word = request.subject_id / kMaskWordBits;
    const std::uint64_t subj_mask = std::uint64_t{1} << (request.subject_id % kMaskWordBits);
    const std::size_t obj_word = request.object_id / kMaskWordBits;
    const std::uint64_t obj_mask = std::uint64_t{1} << (request.object_id % kMaskWordBits);

    // Inline check function (avoids function call overhead in unrolled loop)
    auto check_rule = [&](const PolicyRule& rule) -> bool {
#pragma HLS INLINE
        bool subj_ok = (rule.subjects.words[subj_word] & subj_mask) != 0;
        bool obj_ok  = (rule.objects.words[obj_word] & obj_mask) != 0;
        bool attr_ok =
            ((request.required_attributes.words[0] & rule.attributes.words[0]) == request.required_attributes.words[0]) &&
            ((request.required_attributes.words[1] & rule.attributes.words[1]) == request.required_attributes.words[1]) &&
            ((request.required_attributes.words[2] & rule.attributes.words[2]) == request.required_attributes.words[2]) &&
            ((request.required_attributes.words[3] & rule.attributes.words[3]) == request.required_attributes.words[3]);
        return subj_ok && obj_ok && attr_ok;
    };

    // Two-stage pipeline with partial unroll (2 rules per iteration).
    // Each iteration reads 2 rules from BRAM and checks both.
    // HLS ARRAY_PARTITION creates dual-port access for parallel reads.
    // This halves the iteration count: 512 rules → 256 iterations.

    // Pad rule_count to even
    const std::uint16_t pairs = (bounded_rule_count + 1) / 2;

    PolicyRule rule_a_reg, rule_b_reg;
    bool stage1_valid = false;
    bool found = false;

    for (std::uint16_t p = 0; p < pairs + 1; ++p) {
#pragma HLS PIPELINE II=1
#pragma HLS LOOP_TRIPCOUNT min=0 max=257 avg=129

        // ---- Stage 2: check PREVIOUS pair (from registers) ----
        if (stage1_valid && !found) {
            if (check_rule(rule_a_reg)) {
                found = true;
            } else if (check_rule(rule_b_reg)) {
                found = true;
            }
        }

        // ---- Stage 1: read NEXT pair into registers ----
        if (p < pairs) {
            std::uint16_t idx = p * 2;
            rule_a_reg = policy[idx];
            if (idx + 1 < bounded_rule_count) {
                rule_b_reg = policy[idx + 1];
            } else {
                rule_b_reg = PolicyRule{};
            }
            stage1_valid = true;
        } else {
            stage1_valid = false;
        }
    }

    return found;
}
