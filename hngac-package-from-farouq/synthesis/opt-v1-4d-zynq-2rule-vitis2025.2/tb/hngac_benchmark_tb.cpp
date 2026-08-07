#include <array>
#include <cstdio>
#include <vector>

#include "hngac_kernel.hpp"

namespace {

using hngac::fpga::AuthorizationRequest;
using hngac::fpga::Bitmask256;
using hngac::fpga::PolicyRule;
using hngac::fpga::ProvenanceBit;
using hngac::fpga::ProvenanceMask;
using hngac::fpga::StateBit;
using hngac::fpga::StateMask;
using hngac::fpga::kMaxNodes;
using hngac::fpga::kMaxPolicyRules;
using hngac::fpga::set_bit;
using hngac::fpga::set_provenance_bit;
using hngac::fpga::set_state_bit;

struct RuleDescriptor {
    std::uint16_t subject_id = 0;
    std::uint16_t object_id = 0;
    std::uint16_t attribute_id = 0;
    StateMask required_states = 0;
    ProvenanceMask required_provenance = 0;
};

std::size_t first_set_bit_index(std::uint32_t mask, std::size_t bit_limit) {
    for (std::size_t bit = 0; bit < bit_limit; ++bit) {
        if ((mask & (std::uint32_t{1} << bit)) != 0) {
            return bit;
        }
    }
    return bit_limit;
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

void build_and_run_corpus(std::uint16_t rule_count) {
    PolicyRule policy[kMaxPolicyRules] = {};
    std::vector<AuthorizationRequest> requests;
    requests.reserve(static_cast<std::size_t>(rule_count) * 3);

    for (std::size_t i = 0; i < rule_count; ++i) {
        const RuleDescriptor descriptor = describe_rule(i);
        set_bit(policy[i].subjects, descriptor.subject_id);
        set_bit(policy[i].objects, descriptor.object_id);
        set_bit(policy[i].attributes, descriptor.attribute_id);
        policy[i].required_states = descriptor.required_states;
        policy[i].required_provenance = descriptor.required_provenance;

        requests.push_back(make_atomic_request(
            descriptor.subject_id,
            descriptor.object_id,
            descriptor.attribute_id,
            make_allow_state_mask(descriptor.required_states, i),
            make_allow_provenance_mask(descriptor.required_provenance, i)));

        if (descriptor.required_states != 0) {
            requests.push_back(make_atomic_request(
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
                requests.push_back(make_atomic_request(
                    descriptor.subject_id,
                    descriptor.object_id,
                    descriptor.attribute_id,
                    make_allow_state_mask(descriptor.required_states, i),
                    deny_provenance));
            }
        }
    }

    std::size_t allowed = 0;
    for (std::size_t i = 0; i < requests.size(); ++i) {
        bool result = hngac_authorize(policy, rule_count, requests[i]);
        if (result) {
            ++allowed;
        }
    }

    std::printf("  rule_count=%u  requests=%zu  allowed=%zu  denied=%zu\n",
        rule_count,
        requests.size(),
        allowed,
        requests.size() - allowed);
}

}  // namespace

int main() {
    const std::uint16_t rule_counts[] = {4, 10, 50, 100, 200, 500};

    for (std::uint16_t rc : rule_counts) {
        std::printf("=== %u rules ===\n", rc);
        build_and_run_corpus(rc);
    }

    std::printf("\nBenchmark corpus complete.\n");
    return 0;
}
