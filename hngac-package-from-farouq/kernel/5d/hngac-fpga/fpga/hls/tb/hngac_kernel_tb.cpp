#include <cstdio>

#include "hngac_kernel.hpp"

namespace {

using hngac::fpga::Bitmask256;
using hngac::fpga::PolicyRule;
using hngac::fpga::AuthorizationRequest;
using hngac::fpga::ProvenanceBit;
using hngac::fpga::StateBit;
using hngac::fpga::kMaxPolicyRules;
using hngac::fpga::set_bit;
using hngac::fpga::set_provenance_bit;
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

    // --- out-of-range object_id ---
    AuthorizationRequest out_of_range_obj = make_request(1, 2, 5, {StateBit::battery_low});
    out_of_range_obj.object_id = 256;
    check("out of range object denied", !hngac_authorize(policy, 3, out_of_range_obj));

    // --- calibration_required in isolation ---
    PolicyRule policy_cal[kMaxPolicyRules] = {};
    set_bit(policy_cal[0].subjects, 3);
    set_bit(policy_cal[0].objects, 4);
    set_bit(policy_cal[0].attributes, 9);
    set_state_bit(policy_cal[0].required_states, StateBit::calibration_required);
    check("calibration_required alone allowed",
        hngac_authorize(policy_cal, 1, make_request(3, 4, 9, {StateBit::calibration_required})));
    check("calibration_required absent denied",
        !hngac_authorize(policy_cal, 1, make_request(3, 4, 9, {StateBit::battery_low})));

    // --- safety_interlock in isolation ---
    PolicyRule policy_si[kMaxPolicyRules] = {};
    set_bit(policy_si[0].subjects, 5);
    set_bit(policy_si[0].objects, 6);
    set_bit(policy_si[0].attributes, 3);
    set_state_bit(policy_si[0].required_states, StateBit::safety_interlock);
    check("safety_interlock alone allowed",
        hngac_authorize(policy_si, 1, make_request(5, 6, 3, {StateBit::safety_interlock})));
    check("safety_interlock absent denied",
        !hngac_authorize(policy_si, 1, make_request(5, 6, 3, {})));

    // --- empty policy (rule_count = 0) always denies ---
    check("empty policy denies", !hngac_authorize(policy, 0, make_request(1, 2, 5, {StateBit::battery_low})));

    // --- superset state passes (request carries extra state bits beyond what rule requires) ---
    // Rule 0 requires only battery_low; request carries battery_low + maintenance_mode
    check("superset state allowed",
        hngac_authorize(policy, 3,
            make_request(1, 2, 5, {StateBit::battery_low, StateBit::maintenance_mode})));

    // --- rule with required_states = 0 is a state wildcard ---
    PolicyRule policy_nostate[kMaxPolicyRules] = {};
    set_bit(policy_nostate[0].subjects, 2);
    set_bit(policy_nostate[0].objects, 3);
    set_bit(policy_nostate[0].attributes, 7);
    // required_states left at 0 — must allow any object_state
    check("zero required_states allows any state",
        hngac_authorize(policy_nostate, 1,
            make_request(2, 3, 7, {StateBit::battery_low, StateBit::safety_interlock})));
    check("zero required_states allows empty state",
        hngac_authorize(policy_nostate, 1, make_request(2, 3, 7, {})));

    // --- first-match-wins: second rule matches when first does not ---
    // policy[0] = subject 1, object 2, attr 5, state battery_low
    // policy[1] = subject 7, object 9, attr 11, state safety_interlock+calibration_required
    // Request matching only rule 1 must still be authorized
    check("second rule match allowed",
        hngac_authorize(policy, 3,
            make_request(7, 9, 11, {StateBit::safety_interlock, StateBit::calibration_required})));
    // Confirm rule 0 does not spuriously match the same request
    check("first rule does not match rule-1 request",
        !hngac_authorize(policy, 1,
            make_request(7, 9, 11, {StateBit::safety_interlock, StateBit::calibration_required})));

    // --- multi-bit attribute required by rule ---
    // Rule requires attribute bits 2 AND 5; request must satisfy both
    PolicyRule policy_multiattr[kMaxPolicyRules] = {};
    set_bit(policy_multiattr[0].subjects, 10);
    set_bit(policy_multiattr[0].objects, 20);
    set_bit(policy_multiattr[0].attributes, 2);
    set_bit(policy_multiattr[0].attributes, 5);
    {
        AuthorizationRequest both_attrs = make_request(10, 20, 2, {});
        set_bit(both_attrs.required_attributes, 5);
        check("multi-attr both bits allowed", hngac_authorize(policy_multiattr, 1, both_attrs));

        // Request requires only bit 2 — rule covers bits 2 and 5, so this is allowed (subset)
        AuthorizationRequest one_attr = make_request(10, 20, 2, {});
        check("multi-attr subset request allowed", hngac_authorize(policy_multiattr, 1, one_attr));

        // Request requires bit 8 — rule does not cover bit 8, so denied
        AuthorizationRequest wrong_attr = make_request(10, 20, 8, {});
        check("multi-attr wrong single bit denied", !hngac_authorize(policy_multiattr, 1, wrong_attr));

        AuthorizationRequest extra_attr = make_request(10, 20, 2, {});
        set_bit(extra_attr.required_attributes, 5);
        set_bit(extra_attr.required_attributes, 9);  // request needs bit 9 which rule does not have
        check("multi-attr extra required bit denied", !hngac_authorize(policy_multiattr, 1, extra_attr));
    }

    // --- all four state bits required simultaneously ---
    PolicyRule policy_allstate[kMaxPolicyRules] = {};
    set_bit(policy_allstate[0].subjects, 15);
    set_bit(policy_allstate[0].objects, 25);
    set_bit(policy_allstate[0].attributes, 1);
    set_state_bit(policy_allstate[0].required_states, StateBit::battery_low);
    set_state_bit(policy_allstate[0].required_states, StateBit::maintenance_mode);
    set_state_bit(policy_allstate[0].required_states, StateBit::safety_interlock);
    set_state_bit(policy_allstate[0].required_states, StateBit::calibration_required);
    check("all four state bits satisfied allowed",
        hngac_authorize(policy_allstate, 1,
            make_request(15, 25, 1, {StateBit::battery_low, StateBit::maintenance_mode,
                                     StateBit::safety_interlock, StateBit::calibration_required})));
    check("all four state bits: three present denied",
        !hngac_authorize(policy_allstate, 1,
            make_request(15, 25, 1, {StateBit::battery_low, StateBit::maintenance_mode,
                                     StateBit::safety_interlock})));

    // --- valid boundary nodes: subject 255 and object 255 (kMaxNodes - 1) ---
    PolicyRule policy_boundary[kMaxPolicyRules] = {};
    set_bit(policy_boundary[0].subjects, 255);
    set_bit(policy_boundary[0].objects, 255);
    set_bit(policy_boundary[0].attributes, 0);
    check("subject and object at 255 allowed",
        hngac_authorize(policy_boundary, 1, make_request(255, 255, 0, {})));
    {
        AuthorizationRequest req_255 = make_request(255, 255, 0, {});
        req_255.subject_id = 255;
        req_255.object_id = 254;  // object not in rule
        check("object 254 not in boundary rule denied",
            !hngac_authorize(policy_boundary, 1, req_255));
    }

    // --- rule with multiple subjects: both should be authorized independently ---
    PolicyRule policy_multisub[kMaxPolicyRules] = {};
    set_bit(policy_multisub[0].subjects, 8);
    set_bit(policy_multisub[0].subjects, 12);  // two subjects in one rule
    set_bit(policy_multisub[0].objects, 30);
    set_bit(policy_multisub[0].attributes, 4);
    check("multi-subject rule: first subject allowed",
        hngac_authorize(policy_multisub, 1, make_request(8, 30, 4, {})));
    check("multi-subject rule: second subject allowed",
        hngac_authorize(policy_multisub, 1, make_request(12, 30, 4, {})));
    check("multi-subject rule: unlisted subject denied",
        !hngac_authorize(policy_multisub, 1, make_request(9, 30, 4, {})));

    // --- rule_count clamping: passing count > kMaxPolicyRules is safe ---
    // kernel clamps to kMaxPolicyRules internally; use small policy, large count
    check("rule_count clamped: still finds matching rule",
        hngac_authorize(policy, static_cast<std::uint16_t>(kMaxPolicyRules + 1),
            make_request(1, 2, 5, {StateBit::battery_low})));

    // --- worst-case: 512 non-matching rules to stress the pipelined scan ---
    // Every slot holds a rule that cannot match the request, so the kernel
    // must iterate through all 512 rules before returning false.
    PolicyRule policy_worstcase[kMaxPolicyRules] = {};
    for (std::uint16_t i = 0; i < kMaxPolicyRules; ++i) {
        set_bit(policy_worstcase[i].subjects, 100);
        set_bit(policy_worstcase[i].objects, 100);
        set_bit(policy_worstcase[i].attributes, 0);
    }
    check("worst-case 512-rule full scan denies",
        !hngac_authorize(policy_worstcase, kMaxPolicyRules,
            make_request(0, 0, 0, {})));

    // --- 5D provenance cases ---
    // Policy: subject 20, object 30, attr 2, state battery_low, provenance authenticated_ros2_node
    PolicyRule policy_5d[kMaxPolicyRules] = {};
    set_bit(policy_5d[0].subjects, 20);
    set_bit(policy_5d[0].objects, 30);
    set_bit(policy_5d[0].attributes, 2);
    set_state_bit(policy_5d[0].required_states, StateBit::battery_low);
    set_provenance_bit(policy_5d[0].required_provenance, ProvenanceBit::authenticated_ros2_node);

    // Correct state + correct provenance: allow
    {
        AuthorizationRequest r = make_request(20, 30, 2, {StateBit::battery_low});
        set_provenance_bit(r.source_provenance, ProvenanceBit::authenticated_ros2_node);
        check("5D: correct state + correct provenance allowed", hngac_authorize(policy_5d, 1, r));
    }
    // Correct state + wrong provenance (compromised node scenario): deny
    {
        AuthorizationRequest r = make_request(20, 30, 2, {StateBit::battery_low});
        set_provenance_bit(r.source_provenance, ProvenanceBit::remote_operator);
        check("5D: correct state + wrong provenance denied", !hngac_authorize(policy_5d, 1, r));
    }
    // Correct state + no provenance set: deny
    {
        AuthorizationRequest r = make_request(20, 30, 2, {StateBit::battery_low});
        check("5D: correct state + no provenance denied", !hngac_authorize(policy_5d, 1, r));
    }
    // Superset provenance (source carries extra bits beyond what rule requires): allow
    {
        AuthorizationRequest r = make_request(20, 30, 2, {StateBit::battery_low});
        set_provenance_bit(r.source_provenance, ProvenanceBit::authenticated_ros2_node);
        set_provenance_bit(r.source_provenance, ProvenanceBit::local_terminal);
        check("5D: superset provenance allowed", hngac_authorize(policy_5d, 1, r));
    }
    // Rule with required_provenance == 0 is a wildcard: any source allowed
    {
        PolicyRule policy_noprov[kMaxPolicyRules] = {};
        set_bit(policy_noprov[0].subjects, 20);
        set_bit(policy_noprov[0].objects, 30);
        set_bit(policy_noprov[0].attributes, 2);
        set_state_bit(policy_noprov[0].required_states, StateBit::battery_low);
        // required_provenance left at 0

        AuthorizationRequest r_ros2 = make_request(20, 30, 2, {StateBit::battery_low});
        set_provenance_bit(r_ros2.source_provenance, ProvenanceBit::authenticated_ros2_node);
        check("5D: zero required_provenance allows authenticated_ros2_node",
              hngac_authorize(policy_noprov, 1, r_ros2));

        AuthorizationRequest r_remote = make_request(20, 30, 2, {StateBit::battery_low});
        set_provenance_bit(r_remote.source_provenance, ProvenanceBit::remote_operator);
        check("5D: zero required_provenance allows remote_operator",
              hngac_authorize(policy_noprov, 1, r_remote));

        AuthorizationRequest r_none = make_request(20, 30, 2, {StateBit::battery_low});
        check("5D: zero required_provenance allows zero source_provenance",
              hngac_authorize(policy_noprov, 1, r_none));
    }
    // State fail still denies even when provenance is correct
    {
        AuthorizationRequest r = make_request(20, 30, 2, {});
        set_provenance_bit(r.source_provenance, ProvenanceBit::authenticated_ros2_node);
        check("5D: correct provenance + wrong state denied", !hngac_authorize(policy_5d, 1, r));
    }
    // Rule requires two provenance types; either one satisfies it
    {
        PolicyRule policy_twoprov[kMaxPolicyRules] = {};
        set_bit(policy_twoprov[0].subjects, 20);
        set_bit(policy_twoprov[0].objects, 30);
        set_bit(policy_twoprov[0].attributes, 2);
        set_state_bit(policy_twoprov[0].required_states, StateBit::battery_low);
        set_provenance_bit(policy_twoprov[0].required_provenance, ProvenanceBit::authenticated_ros2_node);
        set_provenance_bit(policy_twoprov[0].required_provenance, ProvenanceBit::local_terminal);

        AuthorizationRequest r_ros2 = make_request(20, 30, 2, {StateBit::battery_low});
        set_provenance_bit(r_ros2.source_provenance, ProvenanceBit::authenticated_ros2_node);
        check("5D: two-provenance rule, first type allowed",
              hngac_authorize(policy_twoprov, 1, r_ros2));

        AuthorizationRequest r_local = make_request(20, 30, 2, {StateBit::battery_low});
        set_provenance_bit(r_local.source_provenance, ProvenanceBit::local_terminal);
        check("5D: two-provenance rule, second type allowed",
              hngac_authorize(policy_twoprov, 1, r_local));

        AuthorizationRequest r_remote = make_request(20, 30, 2, {StateBit::battery_low});
        set_provenance_bit(r_remote.source_provenance, ProvenanceBit::remote_operator);
        check("5D: two-provenance rule, third type denied",
              !hngac_authorize(policy_twoprov, 1, r_remote));
    }

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
