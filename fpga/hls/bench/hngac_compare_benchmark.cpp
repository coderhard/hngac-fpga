#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <queue>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
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

constexpr std::size_t kWarmupIterations = 1000;
constexpr std::uint16_t kRuleCount = 4;

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

struct DagEdge {
    std::uint32_t to = 0;
    Bitmask256 required_attributes{};
};

struct NgacDag {
    std::unordered_map<std::uint32_t, std::vector<DagEdge>> adjacency{};
    std::unordered_map<std::uint16_t, std::uint32_t> subject_nodes{};
    std::unordered_map<std::uint16_t, std::uint32_t> object_nodes{};
};

struct Summary {
    double mean_ns = 0.0;
    double p99_ns = 0.0;
    std::uint64_t max_ns = 0;
    std::size_t allowed = 0;
};

Bitmask256 make_full_mask() {
    Bitmask256 mask{};
    for (std::uint64_t& word : mask.words) {
        word = ~std::uint64_t{0};
    }
    return mask;
}

bool masks_equal(const Bitmask256& lhs, const Bitmask256& rhs) {
    for (std::size_t i = 0; i < lhs.words.size(); ++i) {
        if (lhs.words[i] != rhs.words[i]) {
            return false;
        }
    }
    return true;
}

void or_mask(Bitmask256& destination, const Bitmask256& source) {
    for (std::size_t i = 0; i < destination.words.size(); ++i) {
        destination.words[i] |= source.words[i];
    }
}

std::uint32_t pack_subject_object_key(std::uint16_t subject, std::uint16_t object) {
    return (static_cast<std::uint32_t>(subject) << 16) | static_cast<std::uint32_t>(object);
}

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

std::unordered_map<std::uint32_t, Bitmask256> build_rbac_hash_map(
    const PolicyRule3D policy[kMaxPolicyRules],
    std::uint16_t rule_count) {
    std::unordered_map<std::uint32_t, Bitmask256> permission_map;

    for (std::uint16_t i = 0; i < rule_count; ++i) {
        for (std::size_t subject = 0; subject < kMaxNodes; ++subject) {
            if (!test_bit(policy[i].subjects, subject)) {
                continue;
            }
            for (std::size_t object = 0; object < kMaxNodes; ++object) {
                if (!test_bit(policy[i].objects, object)) {
                    continue;
                }
                Bitmask256& permissions = permission_map[pack_subject_object_key(
                    static_cast<std::uint16_t>(subject),
                    static_cast<std::uint16_t>(object))];
                or_mask(permissions, policy[i].attributes);
            }
        }
    }

    return permission_map;
}

bool authorize_rbac_hash_map(
    const std::unordered_map<std::uint32_t, Bitmask256>& permission_map,
    const AuthorizationRequest& request) {
    const auto it = permission_map.find(pack_subject_object_key(request.subject_id, request.object_id));
    if (it == permission_map.end()) {
        return false;
    }
    return contains_all(request.required_attributes, it->second);
}

bool authorize_rbac_lookup(
    const RolePermissionRule rules[kMaxPolicyRules],
    std::uint16_t rule_count,
    const AuthorizationRequest& request,
    std::uint64_t state_lookup_delay_ns) {
    busy_wait_ns(state_lookup_delay_ns);
    const StateMask looked_up_state = request.object_state;

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

void add_unique_edge(
    NgacDag& dag,
    std::uint32_t from,
    std::uint32_t to,
    const Bitmask256& required_attributes) {
    std::vector<DagEdge>& edges = dag.adjacency[from];
    for (const DagEdge& edge : edges) {
        if (edge.to == to && masks_equal(edge.required_attributes, required_attributes)) {
            return;
        }
    }
    edges.push_back({to, required_attributes});
}

NgacDag build_ngac_dag(const PolicyRule3D policy[kMaxPolicyRules], std::uint16_t rule_count) {
    NgacDag dag{};
    std::unordered_map<std::uint16_t, std::uint32_t> ua_nodes;
    std::uint32_t next_node_id = 1;
    const Bitmask256 all_attributes = make_full_mask();

    const auto ensure_subject_node = [&](std::uint16_t subject_id) {
        auto it = dag.subject_nodes.find(subject_id);
        if (it != dag.subject_nodes.end()) {
            return it->second;
        }
        const std::uint32_t node_id = next_node_id++;
        dag.subject_nodes.emplace(subject_id, node_id);
        return node_id;
    };

    const auto ensure_ua_node = [&](std::uint16_t subject_id) {
        auto it = ua_nodes.find(subject_id);
        if (it != ua_nodes.end()) {
            return it->second;
        }
        const std::uint32_t node_id = next_node_id++;
        ua_nodes.emplace(subject_id, node_id);
        return node_id;
    };

    const auto ensure_object_node = [&](std::uint16_t object_id) {
        auto it = dag.object_nodes.find(object_id);
        if (it != dag.object_nodes.end()) {
            return it->second;
        }
        const std::uint32_t node_id = next_node_id++;
        dag.object_nodes.emplace(object_id, node_id);
        return node_id;
    };

    for (std::uint16_t i = 0; i < rule_count; ++i) {
        const std::uint32_t pc_node = next_node_id++;

        for (std::size_t subject = 0; subject < kMaxNodes; ++subject) {
            if (!test_bit(policy[i].subjects, subject)) {
                continue;
            }
            const std::uint16_t subject_id = static_cast<std::uint16_t>(subject);
            const std::uint32_t subject_node = ensure_subject_node(subject_id);
            const std::uint32_t ua_node = ensure_ua_node(subject_id);
            add_unique_edge(dag, subject_node, ua_node, all_attributes);
            add_unique_edge(dag, ua_node, pc_node, all_attributes);
        }

        for (std::size_t object = 0; object < kMaxNodes; ++object) {
            if (!test_bit(policy[i].objects, object)) {
                continue;
            }
            const std::uint32_t oa_node = next_node_id++;
            const std::uint32_t object_node =
                ensure_object_node(static_cast<std::uint16_t>(object));
            add_unique_edge(dag, pc_node, oa_node, policy[i].attributes);
            add_unique_edge(dag, oa_node, object_node, policy[i].attributes);
        }
    }

    return dag;
}

bool authorize_ngac_dag(const NgacDag& dag, const AuthorizationRequest& request) {
    const auto subject_it = dag.subject_nodes.find(request.subject_id);
    const auto object_it = dag.object_nodes.find(request.object_id);
    if (subject_it == dag.subject_nodes.end() || object_it == dag.object_nodes.end()) {
        return false;
    }

    std::queue<std::uint32_t> frontier;
    std::unordered_set<std::uint32_t> visited;
    frontier.push(subject_it->second);
    visited.insert(subject_it->second);

    while (!frontier.empty()) {
        const std::uint32_t node = frontier.front();
        frontier.pop();

        if (node == object_it->second) {
            return true;
        }

        const auto edges_it = dag.adjacency.find(node);
        if (edges_it == dag.adjacency.end()) {
            continue;
        }

        for (const DagEdge& edge : edges_it->second) {
            if (!contains_all(request.required_attributes, edge.required_attributes)) {
                continue;
            }
            if (visited.insert(edge.to).second) {
                frontier.push(edge.to);
            }
        }
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
    for (std::size_t i = 0; i < kWarmupIterations; ++i) {
        const AuthorizationRequest& request = requests[i % requests.size()];
        static_cast<void>(fn(request));
    }

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

    const auto permission_map = build_rbac_hash_map(policy_3d, kRuleCount);
    const NgacDag dag = build_ngac_dag(policy_3d, kRuleCount);

    AuthorizationRequest request_a_allow = make_request(1, 10, 5, {StateBit::battery_low});
    AuthorizationRequest request_b_allow = make_request(2, 11, 6, {StateBit::maintenance_mode});
    AuthorizationRequest request_c_allow = make_request(3, 12, 7, {StateBit::safety_interlock});
    AuthorizationRequest request_d_allow =
        make_request(4, 13, 8, {StateBit::calibration_required});

    AuthorizationRequest request_a_deny = make_request(1, 10, 5, {});
    AuthorizationRequest request_b_deny = make_request(2, 11, 6, {StateBit::battery_low});
    AuthorizationRequest request_c_deny = make_request(3, 12, 7, {});
    AuthorizationRequest request_d_deny =
        make_request(4, 13, 8, {StateBit::safety_interlock});

    const std::vector<AuthorizationRequest> requests = {
        request_a_allow,
        request_b_allow,
        request_c_allow,
        request_d_allow,
        request_a_deny,
        request_b_deny,
        request_c_deny,
        request_d_deny,
    };

    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "RBAC modeled external state lookup delay: " << lookup_delay_ns << " ns\n";
    std::cout << "Warmup per model: " << kWarmupIterations << " iterations\n";
    std::cout << "Scenario mix: 4 state-satisfying requests + 4 state-failing requests\n";

    const Summary rbac_hash_map = run_benchmark(
        "RBAC hash map",
        requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return authorize_rbac_hash_map(permission_map, request);
        });

    const Summary ngac_dag = run_benchmark(
        "NGAC-DAG traversal",
        requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return authorize_ngac_dag(dag, request);
        });

    const Summary baseline_3d = run_benchmark(
        "3D baseline",
        requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return authorize_3d(policy_3d, kRuleCount, request);
        });

    const Summary state_4d = run_benchmark(
        "4D state-aware",
        requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return hngac_authorize(policy_4d, kRuleCount, request);
        });

    const Summary rbac_lookup = run_benchmark(
        "RBAC + state lookup",
        requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return authorize_rbac_lookup(policy_rbac, kRuleCount, request, lookup_delay_ns);
        });

    const double overhead_pct =
        ((state_4d.mean_ns - baseline_3d.mean_ns) / baseline_3d.mean_ns) * 100.0;
    const double hashmap_gap_pct =
        ((state_4d.mean_ns - rbac_hash_map.mean_ns) / rbac_hash_map.mean_ns) * 100.0;
    const double dag_slowdown = ngac_dag.mean_ns / state_4d.mean_ns;
    const double rbac_slowdown = rbac_lookup.mean_ns / state_4d.mean_ns;

    std::cout << "4D vs 3D mean overhead: " << overhead_pct << "%\n";
    std::cout << "4D vs RBAC hash-map mean overhead: " << hashmap_gap_pct << "%\n";
    std::cout << "NGAC-DAG vs 4D mean slowdown: " << dag_slowdown << "x\n";
    std::cout << "RBAC+lookup vs 4D mean slowdown: " << rbac_slowdown << "x\n";
    return 0;
}
