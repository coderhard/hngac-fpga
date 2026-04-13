#include <cstdio>

#include "hngac_kernel.hpp"

namespace {

using hngac::fpga::Bitmask256;
using hngac::fpga::PolicyRule;
using hngac::fpga::AuthorizationRequest;
using hngac::fpga::StateBit;
using hngac::fpga::hngac_authorize;
using hngac::fpga::kMaxPolicyRules;
using hngac::fpga::set_bit;
using hngac::fpga::set_state_bit;

void add_single_rule(
    PolicyRule policy[kMaxPolicyRules],
    std::uint16_t index,
    std::uint16_t subject,
    std::uint16_t object,
    std::uint16_t attribute,
    std::initializer_list<StateBit> states = {}) {
    set_bit(policy[index].subjects, subject);
    set_bit(policy[index].objects, object);
    set_bit(policy[index].attributes, attribute);
    for (StateBit bit : states) {
        set_state_bit(policy[index].required_states, bit);
    }
}

AuthorizationRequest make_request(
    std::uint16_t subject,
    std::uint16_t object,
    std::uint16_t attribute,
    std::initializer_list<StateBit> states = {}) {
    AuthorizationRequest request{};
    request.subject_id = subject;
    request.object_id = object;
    set_bit(request.required_attributes, attribute);
    for (StateBit bit : states) {
        set_state_bit(request.object_state, bit);
    }
    return request;
}

}  // namespace

int main() {
    PolicyRule policy[kMaxPolicyRules] = {};
    add_single_rule(policy, 0, 1, 2, 5, {StateBit::battery_low});
    add_single_rule(policy, 1, 7, 9, 11, {StateBit::safety_interlock, StateBit::calibration_required});
    add_single_rule(policy, 2, 4, 6, 13, {StateBit::maintenance_mode});

    int passed = 0;
    int failed = 0;

    auto check = [&](const char* name, bool condition) {
        if (condition) {
            std::printf("PASS  %s\n", name);
            ++passed;
        } else {
            std::printf("FAIL  %s\n", name);
            ++failed;
        }
    };

    check(
        "subject 1 object 2 attr 5 battery_low allowed",
        hngac_authorize(policy, 3, make_request(1, 2, 5, {StateBit::battery_low})));
    check(
        "unknown subject denied",
        !hngac_authorize(policy, 3, make_request(99, 2, 5, {StateBit::battery_low})));
    check(
        "wrong object denied",
        !hngac_authorize(policy, 3, make_request(1, 3, 5, {StateBit::battery_low})));
    check(
        "wrong attribute denied",
        !hngac_authorize(policy, 3, make_request(1, 2, 7, {StateBit::battery_low})));
    check(
        "missing required state denied",
        !hngac_authorize(policy, 3, make_request(1, 2, 5)));
    check(
        "maintenance mode rule allowed",
        hngac_authorize(policy, 3, make_request(4, 6, 13, {StateBit::maintenance_mode})));
    check(
        "maintenance mode absent denied",
        !hngac_authorize(policy, 3, make_request(4, 6, 13, {StateBit::battery_low})));
    check(
        "composite state rule allowed",
        hngac_authorize(
            policy,
            3,
            make_request(7, 9, 11, {StateBit::safety_interlock, StateBit::calibration_required})));
    check(
        "partial composite state denied",
        !hngac_authorize(policy, 3, make_request(7, 9, 11, {StateBit::safety_interlock})));

    AuthorizationRequest empty_attr_request{};
    empty_attr_request.subject_id = 1;
    empty_attr_request.object_id = 2;
    set_state_bit(empty_attr_request.object_state, StateBit::battery_low);
    check("empty attrs allowed by design", hngac_authorize(policy, 3, empty_attr_request));

    AuthorizationRequest out_of_range_request = make_request(1, 2, 5, {StateBit::battery_low});
    out_of_range_request.subject_id = 256;
    check("out of range subject denied", !hngac_authorize(policy, 3, out_of_range_request));

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
