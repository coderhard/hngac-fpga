#include <cstdio>

#include "hngac_kernel.hpp"

namespace {

using hngac::fpga::Bitmask256;
using hngac::fpga::PolicyRule;
using hngac::fpga::hngac_authorize;
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

}  // namespace

int main() {
    PolicyRule policy[kMaxPolicyRules] = {};
    add_single_rule(policy, 0, 1, 2, 5);
    add_single_rule(policy, 1, 7, 9, 11);

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

    Bitmask256 attr5{};
    set_bit(attr5, 5);
    check("subject 1 object 2 attr 5 allowed", hngac_authorize(policy, 2, 1, 2, attr5));
    check("unknown subject denied", !hngac_authorize(policy, 2, 99, 2, attr5));
    check("wrong object denied", !hngac_authorize(policy, 2, 1, 3, attr5));

    Bitmask256 attr7{};
    set_bit(attr7, 7);
    check("wrong attribute denied", !hngac_authorize(policy, 2, 1, 2, attr7));

    Bitmask256 empty_attrs{};
    check("empty attrs allowed by design", hngac_authorize(policy, 2, 1, 2, empty_attrs));
    check("out of range subject denied", !hngac_authorize(policy, 2, 256, 2, attr5));

    std::printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
