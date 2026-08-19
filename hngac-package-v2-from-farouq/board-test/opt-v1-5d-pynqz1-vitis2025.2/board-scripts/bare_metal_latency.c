/*
 * Round-trip + AXI Timer capture latency on PYNQ-Z1.
 *
 * Three measurements per rule count:
 *   1. Round-trip (CLOCK_MONOTONIC): write_rule_count -> ap_start -> poll -> read_result
 *   2. AXI Timer capture (differential): hardware-latched counter at ap_done,
 *      minus a rule_count=1 baseline to cancel the ap_start write overhead
 *   3. Fabric cycles: capture differential converted to clock cycles
 *
 * Build on the board:
 *   gcc -O2 -o roundtrip_latency roundtrip_latency.c
 *   sudo ./roundtrip_latency
 */

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define IP_BASE      0x40000000
#define BRAM0_BASE   0x40010000
#define BRAM1_BASE   0x40020000
#define TIMER_BASE   0x41C00000
#define MAP_SIZE     0x10000

/* HLS IP registers */
#define AP_CTRL      0x00
#define AP_RETURN    0x10
#define RULE_COUNT   0x18
#define RB           0x40
#define GIER         0x04
#define IP_IER       0x08
#define IP_ISR       0x0C

/* AXI Timer registers (PG079) */
#define TCSR0        0x00
#define TLR0         0x04
#define TCR0         0x08

/* TCSR0 bits */
#define TCSR_MDT     0x001
#define TCSR_CAPT    0x008
#define TCSR_ARHT    0x010
#define TCSR_LOAD    0x020
#define TCSR_ENT     0x080
#define TCSR_TINT    0x100

#define CAPTURE_MODE (TCSR_MDT | TCSR_CAPT | TCSR_ENT)

/* BRAM layout */
#define WPR          32
#define BPR          128
#define KMAX         256
#define SUBJ_OFF     0
#define OBJ_OFF      8
#define ATTR_OFF     16
#define STATE_OFF    24
#define PROV_OFF     25

#define TRIALS       100
#define CLK_NS       10.0

static volatile uint32_t *ip;
static volatile uint32_t *bram0;
static volatile uint32_t *bram1;
static volatile uint32_t *timer;

static inline uint32_t reg_read(volatile uint32_t *base, int offset) {
    return base[offset / 4];
}

static inline void reg_write(volatile uint32_t *base, int offset, uint32_t val) {
    base[offset / 4] = val;
}

static inline uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

/* ---- BRAM access ---- */

static void clear_rule(int idx) {
    for (int w = 0; w < WPR; w++) {
        bram0[idx * BPR / 4 + w] = 0;
        bram1[idx * BPR / 4 + w] = 0;
    }
}

static void set_field_bit(int idx, int field_off, int bit) {
    int word = idx * BPR / 4 + field_off + bit / 32;
    uint32_t val = bram0[word] | (1u << (bit % 32));
    bram0[word] = val;
    bram1[word] = val;
}

static void set_word(int idx, int off, uint32_t val) {
    bram0[idx * BPR / 4 + off] = val;
    bram1[idx * BPR / 4 + off] = val;
}

/* ---- IP access ---- */

static void write_request(int subj, int obj, int attr, uint32_t state, uint32_t prov) {
    reg_write(ip, RB + 0 * 4, (subj & 0xFFFF) | ((obj & 0xFFFF) << 16));
    reg_write(ip, RB + 1 * 4, 0);
    for (int w = 2; w < 10; w++)
        reg_write(ip, RB + w * 4, 0);
    reg_write(ip, RB + (2 + attr / 32) * 4, 1u << (attr % 32));
    reg_write(ip, RB + 10 * 4, state);
    reg_write(ip, RB + 11 * 4, prov);
}

static int run_kernel(int rule_count) {
    reg_write(ip, RULE_COUNT, rule_count & 0xFFFF);
    reg_write(ip, AP_CTRL, 1);
    while ((reg_read(ip, AP_CTRL) & 0x2) == 0)
        ;
    return reg_read(ip, AP_RETURN) & 0x1;
}

/* ---- round-trip measurement ---- */

static uint64_t measure_roundtrip(int subj, int obj, int attr,
                                  uint32_t state, uint32_t prov,
                                  int rule_count) {
    write_request(subj, obj, attr, state, prov);
    uint64_t t0 = now_ns();
    reg_write(ip, RULE_COUNT, rule_count & 0xFFFF);
    reg_write(ip, AP_CTRL, 1);
    while ((reg_read(ip, AP_CTRL) & 0x2) == 0)
        ;
    reg_read(ip, AP_RETURN);
    uint64_t t1 = now_ns();
    return t1 - t0;
}

/* ---- AXI Timer capture ---- */

static void enable_ip_interrupt(void) {
    reg_write(ip, GIER, 1);
    reg_write(ip, IP_IER, 1);
}

static int clear_ip_interrupt(void) {
    for (int i = 0; i < 4; i++) {
        if (reg_read(ip, IP_ISR) == 0)
            return 1;
        reg_write(ip, IP_ISR, 1);
    }
    return 0;
}

static int run_captured(int rule_count) {
    if (!clear_ip_interrupt())
        return -1;

    reg_write(ip, RULE_COUNT, rule_count & 0xFFFF);

    reg_write(timer, TCSR0, 0);
    reg_write(timer, TCSR0, TCSR_TINT);
    reg_write(timer, TLR0, 0);
    reg_write(timer, TCSR0, TCSR_LOAD);
    reg_write(timer, TCSR0, CAPTURE_MODE);

    reg_write(ip, AP_CTRL, 1);
    while ((reg_read(ip, AP_CTRL) & 0x2) == 0)
        ;

    uint32_t status = reg_read(timer, TCSR0);
    uint32_t captured = reg_read(timer, TLR0);
    reg_write(timer, TCSR0, 0);
    reg_write(timer, TCSR0, TCSR_TINT);

    if (!(status & TCSR_TINT))
        return -1;
    return (int)captured;
}

static int measure_captured_min(int rule_count, int trials) {
    int min_cap = -1;
    for (int t = 0; t < trials; t++) {
        int cap = run_captured(rule_count);
        if (cap >= 0 && (min_cap < 0 || cap < min_cap))
            min_cap = cap;
    }
    return min_cap;
}

/* ---- corpus generator (matches SW benchmark and cosim exactly) ---- */

typedef struct {
    uint16_t subject_id;
    uint16_t object_id;
    uint16_t attribute_id;
    uint32_t required_states;
    uint32_t required_provenance;
} RuleDescriptor;

static RuleDescriptor describe_rule(int i) {
    RuleDescriptor d = {0};
    d.subject_id = i % KMAX;
    d.object_id = ((i % KMAX) + ((i / KMAX) * 97)) % KMAX;
    d.attribute_id = (i * 17 + 5) % KMAX;
    int ps = i % 4, ss = (i + 1) % 4;
    int pp = i % 3, sp = (i + 1) % 3;
    switch (i % 6) {
    case 0: d.required_states = 1u << ps; d.required_provenance = 1u << pp; break;
    case 1: d.required_states = (1u << ps) | (1u << ss); d.required_provenance = 1u << pp; break;
    case 2: d.required_provenance = 1u << pp; break;
    case 3: d.required_states = 1u << ps; d.required_provenance = (1u << pp) | (1u << sp); break;
    case 4: d.required_states = (1u << ps) | (1u << ss); break;
    case 5: d.required_states = 1u << ((i + 2) % 4); d.required_provenance = 1u << ((i + 2) % 3); break;
    }
    return d;
}

static int first_set_bit(uint32_t mask, int limit) {
    for (int b = 0; b < limit; b++)
        if (mask & (1u << b)) return b;
    return limit;
}

static uint32_t allow_state(uint32_t req, int seed) {
    if (req == 0) return 1u << (seed % 4);
    return req | (1u << ((seed + 1) % 4));
}

static uint32_t allow_prov(uint32_t req, int seed) {
    if (req == 0) return 1u << (seed % 3);
    return (1u << first_set_bit(req, 3)) | (1u << ((seed + 1) % 3));
}

static uint32_t deny_state(uint32_t req) {
    if (req == 0) return 0;
    return req & (req - 1);
}

static uint32_t deny_prov(uint32_t req, int seed) {
    if (req == 0) return 0;
    for (int off = 0; off < 3; off++) {
        uint32_t c = 1u << ((seed + off) % 3);
        if ((req & c) == 0) return c;
    }
    return 0;
}

/* golden values from csim */
static const int EXPECTED_REQUESTS[] = {11, 27, 134, 267, 534, 1334};
static const int EXPECTED_ALLOWS[]   = { 4, 10,  50, 100, 200,  500};

typedef struct { int subj, obj, attr; uint32_t state, prov; } Request;

static int build_corpus_requests(int n, Request *out, int max_out) {
    int count = 0;
    for (int i = 0; i < n && count < max_out; i++) {
        RuleDescriptor d = describe_rule(i);
        clear_rule(i);
        set_field_bit(i, SUBJ_OFF, d.subject_id);
        set_field_bit(i, OBJ_OFF, d.object_id);
        set_field_bit(i, ATTR_OFF, d.attribute_id);
        if (d.required_states)
            set_word(i, STATE_OFF, d.required_states);
        if (d.required_provenance)
            set_word(i, PROV_OFF, d.required_provenance);

        out[count++] = (Request){d.subject_id, d.object_id, d.attribute_id,
                                 allow_state(d.required_states, i),
                                 allow_prov(d.required_provenance, i)};
        if (d.required_states && count < max_out)
            out[count++] = (Request){d.subject_id, d.object_id, d.attribute_id,
                                     deny_state(d.required_states),
                                     allow_prov(d.required_provenance, i)};
        if (d.required_provenance && count < max_out) {
            uint32_t dp = deny_prov(d.required_provenance, i);
            if (dp)
                out[count++] = (Request){d.subject_id, d.object_id, d.attribute_id,
                                         allow_state(d.required_states, i), dp};
        }
    }
    return count;
}

/* ---- load worst-case policy (no rule matches request 0,0,0) ---- */

static void load_worst_case_policy(int n) {
    for (int r = 0; r < n; r++) {
        clear_rule(r);
        set_field_bit(r, SUBJ_OFF, 100);
        set_field_bit(r, OBJ_OFF, 100);
        set_field_bit(r, ATTR_OFF, 0);
    }
}

int main(void) {
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) { perror("open /dev/mem (run as root)"); return 1; }

    ip    = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, IP_BASE);
    bram0 = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BRAM0_BASE);
    bram1 = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BRAM1_BASE);
    timer = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, TIMER_BASE);
    if (ip == MAP_FAILED || bram0 == MAP_FAILED ||
        bram1 == MAP_FAILED || timer == MAP_FAILED) {
        perror("mmap"); return 1;
    }

    printf("========================================================================\n");
    printf("HNGAC 5D opt-v1 -- bare-metal latency + functional verification (C)\n");
    printf("PYNQ-Z1 @ 100 MHz, %d trials per latency measurement\n", TRIALS);
    printf("========================================================================\n");

    int rule_counts[] = {4, 10, 50, 100, 200, 500};
    int n_rc = sizeof(rule_counts) / sizeof(rule_counts[0]);

    /* ---- [0] Functional verification (all 2307 requests) ---- */
    printf("\n[0] Functional verification (full corpus, 2307 requests)\n\n");
    printf("%6s | %5s | %5s | %5s | %6s\n",
           "rules", "reqs", "allow", "deny", "result");
    printf("-------+-------+-------+-------+-------\n");

    Request reqs[2048];
    int all_pass = 1;

    for (int rc_i = 0; rc_i < n_rc; rc_i++) {
        int n = rule_counts[rc_i];
        int nreqs = build_corpus_requests(n, reqs, 2048);
        int allows = 0;

        for (int r = 0; r < nreqs; r++) {
            write_request(reqs[r].subj, reqs[r].obj, reqs[r].attr,
                          reqs[r].state, reqs[r].prov);
            allows += run_kernel(n);
        }

        int denies = nreqs - allows;
        int ok = (nreqs == EXPECTED_REQUESTS[rc_i]) &&
                 (allows == EXPECTED_ALLOWS[rc_i]);
        if (!ok) all_pass = 0;

        printf("%6d | %5d | %5d | %5d | %6s\n",
               n, nreqs, allows, denies, ok ? "PASS" : "FAIL");
    }

    if (!all_pass) {
        printf("\nFUNCTIONAL VERIFICATION FAILED\n");
        return 1;
    }
    printf("\nAll corpus decisions match csim/cosim.\n");

    /* ---- check AXI Timer capture ---- */
    enable_ip_interrupt();
    write_request(0, 0, 0, 0, 0);
    int probe = run_captured(4);
    int capture_ok = (probe >= 0);
    printf("\nAXI Timer capture: %s\n", capture_ok ? "active" : "NOT WIRED");

    /* ---- [1] Round-trip latency (full corpus, all requests) ---- */
    printf("\n[1] Round-trip latency (CLOCK_MONOTONIC, full corpus)\n\n");
    printf("%6s | %5s | %8s | %8s | %8s\n",
           "rules", "reqs", "min(ns)", "avg(ns)", "max(ns)");
    printf("-------+-------+----------+----------+---------\n");

    for (int rc_i = 0; rc_i < n_rc; rc_i++) {
        int n = rule_counts[rc_i];
        int nreqs = build_corpus_requests(n, reqs, 2048);

        /* warmup */
        for (int r = 0; r < nreqs; r++)
            measure_roundtrip(reqs[r].subj, reqs[r].obj, reqs[r].attr,
                              reqs[r].state, reqs[r].prov, n);

        uint64_t min_ns = UINT64_MAX, max_ns = 0, sum_ns = 0;
        int total = 0;
        for (int t = 0; t < TRIALS; t++) {
            for (int r = 0; r < nreqs; r++) {
                uint64_t lat = measure_roundtrip(reqs[r].subj, reqs[r].obj,
                                                 reqs[r].attr, reqs[r].state,
                                                 reqs[r].prov, n);
                if (lat < min_ns) min_ns = lat;
                if (lat > max_ns) max_ns = lat;
                sum_ns += lat;
                total++;
            }
        }
        printf("%6d | %5d | %8llu | %8.0f | %8llu\n",
               n, nreqs, (unsigned long long)min_ns,
               (double)sum_ns / total, (unsigned long long)max_ns);
    }

    /* ---- [2] AXI Timer capture (full corpus, all requests) ---- */
    if (capture_ok) {
        printf("\n[2] AXI Timer capture (full corpus, hardware-latched)\n\n");
        printf("%6s | %5s | %8s | %8s | %8s\n",
               "rules", "reqs", "min(cyc)", "avg(cyc)", "max(cyc)");
        printf("-------+-------+----------+----------+---------\n");

        for (int rc_i = 0; rc_i < n_rc; rc_i++) {
            int n = rule_counts[rc_i];
            int nreqs = build_corpus_requests(n, reqs, 2048);

            int min_cap = -1, max_cap = 0;
            long sum_cap = 0;
            int count = 0;

            for (int r = 0; r < nreqs; r++) {
                write_request(reqs[r].subj, reqs[r].obj, reqs[r].attr,
                              reqs[r].state, reqs[r].prov);
                int cap = measure_captured_min(n, 10);
                if (cap >= 0) {
                    if (min_cap < 0 || cap < min_cap) min_cap = cap;
                    if (cap > max_cap) max_cap = cap;
                    sum_cap += cap;
                    count++;
                }
            }

            if (count > 0)
                printf("%6d | %5d | %8d | %8.0f | %8d\n",
                       n, nreqs, min_cap, (double)sum_cap / count, max_cap);
            else
                printf("%6d | %5d | %8s | %8s | %8s\n",
                       n, nreqs, "-", "-", "-");
        }
    }

    munmap((void *)ip, MAP_SIZE);
    munmap((void *)bram0, MAP_SIZE);
    munmap((void *)bram1, MAP_SIZE);
    munmap((void *)timer, MAP_SIZE);
    close(fd);
    return 0;
}
