#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <string_view>
#include <vector>

#include "hngac_kernel.hpp"

namespace {

using Clock = std::chrono::steady_clock;
using Ns = std::chrono::nanoseconds;

using hngac::fpga::AuthorizationRequest;
using hngac::fpga::Bitmask256;
using hngac::fpga::PolicyRule;
using hngac::fpga::ProvenanceMask;
using hngac::fpga::StateBit;
using hngac::fpga::StateMask;
using hngac::fpga::contains_all;
using hngac::fpga::contains_all_states;
using hngac::fpga::hngac_authorize;
using hngac::fpga::kMaxNodes;
using hngac::fpga::kMaxPolicyRules;
using hngac::fpga::set_bit;
using hngac::fpga::set_state_bit;
using hngac::fpga::test_bit;

struct PolicyRule3D {
    Bitmask256 subjects{};
    Bitmask256 objects{};
    Bitmask256 attributes{};
};

struct RolePermissionRule {
    std::uint16_t role_id = 0;
    std::uint16_t object_id = 0;
    Bitmask256 attributes{};
    StateMask required_states = 0;
    ProvenanceMask reserved_provenance = 0;
};

struct Summary {
    double mean_ns = 0.0;
    double p99_ns = 0.0;
    std::uint64_t max_ns = 0;
    std::size_t allowed = 0;
};

void busy_wait_ns(std::uint64_t delay_ns) {
    if (delay_ns == 0) {
        return;
    }
    const auto start = Clock::now();
    while (std::chrono::duration_cast<Ns>(Clock::now() - start).count() <
           static_cast<long long>(delay_ns)) {
    }
}

AuthorizationRequest make_request(
    std::uint16_t subject,
    std::uint16_t object,
    std::uint16_t attribute,
    std::initializer_list<StateBit> states) {
    AuthorizationRequest request{};
    request.subject_id = subject;
    request.object_id = object;
    set_bit(request.required_attributes, attribute);
    for (StateBit bit : states) {
        set_state_bit(request.object_state, bit);
    }
    return request;
}

void add_rule_4d(
    PolicyRule policy[kMaxPolicyRules],
    std::uint16_t index,
    std::uint16_t subject,
    std::uint16_t object,
    std::uint16_t attribute,
    std::initializer_list<StateBit> states) {
    set_bit(policy[index].subjects, subject);
    set_bit(policy[index].objects, object);
    set_bit(policy[index].attributes, attribute);
    for (StateBit bit : states) {
        set_state_bit(policy[index].required_states, bit);
    }
}

void add_rule_3d(
    PolicyRule3D policy[kMaxPolicyRules],
    std::uint16_t index,
    std::uint16_t subject,
    std::uint16_t object,
    std::uint16_t attribute) {
    set_bit(policy[index].subjects, subject);
    set_bit(policy[index].objects, object);
    set_bit(policy[index].attributes, attribute);
}

void add_rule_rbac(
    RolePermissionRule rules[kMaxPolicyRules],
    std::uint16_t index,
    std::uint16_t role_id,
    std::uint16_t object_id,
    std::uint16_t attribute,
    std::initializer_list<StateBit> states) {
    rules[index].role_id = role_id;
    rules[index].object_id = object_id;
    set_bit(rules[index].attributes, attribute);
    for (StateBit bit : states) {
        set_state_bit(rules[index].required_states, bit);
    }
}

bool authorize_3d(
    const PolicyRule3D policy[kMaxPolicyRules],
    std::uint16_t rule_count,
    const AuthorizationRequest& request) {
    for (std::uint16_t i = 0; i < rule_count; ++i) {
        const PolicyRule3D& rule = policy[i];
        if (!test_bit(rule.subjects, request.subject_id)) {
            continue;
        }
        if (!test_bit(rule.objects, request.object_id)) {
            continue;
        }
        if (!contains_all(request.required_attributes, rule.attributes)) {
            continue;
        }
        return true;
    }
    return false;
}

bool authorize_rbac_lookup(
    const RolePermissionRule rules[kMaxPolicyRules],
    std::uint16_t rule_count,
    const std::array<StateMask, kMaxNodes>& state_store,
    const AuthorizationRequest& request,
    std::uint64_t state_lookup_delay_ns) {
    busy_wait_ns(state_lookup_delay_ns);
    const StateMask looked_up_state = state_store[request.object_id];

    for (std::uint16_t i = 0; i < rule_count; ++i) {
        const RolePermissionRule& rule = rules[i];
        if (rule.role_id != request.subject_id) {
            continue;
        }
        if (rule.object_id != request.object_id) {
            continue;
        }
        if (!contains_all(request.required_attributes, rule.attributes)) {
            continue;
        }
        if (!contains_all_states(rule.required_states, looked_up_state)) {
            continue;
        }
        return true;
    }
    return false;
}

Summary summarize(const std::vector<std::uint64_t>& samples, std::size_t allowed_count) {
    std::vector<std::uint64_t> sorted = samples;
    std::sort(sorted.begin(), sorted.end());

    const double mean = static_cast<double>(
        std::accumulate(samples.begin(), samples.end(), std::uint64_t{0})) /
        static_cast<double>(samples.size());

    const std::size_t p99_index =
        static_cast<std::size_t>(0.99 * static_cast<double>(sorted.size() - 1));

    Summary summary{};
    summary.mean_ns = mean;
    summary.p99_ns = static_cast<double>(sorted[p99_index]);
    summary.max_ns = sorted.back();
    summary.allowed = allowed_count;
    return summary;
}

template <typename Fn>
Summary run_benchmark(
    std::string_view label,
    const std::vector<AuthorizationRequest>& requests,
    std::size_t iterations,
    Fn&& fn) {
    std::vector<std::uint64_t> samples;
    samples.reserve(iterations);
    std::size_t allowed = 0;

    for (std::size_t i = 0; i < iterations; ++i) {
        const AuthorizationRequest& request = requests[i % requests.size()];
        const auto start = Clock::now();
        const bool result = fn(request);
        const auto end = Clock::now();
        samples.push_back(static_cast<std::uint64_t>(
            std::chrono::duration_cast<Ns>(end - start).count()));
        if (result) {
            ++allowed;
        }
    }

    const Summary summary = summarize(samples, allowed);
    std::cout << label << ": mean=" << summary.mean_ns << " ns"
              << " p99=" << summary.p99_ns << " ns"
              << " max=" << summary.max_ns << " ns"
              << " allowed=" << summary.allowed << "/" << iterations << "\n";
    return summary;
}

}  // namespace

int main(int argc, char** argv) {
    std::size_t iterations = 20000;
    std::uint64_t lookup_delay_ns = 100000;

    if (argc > 1) {
        iterations = static_cast<std::size_t>(std::strtoull(argv[1], nullptr, 10));
    }
    if (argc > 2) {
        lookup_delay_ns = static_cast<std::uint64_t>(std::strtoull(argv[2], nullptr, 10));
    }

    PolicyRule policy_4d[kMaxPolicyRules] = {};
    PolicyRule3D policy_3d[kMaxPolicyRules] = {};
    RolePermissionRule policy_rbac[kMaxPolicyRules] = {};

    add_rule_4d(policy_4d, 0, 1, 10, 5, {StateBit::battery_low});
    add_rule_4d(policy_4d, 1, 2, 11, 6, {StateBit::maintenance_mode});
    add_rule_4d(policy_4d, 2, 3, 12, 7, {StateBit::safety_interlock});
    add_rule_4d(policy_4d, 3, 4, 13, 8, {StateBit::calibration_required});

    add_rule_3d(policy_3d, 0, 1, 10, 5);
    add_rule_3d(policy_3d, 1, 2, 11, 6);
    add_rule_3d(policy_3d, 2, 3, 12, 7);
    add_rule_3d(policy_3d, 3, 4, 13, 8);

    add_rule_rbac(policy_rbac, 0, 1, 10, 5, {StateBit::battery_low});
    add_rule_rbac(policy_rbac, 1, 2, 11, 6, {StateBit::maintenance_mode});
    add_rule_rbac(policy_rbac, 2, 3, 12, 7, {StateBit::safety_interlock});
    add_rule_rbac(policy_rbac, 3, 4, 13, 8, {StateBit::calibration_required});

    std::array<StateMask, kMaxNodes> state_store{};
    AuthorizationRequest request_a = make_request(1, 10, 5, {StateBit::battery_low});
    AuthorizationRequest request_b = make_request(2, 11, 6, {StateBit::maintenance_mode});
    AuthorizationRequest request_c = make_request(3, 12, 7, {StateBit::safety_interlock});
    AuthorizationRequest request_d =
        make_request(4, 13, 8, {StateBit::calibration_required});

    state_store[request_a.object_id] = request_a.object_state;
    state_store[request_b.object_id] = request_b.object_state;
    state_store[request_c.object_id] = request_c.object_state;
    state_store[request_d.object_id] = request_d.object_state;

    const std::vector<AuthorizationRequest> requests = {
        request_a,
        request_b,
        request_c,
        request_d,
    };

    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "RBAC modeled external state lookup delay: " << lookup_delay_ns << " ns\n";

    const Summary baseline_3d = run_benchmark(
        "3D baseline",
        requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return authorize_3d(policy_3d, 4, request);
        });

    const Summary state_4d = run_benchmark(
        "4D state-aware",
        requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return hngac_authorize(policy_4d, 4, request);
        });

    const Summary rbac_lookup = run_benchmark(
        "RBAC + state lookup",
        requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return authorize_rbac_lookup(policy_rbac, 4, state_store, request, lookup_delay_ns);
        });

    const double overhead_pct =
        ((state_4d.mean_ns - baseline_3d.mean_ns) / baseline_3d.mean_ns) * 100.0;
    const double rbac_slowdown = rbac_lookup.mean_ns / state_4d.mean_ns;

    std::cout << "4D vs 3D mean overhead: " << overhead_pct << "%\n";
    std::cout << "RBAC+lookup vs 4D mean slowdown: " << rbac_slowdown << "x\n";
    return 0;
}
