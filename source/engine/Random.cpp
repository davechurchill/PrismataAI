#include "Random.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <random>
#include <thread>

namespace
{
    std::atomic<uint64_t> g_baseSeed(
        static_cast<uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::atomic<uint64_t> g_seedSequence(0);

    uint64_t mixSeed(uint64_t x)
    {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }

    uint64_t nextThreadSeed()
    {
        const uint64_t sequence = g_seedSequence.fetch_add(1, std::memory_order_relaxed);
        const uint64_t threadHash = static_cast<uint64_t>(std::hash<std::thread::id>{}(std::this_thread::get_id()));

        return mixSeed(g_baseSeed.load(std::memory_order_relaxed) + sequence + threadHash);
    }

    std::mt19937_64 & engine()
    {
        thread_local std::mt19937_64 rng(nextThreadSeed());
        return rng;
    }
}

namespace Prismata
{
namespace Random
{
    void Seed(uint64_t seed)
    {
        g_baseSeed.store(seed, std::memory_order_relaxed);
        g_seedSequence.store(0, std::memory_order_relaxed);
        engine().seed(nextThreadSeed());
    }

    size_t Int(size_t exclusiveMax)
    {
        PRISMATA_ASSERT(exclusiveMax > 0, "Random::Int called with exclusiveMax 0");

        std::uniform_int_distribution<size_t> dist(0, exclusiveMax - 1);
        return dist(engine());
    }
}
}
