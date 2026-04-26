#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <queue>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef HNGAC_HAVE_SQLITE
#include <sqlite3.h>
#endif

#include "hngac_kernel.hpp"

namespace {

using Clock = std::chrono::steady_clock;
using Ns = std::chrono::nanoseconds;

using hngac::fpga::AuthorizationRequest;
using hngac::fpga::Bitmask256;
using hngac::fpga::PolicyRule;
using hngac::fpga::ProvenanceBit;
using hngac::fpga::ProvenanceMask;
using hngac::fpga::StateBit;
using hngac::fpga::StateMask;
using hngac::fpga::contains_all;
using hngac::fpga::contains_all_states;
using hngac::fpga::hngac_authorize;
using hngac::fpga::kMaxNodes;
using hngac::fpga::kMaxPolicyRules;
using hngac::fpga::provenance_permitted;
using hngac::fpga::set_bit;
using hngac::fpga::set_provenance_bit;
using hngac::fpga::set_state_bit;
using hngac::fpga::test_bit;

constexpr std::size_t kWarmupIterations = 1000;
constexpr std::size_t kBuildMeasurementRepeats = 12;
constexpr std::uint16_t kDefaultRuleCount = 4;
constexpr StateMask kEnumeratedStateMaskLimit = 1u << 4;
constexpr ProvenanceMask kEnumeratedProvenanceMaskLimit = 1u << 3;

volatile std::uint64_t g_build_sink = 0;

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

#ifdef HNGAC_HAVE_SQLITE
struct SqliteStateLookup {
    sqlite3* db = nullptr;
    sqlite3_stmt* select_state_stmt = nullptr;
};
#endif

struct RuleDescriptor {
    std::uint16_t subject_id = 0;
    std::uint16_t object_id = 0;
    std::uint16_t attribute_id = 0;
    StateMask required_states = 0;
    ProvenanceMask required_provenance = 0;
};

struct PolicyCorpus {
    std::array<PolicyRule, kMaxPolicyRules> policy_5d{};
    std::array<PolicyRule, kMaxPolicyRules> policy_4d{};
    std::array<PolicyRule3D, kMaxPolicyRules> policy_3d{};
    std::array<RolePermissionRule, kMaxPolicyRules> policy_rbac{};
    std::vector<AuthorizationRequest> requests{};
};

struct Summary {
    double mean_ns = 0.0;
    double p95_ns = 0.0;
    double p99_ns = 0.0;
    double stddev_ns = 0.0;
    std::uint64_t min_ns = 0;
    std::uint64_t max_ns = 0;
    std::size_t allowed = 0;
};

struct BuildStats {
    double build_mean_ns = 0.0;
    std::uint64_t build_max_ns = 0;
    double reload_mean_ns = 0.0;
    std::uint64_t reload_max_ns = 0;
    std::size_t memory_bytes = 0;
    std::size_t entry_count = 0;
    std::size_t max_probe = 0;
};

std::uint64_t mix64(std::uint64_t value) {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

std::size_t next_power_of_two(std::size_t value) {
    if (value <= 1) {
        return 1;
    }
    std::size_t power = 1;
    while (power < value) {
        power <<= 1;
    }
    return power;
}

class FlatHashAllowSet {
  public:
    static constexpr std::uint64_t kEmptyKey = std::numeric_limits<std::uint64_t>::max();

    FlatHashAllowSet() = default;

    explicit FlatHashAllowSet(std::size_t expected_entries) {
        reserve(expected_entries);
    }

    void reserve(std::size_t expected_entries) {
        const std::size_t capacity =
            next_power_of_two(std::max<std::size_t>(16, expected_entries * 2));
        buckets_.assign(capacity, kEmptyKey);
        size_ = 0;
        max_probe_ = 0;
    }

    void insert(std::uint64_t key) {
        if (buckets_.empty()) {
            reserve(16);
        }

        std::size_t index = bucket_index(key);
        std::size_t probe = 0;
        for (;;) {
            const std::uint64_t current = buckets_[index];
            if (current == kEmptyKey) {
                buckets_[index] = key;
                ++size_;
                max_probe_ = std::max(max_probe_, probe);
                return;
            }
            if (current == key) {
                max_probe_ = std::max(max_probe_, probe);
                return;
            }
            ++probe;
            index = (index + 1) & (buckets_.size() - 1);
        }
    }

    bool contains(std::uint64_t key) const {
        if (buckets_.empty()) {
            return false;
        }

        std::size_t index = bucket_index(key);
        std::size_t probe = 0;
        while (probe < buckets_.size()) {
            const std::uint64_t current = buckets_[index];
            if (current == kEmptyKey) {
                return false;
            }
            if (current == key) {
                return true;
            }
            ++probe;
            index = (index + 1) & (buckets_.size() - 1);
        }
        return false;
    }

    std::size_t size() const {
        return size_;
    }

    std::size_t bucket_count() const {
        return buckets_.size();
    }

    std::size_t memory_bytes() const {
        return buckets_.size() * sizeof(std::uint64_t);
    }

    std::size_t max_probe() const {
        return max_probe_;
    }

  private:
    std::size_t bucket_index(std::uint64_t key) const {
        return static_cast<std::size_t>(mix64(key)) & (buckets_.size() - 1);
    }

    std::vector<std::uint64_t> buckets_{};
    std::size_t size_ = 0;
    std::size_t max_probe_ = 0;
};

struct Flattened5DLookup {
    FlatHashAllowSet allowed_keys{};
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

std::uint64_t pack_flattened_5d_key(
    std::uint16_t subject,
    std::uint16_t object,
    std::uint16_t attribute,
    StateMask state_mask,
    ProvenanceMask provenance_mask) {
    return (static_cast<std::uint64_t>(subject) << 48) |
           (static_cast<std::uint64_t>(object) << 32) |
           (static_cast<std::uint64_t>(attribute) << 16) |
           (static_cast<std::uint64_t>(state_mask & 0xffu) << 8) |
           static_cast<std::uint64_t>(provenance_mask & 0xffu);
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

std::size_t first_set_bit_index(std::uint32_t mask, std::size_t bit_limit) {
    for (std::size_t bit = 0; bit < bit_limit; ++bit) {
        if ((mask & (std::uint32_t{1} << bit)) != 0) {
            return bit;
        }
    }
    return bit_limit;
}

bool extract_single_attribute_id(const Bitmask256& mask, std::uint16_t& attribute_id) {
    bool seen = false;
    for (std::size_t bit = 0; bit < kMaxNodes; ++bit) {
        if (!test_bit(mask, bit)) {
            continue;
        }
        if (seen) {
            return false;
        }
        attribute_id = static_cast<std::uint16_t>(bit);
        seen = true;
    }
    return seen;
}

AuthorizationRequest make_atomic_request(
    std::uint16_t subject,
    std::uint16_t object,
    std::uint16_t attribute,
    StateMask state_mask,
    ProvenanceMask provenance_mask) {
    AuthorizationRequest request{};
    request.subject_id = subject;
    request.object_id = object;
    set_bit(request.required_attributes, attribute);
    request.object_state = state_mask;
    request.source_provenance = provenance_mask;
    return request;
}

StateMask make_allow_state_mask(StateMask required, std::size_t seed) {
    StateMask state_mask = required;
    if (state_mask == 0) {
        state_mask = (StateMask{1} << (seed % 4));
        return state_mask;
    }
    state_mask |= (StateMask{1} << ((seed + 1) % 4));
    return state_mask;
}

StateMask make_deny_state_mask(StateMask required) {
    if (required == 0) {
        return 0;
    }
    return required & (required - 1);
}

ProvenanceMask make_allow_provenance_mask(ProvenanceMask required, std::size_t seed) {
    if (required == 0) {
        return (ProvenanceMask{1} << (seed % 3));
    }

    ProvenanceMask provenance_mask =
        (ProvenanceMask{1} << first_set_bit_index(required, 3));
    provenance_mask |= (ProvenanceMask{1} << ((seed + 1) % 3));
    return provenance_mask;
}

ProvenanceMask make_deny_provenance_mask(ProvenanceMask required, std::size_t seed) {
    if (required == 0) {
        return 0;
    }

    for (std::size_t offset = 0; offset < 3; ++offset) {
        const ProvenanceMask candidate = ProvenanceMask{1} << ((seed + offset) % 3);
        if ((required & candidate) == 0) {
            return candidate;
        }
    }
    return 0;
}

RuleDescriptor describe_rule(std::size_t index) {
    RuleDescriptor descriptor{};
    descriptor.subject_id = static_cast<std::uint16_t>(index % kMaxNodes);
    descriptor.object_id =
        static_cast<std::uint16_t>(((index % kMaxNodes) + ((index / kMaxNodes) * 97u)) % kMaxNodes);
    descriptor.attribute_id = static_cast<std::uint16_t>((index * 17u + 5u) % kMaxNodes);

    const StateBit primary_state = static_cast<StateBit>(index % 4);
    const StateBit secondary_state = static_cast<StateBit>((index + 1) % 4);
    const ProvenanceBit primary_provenance = static_cast<ProvenanceBit>(index % 3);
    const ProvenanceBit secondary_provenance = static_cast<ProvenanceBit>((index + 1) % 3);

    switch (index % 6) {
        case 0:
            set_state_bit(descriptor.required_states, primary_state);
            set_provenance_bit(descriptor.required_provenance, primary_provenance);
            break;
        case 1:
            set_state_bit(descriptor.required_states, primary_state);
            set_state_bit(descriptor.required_states, secondary_state);
            set_provenance_bit(descriptor.required_provenance, primary_provenance);
            break;
        case 2:
            set_provenance_bit(descriptor.required_provenance, primary_provenance);
            break;
        case 3:
            set_state_bit(descriptor.required_states, primary_state);
            set_provenance_bit(descriptor.required_provenance, primary_provenance);
            set_provenance_bit(descriptor.required_provenance, secondary_provenance);
            break;
        case 4:
            set_state_bit(descriptor.required_states, primary_state);
            set_state_bit(descriptor.required_states, secondary_state);
            break;
        case 5:
            set_state_bit(descriptor.required_states, static_cast<StateBit>((index + 2) % 4));
            set_provenance_bit(
                descriptor.required_provenance,
                static_cast<ProvenanceBit>((index + 2) % 3));
            break;
        default:
            break;
    }

    return descriptor;
}

void add_rule_5d(
    std::array<PolicyRule, kMaxPolicyRules>& policy,
    std::size_t index,
    const RuleDescriptor& descriptor) {
    set_bit(policy[index].subjects, descriptor.subject_id);
    set_bit(policy[index].objects, descriptor.object_id);
    set_bit(policy[index].attributes, descriptor.attribute_id);
    policy[index].required_states = descriptor.required_states;
    policy[index].required_provenance = descriptor.required_provenance;
}

void add_rule_4d(
    std::array<PolicyRule, kMaxPolicyRules>& policy,
    std::size_t index,
    const RuleDescriptor& descriptor) {
    set_bit(policy[index].subjects, descriptor.subject_id);
    set_bit(policy[index].objects, descriptor.object_id);
    set_bit(policy[index].attributes, descriptor.attribute_id);
    policy[index].required_states = descriptor.required_states;
}

void add_rule_3d(
    std::array<PolicyRule3D, kMaxPolicyRules>& policy,
    std::size_t index,
    const RuleDescriptor& descriptor) {
    set_bit(policy[index].subjects, descriptor.subject_id);
    set_bit(policy[index].objects, descriptor.object_id);
    set_bit(policy[index].attributes, descriptor.attribute_id);
}

void add_rule_rbac(
    std::array<RolePermissionRule, kMaxPolicyRules>& policy,
    std::size_t index,
    const RuleDescriptor& descriptor) {
    policy[index].role_id = descriptor.subject_id;
    policy[index].object_id = descriptor.object_id;
    set_bit(policy[index].attributes, descriptor.attribute_id);
    policy[index].required_states = descriptor.required_states;
}

PolicyCorpus build_policy_corpus(std::uint16_t rule_count) {
    PolicyCorpus corpus{};
    corpus.requests.reserve(static_cast<std::size_t>(rule_count) * 3);

    for (std::size_t i = 0; i < rule_count; ++i) {
        const RuleDescriptor descriptor = describe_rule(i);
        add_rule_5d(corpus.policy_5d, i, descriptor);
        add_rule_4d(corpus.policy_4d, i, descriptor);
        add_rule_3d(corpus.policy_3d, i, descriptor);
        add_rule_rbac(corpus.policy_rbac, i, descriptor);

        corpus.requests.push_back(make_atomic_request(
            descriptor.subject_id,
            descriptor.object_id,
            descriptor.attribute_id,
            make_allow_state_mask(descriptor.required_states, i),
            make_allow_provenance_mask(descriptor.required_provenance, i)));

        if (descriptor.required_states != 0) {
            corpus.requests.push_back(make_atomic_request(
                descriptor.subject_id,
                descriptor.object_id,
                descriptor.attribute_id,
                make_deny_state_mask(descriptor.required_states),
                make_allow_provenance_mask(descriptor.required_provenance, i)));
        }

        if (descriptor.required_provenance != 0) {
            const ProvenanceMask deny_provenance =
                make_deny_provenance_mask(descriptor.required_provenance, i);
            if (deny_provenance != 0) {
                corpus.requests.push_back(make_atomic_request(
                    descriptor.subject_id,
                    descriptor.object_id,
                    descriptor.attribute_id,
                    make_allow_state_mask(descriptor.required_states, i),
                    deny_provenance));
            }
        }
    }

    return corpus;
}

std::array<PolicyRule, kMaxPolicyRules> make_mutated_policy(
    const std::array<PolicyRule, kMaxPolicyRules>& base_policy,
    std::uint16_t rule_count) {
    std::array<PolicyRule, kMaxPolicyRules> mutated = base_policy;
    if (rule_count == 0) {
        return mutated;
    }

    PolicyRule& rule = mutated[rule_count - 1];

    std::uint16_t attribute_id = 0;
    if (extract_single_attribute_id(rule.attributes, attribute_id)) {
        rule.attributes = {};
        set_bit(rule.attributes, (attribute_id + 1) % kMaxNodes);
    }

    if (rule.required_states == 0) {
        set_state_bit(rule.required_states, static_cast<StateBit>(rule_count % 4));
    } else {
        const StateMask next_state = make_deny_state_mask(rule.required_states);
        rule.required_states = next_state != 0 ? next_state : (StateMask{1} << (rule_count % 4));
    }

    const ProvenanceMask next_provenance =
        make_deny_provenance_mask(rule.required_provenance, rule_count);
    if (next_provenance != 0) {
        rule.required_provenance = next_provenance;
    } else if (rule.required_provenance == 0) {
        set_provenance_bit(
            rule.required_provenance,
            static_cast<ProvenanceBit>(rule_count % 3));
    } else {
        rule.required_provenance = 0;
    }

    return mutated;
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
    permission_map.reserve(rule_count);

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

#ifdef HNGAC_HAVE_SQLITE
bool initialize_sqlite_state_lookup(
    SqliteStateLookup& sqlite_lookup,
    const std::vector<AuthorizationRequest>& requests) {
    if (sqlite3_open(":memory:", &sqlite_lookup.db) != SQLITE_OK) {
        return false;
    }

    sqlite3_exec(sqlite_lookup.db, "PRAGMA journal_mode=OFF;", nullptr, nullptr, nullptr);
    sqlite3_exec(sqlite_lookup.db, "PRAGMA synchronous=OFF;", nullptr, nullptr, nullptr);
    sqlite3_exec(sqlite_lookup.db, "PRAGMA temp_store=MEMORY;", nullptr, nullptr, nullptr);

    constexpr const char* create_sql =
        "CREATE TABLE object_state ("
        "object_id INTEGER NOT NULL, "
        "observed_state INTEGER NOT NULL, "
        "PRIMARY KEY (object_id, observed_state));";
    if (sqlite3_exec(sqlite_lookup.db, create_sql, nullptr, nullptr, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_stmt* insert_stmt = nullptr;
    constexpr const char* insert_sql =
        "INSERT OR IGNORE INTO object_state(object_id, observed_state) VALUES (?, ?);";
    if (sqlite3_prepare_v2(sqlite_lookup.db, insert_sql, -1, &insert_stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    for (const AuthorizationRequest& request : requests) {
        sqlite3_bind_int(insert_stmt, 1, static_cast<int>(request.object_id));
        sqlite3_bind_int(insert_stmt, 2, static_cast<int>(request.object_state));
        if (sqlite3_step(insert_stmt) != SQLITE_DONE) {
            sqlite3_finalize(insert_stmt);
            return false;
        }
        sqlite3_reset(insert_stmt);
        sqlite3_clear_bindings(insert_stmt);
    }
    sqlite3_finalize(insert_stmt);

    constexpr const char* select_sql =
        "SELECT observed_state FROM object_state WHERE object_id = ? AND observed_state = ?;";
    if (sqlite3_prepare_v2(
            sqlite_lookup.db, select_sql, -1, &sqlite_lookup.select_state_stmt, nullptr) !=
        SQLITE_OK) {
        return false;
    }

    return true;
}

bool lookup_sqlite_state(
    SqliteStateLookup& sqlite_lookup,
    const AuthorizationRequest& request,
    StateMask& looked_up_state) {
    sqlite3_bind_int(sqlite_lookup.select_state_stmt, 1, static_cast<int>(request.object_id));
    sqlite3_bind_int(sqlite_lookup.select_state_stmt, 2, static_cast<int>(request.object_state));

    const int rc = sqlite3_step(sqlite_lookup.select_state_stmt);
    if (rc == SQLITE_ROW) {
        looked_up_state =
            static_cast<StateMask>(sqlite3_column_int(sqlite_lookup.select_state_stmt, 0));
        sqlite3_reset(sqlite_lookup.select_state_stmt);
        sqlite3_clear_bindings(sqlite_lookup.select_state_stmt);
        return true;
    }

    sqlite3_reset(sqlite_lookup.select_state_stmt);
    sqlite3_clear_bindings(sqlite_lookup.select_state_stmt);
    return false;
}

bool authorize_rbac_sqlite_lookup(
    SqliteStateLookup& sqlite_lookup,
    const RolePermissionRule rules[kMaxPolicyRules],
    std::uint16_t rule_count,
    const AuthorizationRequest& request) {
    StateMask looked_up_state = 0;
    if (!lookup_sqlite_state(sqlite_lookup, request, looked_up_state)) {
        return false;
    }

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

void destroy_sqlite_state_lookup(SqliteStateLookup& sqlite_lookup) {
    if (sqlite_lookup.select_state_stmt != nullptr) {
        sqlite3_finalize(sqlite_lookup.select_state_stmt);
        sqlite_lookup.select_state_stmt = nullptr;
    }
    if (sqlite_lookup.db != nullptr) {
        sqlite3_close(sqlite_lookup.db);
        sqlite_lookup.db = nullptr;
    }
}
#endif

Flattened5DLookup build_flattened_5d_lookup(
    const PolicyRule policy[kMaxPolicyRules],
    std::uint16_t rule_count) {
    Flattened5DLookup lookup{};
    lookup.allowed_keys.reserve(
        static_cast<std::size_t>(rule_count) * kEnumeratedStateMaskLimit *
        kEnumeratedProvenanceMaskLimit);

    for (std::uint16_t i = 0; i < rule_count; ++i) {
        const PolicyRule& rule = policy[i];
        for (std::size_t subject = 0; subject < kMaxNodes; ++subject) {
            if (!test_bit(rule.subjects, subject)) {
                continue;
            }
            for (std::size_t object = 0; object < kMaxNodes; ++object) {
                if (!test_bit(rule.objects, object)) {
                    continue;
                }
                for (std::size_t attribute = 0; attribute < kMaxNodes; ++attribute) {
                    if (!test_bit(rule.attributes, attribute)) {
                        continue;
                    }
                    for (StateMask state_mask = 0;
                         state_mask < kEnumeratedStateMaskLimit;
                         ++state_mask) {
                        if (!contains_all_states(rule.required_states, state_mask)) {
                            continue;
                        }
                        for (ProvenanceMask provenance_mask = 0;
                             provenance_mask < kEnumeratedProvenanceMaskLimit;
                             ++provenance_mask) {
                            if (!provenance_permitted(
                                    rule.required_provenance, provenance_mask)) {
                                continue;
                            }
                            lookup.allowed_keys.insert(pack_flattened_5d_key(
                                static_cast<std::uint16_t>(subject),
                                static_cast<std::uint16_t>(object),
                                static_cast<std::uint16_t>(attribute),
                                state_mask,
                                provenance_mask));
                        }
                    }
                }
            }
        }
    }

    return lookup;
}

bool authorize_flattened_5d_lookup(
    const Flattened5DLookup& lookup,
    const AuthorizationRequest& request) {
    std::uint16_t attribute_id = 0;
    if (!extract_single_attribute_id(request.required_attributes, attribute_id)) {
        return false;
    }

    return lookup.allowed_keys.contains(pack_flattened_5d_key(
        request.subject_id,
        request.object_id,
        attribute_id,
        request.object_state,
        request.source_provenance));
}

BuildStats measure_policy_array_stats(
    const std::array<PolicyRule, kMaxPolicyRules>& base_policy,
    const std::array<PolicyRule, kMaxPolicyRules>& mutated_policy,
    std::uint16_t rule_count) {
    std::vector<std::uint64_t> build_samples;
    std::vector<std::uint64_t> reload_samples;

    for (std::size_t repeat = 0; repeat < kBuildMeasurementRepeats; ++repeat) {
        const auto start = Clock::now();
        std::array<PolicyRule, kMaxPolicyRules> copy{};
        std::copy_n(base_policy.begin(), rule_count, copy.begin());
        const auto end = Clock::now();
        build_samples.push_back(static_cast<std::uint64_t>(
            std::chrono::duration_cast<Ns>(end - start).count()));
        g_build_sink += copy[0].required_states;
    }

    for (std::size_t repeat = 0; repeat < kBuildMeasurementRepeats; ++repeat) {
        const auto start = Clock::now();
        std::array<PolicyRule, kMaxPolicyRules> copy{};
        std::copy_n(mutated_policy.begin(), rule_count, copy.begin());
        const auto end = Clock::now();
        reload_samples.push_back(static_cast<std::uint64_t>(
            std::chrono::duration_cast<Ns>(end - start).count()));
        g_build_sink += copy[rule_count - 1].required_provenance;
    }

    BuildStats stats{};
    const auto build_summary = [&build_samples]() {
        const double mean = static_cast<double>(
            std::accumulate(build_samples.begin(), build_samples.end(), std::uint64_t{0})) /
            static_cast<double>(build_samples.size());
        const std::uint64_t max =
            *std::max_element(build_samples.begin(), build_samples.end());
        return std::pair<double, std::uint64_t>{mean, max};
    }();
    const auto reload_summary = [&reload_samples]() {
        const double mean = static_cast<double>(
            std::accumulate(reload_samples.begin(), reload_samples.end(), std::uint64_t{0})) /
            static_cast<double>(reload_samples.size());
        const std::uint64_t max =
            *std::max_element(reload_samples.begin(), reload_samples.end());
        return std::pair<double, std::uint64_t>{mean, max};
    }();

    stats.build_mean_ns = build_summary.first;
    stats.build_max_ns = build_summary.second;
    stats.reload_mean_ns = reload_summary.first;
    stats.reload_max_ns = reload_summary.second;
    stats.memory_bytes = static_cast<std::size_t>(rule_count) * sizeof(PolicyRule);
    stats.entry_count = rule_count;
    return stats;
}

std::pair<Flattened5DLookup, BuildStats> build_flattened_lookup_with_stats(
    const std::array<PolicyRule, kMaxPolicyRules>& base_policy,
    const std::array<PolicyRule, kMaxPolicyRules>& mutated_policy,
    std::uint16_t rule_count) {
    std::vector<std::uint64_t> build_samples;
    std::vector<std::uint64_t> reload_samples;
    Flattened5DLookup lookup{};

    for (std::size_t repeat = 0; repeat < kBuildMeasurementRepeats; ++repeat) {
        const auto start = Clock::now();
        Flattened5DLookup candidate = build_flattened_5d_lookup(base_policy.data(), rule_count);
        const auto end = Clock::now();
        build_samples.push_back(static_cast<std::uint64_t>(
            std::chrono::duration_cast<Ns>(end - start).count()));
        g_build_sink += candidate.allowed_keys.size();
        if (repeat + 1 == kBuildMeasurementRepeats) {
            lookup = std::move(candidate);
        }
    }

    for (std::size_t repeat = 0; repeat < kBuildMeasurementRepeats; ++repeat) {
        const auto start = Clock::now();
        Flattened5DLookup candidate =
            build_flattened_5d_lookup(mutated_policy.data(), rule_count);
        const auto end = Clock::now();
        reload_samples.push_back(static_cast<std::uint64_t>(
            std::chrono::duration_cast<Ns>(end - start).count()));
        g_build_sink += candidate.allowed_keys.size();
    }

    BuildStats stats{};
    const auto build_summary = [&build_samples]() {
        const double mean = static_cast<double>(
            std::accumulate(build_samples.begin(), build_samples.end(), std::uint64_t{0})) /
            static_cast<double>(build_samples.size());
        const std::uint64_t max =
            *std::max_element(build_samples.begin(), build_samples.end());
        return std::pair<double, std::uint64_t>{mean, max};
    }();
    const auto reload_summary = [&reload_samples]() {
        const double mean = static_cast<double>(
            std::accumulate(reload_samples.begin(), reload_samples.end(), std::uint64_t{0})) /
            static_cast<double>(reload_samples.size());
        const std::uint64_t max =
            *std::max_element(reload_samples.begin(), reload_samples.end());
        return std::pair<double, std::uint64_t>{mean, max};
    }();

    stats.build_mean_ns = build_summary.first;
    stats.build_max_ns = build_summary.second;
    stats.reload_mean_ns = reload_summary.first;
    stats.reload_max_ns = reload_summary.second;
    stats.memory_bytes = lookup.allowed_keys.memory_bytes();
    stats.entry_count = lookup.allowed_keys.size();
    stats.max_probe = lookup.allowed_keys.max_probe();
    return {std::move(lookup), stats};
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

    long double total = 0.0L;
    for (std::uint64_t sample : samples) {
        total += static_cast<long double>(sample);
    }
    const long double mean = total / static_cast<long double>(samples.size());

    long double variance = 0.0L;
    for (std::uint64_t sample : samples) {
        const long double delta = static_cast<long double>(sample) - mean;
        variance += delta * delta;
    }
    variance /= static_cast<long double>(samples.size());

    const std::size_t p95_index =
        static_cast<std::size_t>(0.95 * static_cast<double>(sorted.size() - 1));
    const std::size_t p99_index =
        static_cast<std::size_t>(0.99 * static_cast<double>(sorted.size() - 1));

    Summary summary{};
    summary.mean_ns = static_cast<double>(mean);
    summary.p95_ns = static_cast<double>(sorted[p95_index]);
    summary.p99_ns = static_cast<double>(sorted[p99_index]);
    summary.stddev_ns = std::sqrt(static_cast<double>(variance));
    summary.min_ns = sorted.front();
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
    std::cout << std::fixed << std::setprecision(2)
              << label << ": mean=" << summary.mean_ns << " ns"
              << " p95=" << summary.p95_ns << " ns"
              << " p99=" << summary.p99_ns << " ns"
              << " max=" << static_cast<double>(summary.max_ns) << " ns"
              << " stddev=" << summary.stddev_ns << " ns"
              << " allowed=" << summary.allowed << "/" << iterations << "\n";
    std::cout << "SUMMARY|" << label
              << "|" << summary.mean_ns
              << "|" << summary.p95_ns
              << "|" << summary.p99_ns
              << "|" << summary.max_ns
              << "|" << summary.stddev_ns
              << "|" << summary.allowed
              << "|" << iterations << "\n";
    return summary;
}

void print_build_stats(std::string_view label, const BuildStats& stats) {
    std::cout << std::fixed << std::setprecision(2)
              << label << " build: mean=" << stats.build_mean_ns << " ns"
              << " reload_mean=" << stats.reload_mean_ns << " ns"
              << " memory=" << static_cast<double>(stats.memory_bytes) << " B"
              << " entries=" << stats.entry_count;
    if (stats.max_probe != 0) {
        std::cout << " max_probe=" << stats.max_probe;
    }
    std::cout << "\n";
    std::cout << "BUILD|" << label
              << "|" << stats.build_mean_ns
              << "|" << stats.build_max_ns
              << "|" << stats.reload_mean_ns
              << "|" << stats.reload_max_ns
              << "|" << stats.memory_bytes
              << "|" << stats.entry_count
              << "|" << stats.max_probe << "\n";
}

bool validate_flattened_lookup(
    const std::array<PolicyRule, kMaxPolicyRules>& policy_5d,
    const Flattened5DLookup& lookup,
    std::uint16_t rule_count,
    const std::vector<AuthorizationRequest>& requests) {
    for (const AuthorizationRequest& request : requests) {
        const bool kernel_result = hngac_authorize(policy_5d.data(), rule_count, request);
        const bool flattened_result = authorize_flattened_5d_lookup(lookup, request);
        if (kernel_result != flattened_result) {
            return false;
        }
    }
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    std::size_t iterations = 20000;
    std::uint64_t lookup_delay_ns = 100000;
    std::uint16_t rule_count = kDefaultRuleCount;

    if (argc > 1) {
        iterations = static_cast<std::size_t>(std::strtoull(argv[1], nullptr, 10));
    }
    if (argc > 2) {
        lookup_delay_ns = static_cast<std::uint64_t>(std::strtoull(argv[2], nullptr, 10));
    }
    if (argc > 3) {
        const unsigned long parsed = std::strtoul(argv[3], nullptr, 10);
        if (parsed == 0 || parsed > kMaxPolicyRules) {
            std::cerr << "rule_count must be in [1, " << kMaxPolicyRules << "]\n";
            return 1;
        }
        rule_count = static_cast<std::uint16_t>(parsed);
    }

    const PolicyCorpus corpus = build_policy_corpus(rule_count);
    const std::array<PolicyRule, kMaxPolicyRules> mutated_policy =
        make_mutated_policy(corpus.policy_5d, rule_count);
    const auto permission_map = build_rbac_hash_map(corpus.policy_3d.data(), rule_count);
    const NgacDag dag = build_ngac_dag(corpus.policy_3d.data(), rule_count);
    const auto [flattened_5d_lookup, flattened_5d_stats] =
        build_flattened_lookup_with_stats(corpus.policy_5d, mutated_policy, rule_count);
    const BuildStats hngac_5d_policy_stats =
        measure_policy_array_stats(corpus.policy_5d, mutated_policy, rule_count);

    if (!validate_flattened_lookup(
            corpus.policy_5d, flattened_5d_lookup, rule_count, corpus.requests)) {
        std::cerr << "Flattened 5D direct-lookup baseline does not match H-NGAC 5D on the "
                  << "benchmark corpus.\n";
        return 1;
    }

#ifdef HNGAC_HAVE_SQLITE
    SqliteStateLookup sqlite_lookup{};
    const bool sqlite_available = initialize_sqlite_state_lookup(sqlite_lookup, corpus.requests);
#else
    const bool sqlite_available = false;
#endif

    std::size_t canonical_allow_per_cycle = 0;
    for (const AuthorizationRequest& request : corpus.requests) {
        if (hngac_authorize(corpus.policy_5d.data(), rule_count, request)) {
            ++canonical_allow_per_cycle;
        }
    }

    std::cout << "Iterations: " << iterations << "\n";
    std::cout << "Rule count: " << rule_count << "\n";
    std::cout << "RBAC modeled external state lookup delay: " << lookup_delay_ns << " ns\n";
    std::cout << "Warmup per model: " << kWarmupIterations << " iterations\n";
    std::cout << "Scenario mix: shared 5D-aware corpus with atomic-action requests, "
              << canonical_allow_per_cycle << " allow and "
              << (corpus.requests.size() - canonical_allow_per_cycle) << " deny per cycle\n";
    std::cout << "Flattened 5D direct lookup: validated against H-NGAC 5D on "
              << corpus.requests.size() << " corpus requests\n";

    print_build_stats("H-NGAC 5D policy array", hngac_5d_policy_stats);
    print_build_stats("Flattened 5D direct lookup", flattened_5d_stats);

    const Summary rbac_hash_map = run_benchmark(
        "RBAC hash map",
        corpus.requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return authorize_rbac_hash_map(permission_map, request);
        });
    (void)rbac_hash_map;

    const Summary ngac_dag = run_benchmark(
        "NGAC-DAG traversal",
        corpus.requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return authorize_ngac_dag(dag, request);
        });

    const Summary baseline_3d = run_benchmark(
        "H-NGAC 3D",
        corpus.requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return authorize_3d(corpus.policy_3d.data(), rule_count, request);
        });

    const Summary state_4d = run_benchmark(
        "H-NGAC 4D",
        corpus.requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return hngac_authorize(corpus.policy_4d.data(), rule_count, request);
        });

    const Summary provenance_5d = run_benchmark(
        "H-NGAC 5D",
        corpus.requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return hngac_authorize(corpus.policy_5d.data(), rule_count, request);
        });

    const Summary flattened_5d = run_benchmark(
        "Flattened 5D direct lookup",
        corpus.requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return authorize_flattened_5d_lookup(flattened_5d_lookup, request);
        });

    const Summary rbac_lookup = run_benchmark(
        "RBAC + state lookup",
        corpus.requests,
        iterations,
        [&](const AuthorizationRequest& request) {
            return authorize_rbac_lookup(
                corpus.policy_rbac.data(), rule_count, request, lookup_delay_ns);
        });

    Summary sqlite_lookup_summary{};
    if (sqlite_available) {
        sqlite_lookup_summary = run_benchmark(
            "RBAC + SQLite state lookup",
            corpus.requests,
            iterations,
            [&](const AuthorizationRequest& request) {
#ifdef HNGAC_HAVE_SQLITE
                return authorize_rbac_sqlite_lookup(
                    sqlite_lookup, corpus.policy_rbac.data(), rule_count, request);
#else
                static_cast<void>(request);
                return false;
#endif
            });
    } else {
        std::cout << "RBAC + SQLite state lookup: unavailable\n";
    }

    const double overhead_4d_vs_3d =
        ((state_4d.mean_ns - baseline_3d.mean_ns) / baseline_3d.mean_ns) * 100.0;
    const double overhead_5d_vs_4d =
        ((provenance_5d.mean_ns - state_4d.mean_ns) / state_4d.mean_ns) * 100.0;
    const double overhead_5d_vs_flattened =
        ((provenance_5d.mean_ns - flattened_5d.mean_ns) / flattened_5d.mean_ns) * 100.0;
    const double p99_ratio_5d_vs_flattened =
        provenance_5d.p99_ns / flattened_5d.p99_ns;
    const double memory_ratio_flattened_vs_5d =
        static_cast<double>(flattened_5d_stats.memory_bytes) /
        static_cast<double>(hngac_5d_policy_stats.memory_bytes);
    const double reload_ratio_flattened_vs_5d =
        flattened_5d_stats.reload_mean_ns / hngac_5d_policy_stats.reload_mean_ns;
    const double dag_slowdown = ngac_dag.mean_ns / provenance_5d.mean_ns;
    const double sqlite_slowdown =
        sqlite_available ? (sqlite_lookup_summary.mean_ns / provenance_5d.mean_ns) : 0.0;
    const double modeled_rbac_slowdown = rbac_lookup.mean_ns / provenance_5d.mean_ns;

    std::cout << "4D vs 3D mean overhead: " << overhead_4d_vs_3d << "%\n";
    std::cout << "5D vs 4D mean overhead: " << overhead_5d_vs_4d << "%\n";
    std::cout << "H-NGAC 5D vs flattened 5D mean overhead: " << overhead_5d_vs_flattened << "%\n";
    std::cout << "H-NGAC 5D vs flattened 5D p99 ratio: " << p99_ratio_5d_vs_flattened << "x\n";
    std::cout << "Flattened 5D memory vs H-NGAC 5D memory: "
              << memory_ratio_flattened_vs_5d << "x\n";
    std::cout << "Flattened 5D reload cost vs H-NGAC 5D reload cost: "
              << reload_ratio_flattened_vs_5d << "x\n";
    std::cout << "NGAC-DAG vs H-NGAC 5D mean slowdown: " << dag_slowdown << "x\n";
    std::cout << "RBAC+lookup vs H-NGAC 5D mean slowdown: " << modeled_rbac_slowdown << "x\n";
    std::cout << "COMPARE|4D_vs_3D_mean_overhead_pct|" << overhead_4d_vs_3d << "\n";
    std::cout << "COMPARE|5D_vs_4D_mean_overhead_pct|" << overhead_5d_vs_4d << "\n";
    std::cout << "COMPARE|HNGAC5D_vs_flattened5D_mean_overhead_pct|" << overhead_5d_vs_flattened << "\n";
    std::cout << "COMPARE|HNGAC5D_vs_flattened5D_p99_ratio_x|" << p99_ratio_5d_vs_flattened << "\n";
    std::cout << "COMPARE|flattened5D_memory_vs_HNGAC5D_x|" << memory_ratio_flattened_vs_5d << "\n";
    std::cout << "COMPARE|flattened5D_reload_vs_HNGAC5D_x|" << reload_ratio_flattened_vs_5d << "\n";
    std::cout << "COMPARE|NGACDAG_vs_HNGAC5D_mean_slowdown_x|" << dag_slowdown << "\n";
    std::cout << "COMPARE|RBAClookup_vs_HNGAC5D_mean_slowdown_x|" << modeled_rbac_slowdown << "\n";
    if (sqlite_available) {
        std::cout << "RBAC+SQLite vs H-NGAC 5D mean slowdown: " << sqlite_slowdown << "x\n";
        std::cout << "COMPARE|RBACSQLite_vs_HNGAC5D_mean_slowdown_x|" << sqlite_slowdown << "\n";
    }

#ifdef HNGAC_HAVE_SQLITE
    destroy_sqlite_state_lookup(sqlite_lookup);
#endif
    return 0;
}
