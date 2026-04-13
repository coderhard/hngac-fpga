#include <bitset>
#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

constexpr size_t MAX_NODES = 128;

struct Hyperedge {
    std::bitset<MAX_NODES> subjects;
    std::bitset<MAX_NODES> objects;
    std::bitset<MAX_NODES> attributes;
};

class NGACEngine {
private:
    std::vector<Hyperedge> policy;

public:
    void add_rule(const Hyperedge& edge) {
        policy.push_back(edge);
    }

    // Force inline to minimize function call overhead in stats
    inline bool authorize(size_t subject, size_t object, const std::bitset<MAX_NODES>& required_attrs) {
        for (const auto& edge : policy) {
            if (edge.subjects.test(subject) && 
                edge.objects.test(object) && 
                (required_attrs & edge.attributes) == required_attrs) {
                return true;
            }
        }
        return false;
    }
};

int main() {
    NGACEngine engine;

    // 1. Setup Policy (Same 100 rules)
    for (int i = 0; i < 100; ++i) {
        Hyperedge edge;
        edge.subjects.set(i % 20);
        edge.objects.set((i * 2) % 50);
        edge.attributes.set(i % 10);
        engine.add_rule(edge);
    }

    constexpr int ITERATIONS = 100000;
    std::vector<double> latencies;
    latencies.reserve(ITERATIONS);
    
    std::bitset<MAX_NODES> required_attrs;
    required_attrs.set(5);
    volatile bool result; 

    // 2. Warmup
    for (int i = 0; i < 1000; ++i) {
        result = engine.authorize(i % 20, (i * 3) % 50, required_attrs);
    }

    // 3. Jitter Benchmark (Measure EVERY iteration)
    for (int i = 0; i < ITERATIONS; ++i) {
        auto t1 = std::chrono::high_resolution_clock::now();
        
        result = engine.authorize(i % 20, (i * 3) % 50, required_attrs);
        
        auto t2 = std::chrono::high_resolution_clock::now();
        
        double ns = std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count();
        latencies.push_back(ns);
    }

    // 4. Analysis
    std::sort(latencies.begin(), latencies.end());
    
    double sum = std::accumulate(latencies.begin(), latencies.end(), 0.0);
    double avg = sum / ITERATIONS;
    double min_lat = latencies[0];
    double max_lat = latencies[ITERATIONS - 1];
    double p95 = latencies[static_cast<size_t>(ITERATIONS * 0.95)];
    double p99 = latencies[static_cast<size_t>(ITERATIONS * 0.99)];

    std::cout << "=== NGAC Latency Distribution (Instrumented) ===" << std::endl;
    std::cout << "NOTE: Includes ~20-50ns overhead per call due to clock read." << std::endl;
    std::cout << "Iterations: " << ITERATIONS << std::endl;
    std::cout << "Min: " << min_lat << " ns" << std::endl;
    std::cout << "Avg: " << avg << " ns" << std::endl;
    std::cout << "p95: " << p95 << " ns" << std::endl;
    std::cout << "p99: " << p99 << " ns" << std::endl;
    std::cout << "Max: " << max_lat << " ns" << std::endl;

    return 0;
}
