#include "hngac_kernel.hpp"

namespace hngac::fpga {

bool hngac_authorize(
    const PolicyRule policy[kMaxPolicyRules],
    std::uint16_t rule_count,
    std::uint16_t subject_id,
    std::uint16_t object_id,
    const Bitmask256& required_attributes) {
    if (subject_id >= kMaxNodes || object_id >= kMaxNodes) {
        return false;
    }

    const std::uint16_t bounded_rule_count =
        rule_count > kMaxPolicyRules ? static_cast<std::uint16_t>(kMaxPolicyRules) : rule_count;

    for (std::uint16_t i = 0; i < bounded_rule_count; ++i) {
        const PolicyRule& rule = policy[i];

        if (!test_bit(rule.subjects, subject_id)) {
            continue;
        }
        if (!test_bit(rule.objects, object_id)) {
            continue;
        }
        if (!contains_all(required_attributes, rule.attributes)) {
            continue;
        }
        return true;
    }

    return false;
}

}  // namespace hngac::fpga
