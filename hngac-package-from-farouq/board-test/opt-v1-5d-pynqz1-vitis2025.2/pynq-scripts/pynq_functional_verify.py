"""
Functional verification of the HNGAC 5D opt-v1 kernel on real silicon.

Runs build_policy_corpus() at 4/10/50/100/200/500 rules -- byte-identical rules
and requests to the SW benchmark and the cosim testbench -- and checks every
decision against golden csim values. 2307 requests total.

This asserts rather than reports: a mismatch raises SystemExit, so a board that
silently returns wrong decisions fails the run. That matters, because the BRAM
address-slice regression produced correct-looking BRAM readback while the IP
saw zeros, and a print-only check would have passed it.

Latency is deliberately not measured here. See pynq_capture_latency.py.
"""

import os

from pynq import Overlay
from pynq.pl_server.embedded_device import EmbeddedDevice

# Absolute path: sudo runs with cwd=/home/xilinx, which holds older bitstreams.
# A relative name silently loads whichever stale .bit is sitting there.
BITSTREAM = os.path.join(os.path.dirname(os.path.abspath(__file__)), "hngac_test.bit")

device = EmbeddedDevice()
overlay = Overlay(BITSTREAM, device=device)
ip = overlay.hngac_authorize_0
bram0 = overlay.axi_bram_ctrl_0
bram1 = overlay.axi_bram_ctrl_1

AP_CTRL, AP_RETURN, RULE_COUNT, RB = 0x00, 0x10, 0x18, 0x40
WPR, BPR = 32, 128
SUBJ_OFF, OBJ_OFF, ATTR_OFF, STATE_OFF, PROV_OFF = 0, 8, 16, 24, 25
KMAX = 256

# Rule counts must match the SW benchmark and cosim benchmark exactly.
CORPUS_SIZES = [4, 10, 50, 100, 200, 500]

# Golden values from csim (and reproduced by cosim). The corpus generator is
# deterministic, so these are exact, not tolerances. Checked, not just printed:
# a board that silently returns the wrong decisions must fail the run.
EXPECTED_REQUESTS = {4: 11, 10: 27, 50: 134, 100: 267, 200: 534, 500: 1334}
EXPECTED_ALLOWS   = {4: 4,  10: 10, 50: 50,  100: 100, 200: 200, 500: 500}


# ---------------------------------------------------------------- BRAM access
def clear_rule(idx):
    for w in range(WPR):
        bram0.write(idx * BPR + w * 4, 0)
        bram1.write(idx * BPR + w * 4, 0)


def set_field_bit(idx, field_off, bit):
    addr = idx * BPR + (field_off + bit // 32) * 4
    cur = bram0.read(addr) | (1 << (bit % 32))
    bram0.write(addr, cur)
    bram1.write(addr, cur)


def set_word(idx, off, val):
    bram0.write(idx * BPR + off * 4, val)
    bram1.write(idx * BPR + off * 4, val)


def write_request(subj, obj, attr, state=0, prov=0):
    """384-bit request; word 1 is alignment padding (Bitmask256 is 64-bit aligned)."""
    ip.write(RB + 0 * 4, (subj & 0xFFFF) | ((obj & 0xFFFF) << 16))
    ip.write(RB + 1 * 4, 0)
    for w in range(2, 10):
        ip.write(RB + w * 4, 0)
    ip.write(RB + (2 + attr // 32) * 4, 1 << (attr % 32))
    ip.write(RB + 10 * 4, state)
    ip.write(RB + 11 * 4, prov)


def run(rule_count):
    ip.write(RULE_COUNT, rule_count & 0xFFFF)
    ip.write(AP_CTRL, 1)
    while (ip.read(AP_CTRL) & 0x2) == 0:
        pass
    return ip.read(AP_RETURN) & 0x1


# ------------------------------------------- corpus (mirrors describe_rule())
def describe_rule(i):
    subj = i % KMAX
    obj = ((i % KMAX) + ((i // KMAX) * 97)) % KMAX
    attr = (i * 17 + 5) % KMAX
    state = prov = 0
    ps, ss = i % 4, (i + 1) % 4
    pp, sp = i % 3, (i + 1) % 3
    k = i % 6
    if k == 0:
        state, prov = 1 << ps, 1 << pp
    elif k == 1:
        state, prov = (1 << ps) | (1 << ss), 1 << pp
    elif k == 2:
        prov = 1 << pp
    elif k == 3:
        state, prov = 1 << ps, (1 << pp) | (1 << sp)
    elif k == 4:
        state = (1 << ps) | (1 << ss)
    else:
        state, prov = 1 << ((i + 2) % 4), 1 << ((i + 2) % 3)
    return subj, obj, attr, state, prov


def first_set_bit(mask, limit):
    for b in range(limit):
        if mask & (1 << b):
            return b
    return limit


def allow_state(req, seed):
    if req == 0:
        return 1 << (seed % 4)
    return req | (1 << ((seed + 1) % 4))


def allow_prov(req, seed):
    if req == 0:
        return 1 << (seed % 3)
    return (1 << first_set_bit(req, 3)) | (1 << ((seed + 1) % 3))


def deny_state(req):
    return 0 if req == 0 else req & (req - 1)


def deny_prov(req, seed):
    if req == 0:
        return 0
    for off in range(3):
        c = 1 << ((seed + off) % 3)
        if (req & c) == 0:
            return c
    return 0


def load_corpus(n):
    """Write n rules, return the matching request list (allow + deny mix)."""
    reqs = []
    for i in range(n):
        subj, obj, attr, state, prov = describe_rule(i)
        clear_rule(i)
        set_field_bit(i, SUBJ_OFF, subj)
        set_field_bit(i, OBJ_OFF, obj)
        set_field_bit(i, ATTR_OFF, attr)
        if state:
            set_word(i, STATE_OFF, state)
        if prov:
            set_word(i, PROV_OFF, prov)

        reqs.append((subj, obj, attr, allow_state(state, i), allow_prov(prov, i)))
        if state:
            reqs.append((subj, obj, attr, deny_state(state), allow_prov(prov, i)))
        if prov:
            d = deny_prov(prov, i)
            if d:
                reqs.append((subj, obj, attr, allow_state(state, i), d))
    return reqs


# --------------------------------------------------------------------- report
print("=" * 74)
print("HNGAC 5D opt-v1 -- functional verification on PYNQ-Z1 silicon")
print("=" * 74)

print("\nCorpus -- same rules/requests as SW benchmark and cosim\n")
print(f"{'rules':>6} | {'reqs':>5} | {'allow':>5} | {'deny':>5} | {'result':>6}")
print(f"{'-'*6}-+-{'-'*5}-+-{'-'*5}-+-{'-'*5}-+-{'-'*6}")

failures = []
for n in CORPUS_SIZES:
    reqs = load_corpus(n)
    allows = 0
    for (s, o, a, st, pv) in reqs:
        write_request(s, o, a, st, pv)
        allows += run(n)
    denies = len(reqs) - allows
    ok = (len(reqs) == EXPECTED_REQUESTS[n]) and (allows == EXPECTED_ALLOWS[n])
    if not ok:
        failures.append(f"{n} rules: {len(reqs)} reqs / {allows} allows, "
                        f"expected {EXPECTED_REQUESTS[n]} / {EXPECTED_ALLOWS[n]}")
    print(f"{n:>6} | {len(reqs):>5} | {allows:>5} | {denies:>5} | {'PASS' if ok else 'FAIL':>6}")

if failures:
    raise SystemExit("FUNCTIONAL VERIFICATION FAILED\n  " + "\n  ".join(failures))
print("\nAll corpus decisions match csim/cosim.")
