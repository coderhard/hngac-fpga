#include <cstdio>

#include "hngac_kernel.hpp"

namespace {

using hngac::fpga::Bitmask256;
using hngac::fpga::PolicyRule;
using hngac::fpga::AuthorizationRequest;
// hngac_authorize is at file scope (not in hngac::fpga) for Vitis 2025.2 cosim.
using hngac::fpga::kMaxPolicyRules;
using hngac::fpga::set_bit;

void add_single_rule(
    PolicyRule policy[kMaxPolicyRules],
    std::uint16_t index,
    std::uint16_t subject,
    std::uint16_t object,
    std::uint16_t attribute) {
    set_bit(policy[index].subjects, subject);
    set_bit(policy[index].objects, object);
    set_bit(policy[index].attributes, attribute);
}

AuthorizationRequest make_request(
    std::uint16_t subject,
    std::uint16_t object,
    std::uint16_t attribute) {
    AuthorizationRequest request{};
    request.subject_id = subject;
    request.object_id = object;
    set_bit(request.required_attributes, attribute);
    return request;
}

}  // namespace

int main() {
    PolicyRule policy[kMaxPolicyRules] = {};
    add_single_rule(policy, 0, 1, 2, 5);
    add_single_rule(policy, 1, 7, 9, 11);
    add_single_rule(policy, 2, 4, 6, 13);

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
        "subject 1 object 2 attr 5 allowed",
        hngac_authorize(policy, 3, make_request(1, 2, 5)));
    check(
        "unknown subject denied",
        !hngac_authorize(policy, 3, make_request(99, 2, 5)));
    check(
        "wrong object denied",
        !hngac_authorize(policy, 3, make_request(1, 3, 5)));
    check(
        "wrong attribute denied",
        !hngac_authorize(policy, 3, make_request(1, 2, 7)));

    check(
        "second rule allowed",
        hngac_authorize(policy, 3, make_request(4, 6, 13)));
    check(
        "third rule allowed",
        hngac_authorize(policy, 3, make_request(7, 9, 11)));

    AuthorizationRequest empty_attr_request{};
    empty_attr_request.subject_id = 1;
    empty_attr_request.object_id = 2;
    check("empty attrs allowed by design", hngac_authorize(policy, 3, empty_attr_request));

    AuthorizationRequest out_of_range_request = make_request(1, 2, 5);
    out_of_range_request.subject_id = 256;
    check("out of range subject denied", !hngac_authorize(policy, 3, out_of_range_request));

    // --- out-of-range object_id ---
    AuthorizationRequest out_of_range_obj = make_request(1, 2, 5);
    out_of_range_obj.object_id = 256;
    check("out of range object denied", !hngac_authorize(policy, 3, out_of_range_obj));

    // --- empty policy (rule_count = 0) always denies ---
    check("empty policy denies", !hngac_authorize(policy, 0, make_request(1, 2, 5)));

    // --- first-match-wins: second rule matches when first does not ---
    check("second rule match allowed",
        hngac_authorize(policy, 3, make_request(7, 9, 11)));
    check("first rule does not match rule-1 request",
        !hngac_authorize(policy, 1, make_request(7, 9, 11)));

    // --- multi-bit attribute required by rule ---
    PolicyRule policy_multiattr[kMaxPolicyRules] = {};
    set_bit(policy_multiattr[0].subjects, 10);
    set_bit(policy_multiattr[0].objects, 20);
    set_bit(policy_multiattr[0].attributes, 2);
    set_bit(policy_multiattr[0].attributes, 5);
    {
        AuthorizationRequest both_attrs = make_request(10, 20, 2);
        set_bit(both_attrs.required_attributes, 5);
        check("multi-attr both bits allowed", hngac_authorize(policy_multiattr, 1, both_attrs));

        AuthorizationRequest one_attr = make_request(10, 20, 2);
        check("multi-attr subset request allowed", hngac_authorize(policy_multiattr, 1, one_attr));

        AuthorizationRequest wrong_attr = make_request(10, 20, 8);
        check("multi-attr wrong single bit denied", !hngac_authorize(policy_multiattr, 1, wrong_attr));

        AuthorizationRequest extra_attr = make_request(10, 20, 2);
        set_bit(extra_attr.required_attributes, 5);
        set_bit(extra_attr.required_attributes, 9);
        check("multi-attr extra required bit denied", !hngac_authorize(policy_multiattr, 1, extra_attr));
    }

    // --- valid boundary nodes: subject 255 and object 255 (kMaxNodes - 1) ---
    PolicyRule policy_boundary[kMaxPolicyRules] = {};
    set_bit(policy_boundary[0].subjects, 255);
    set_bit(policy_boundary[0].objects, 255);
    set_bit(policy_boundary[0].attributes, 0);
    check("subject and object at 255 allowed",
        hngac_authorize(policy_boundary, 1, make_request(255, 255, 0)));
    {
        AuthorizationRequest req_255 = make_request(255, 255, 0);
        req_255.object_id = 254;
        check("object 254 not in boundary rule denied",
            !hngac_authorize(policy_boundary, 1, req_255));
    }

    // --- rule with multiple subjects: both should be authorized independently ---
    PolicyRule policy_multisub[kMaxPolicyRules] = {};
    set_bit(policy_multisub[0].subjects, 8);
    set_bit(policy_multisub[0].subjects, 12);
    set_bit(policy_multisub[0].objects, 30);
    set_bit(policy_multisub[0].attributes, 4);
    check("multi-subject rule: first subject allowed",
        hngac_authorize(policy_multisub, 1, make_request(8, 30, 4)));
    check("multi-subject rule: second subject allowed",
        hngac_authorize(policy_multisub, 1, make_request(12, 30, 4)));
    check("multi-subject rule: unlisted subject denied",
        !hngac_authorize(policy_multisub, 1, make_request(9, 30, 4)));

    // --- rule_count clamping: passing count > kMaxPolicyRules is safe ---
    check("rule_count clamped: still finds matching rule",
        hngac_authorize(policy, static_cast<std::uint16_t>(kMaxPolicyRules + 1),
            make_request(1, 2, 5)));

    // --- worst-case: 512 non-matching rules to stress the pipelined scan ---
    PolicyRule policy_worstcase[kMaxPolicyRules] = {};
    for (std::uint16_t i = 0; i < kMaxPolicyRules; ++i) {
        set_bit(policy_worstcase[i].subjects, 100);
        set_bit(policy_worstcase[i].objects, 100);
        set_bit(policy_worstcase[i].attributes, 0);
    }
    check("worst-case 512-rule full scan denies",
        !hngac_authorize(policy_worstcase, kMaxPolicyRules,
            make_request(0, 0, 0)));

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
