#include "hngac_kernel.hpp"

namespace hngac::fpga {

bool hngac_authorize(
    const PolicyRule policy[kMaxPolicyRules],
    std::uint16_t rule_count,
    const AuthorizationRequest& request) {
    if (request.subject_id >= kMaxNodes || request.object_id >= kMaxNodes) {
        return false;
    }

    const std::uint16_t bounded_rule_count =
        rule_count > kMaxPolicyRules ? static_cast<std::uint16_t>(kMaxPolicyRules) : rule_count;

    for (std::uint16_t i = 0; i < bounded_rule_count; ++i) {
        const PolicyRule& rule = policy[i];

        if (!test_bit(rule.subjects, request.subject_id)) {
            continue;
        }
        if (!test_bit(rule.objects, request.object_id)) {
            continue;
        }
        if (!contains_all(request.required_attributes, rule.attributes)) {
            continue;
        }
        if (!contains_all_states(rule.required_states, request.object_state)) {
            continue;
        }
        return true;
    }

    return false;
}

}  // namespace hngac::fpga
