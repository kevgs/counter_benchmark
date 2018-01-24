#include <benchmark/benchmark.h>

#include "distributable_counter.h"
#include "sharded_counter.h"

#include <atomic>
#include <thread>

std::atomic<uint64_t> g_simple_atomic;
static void BM_SimpleAtomic(benchmark::State &state) {
  for (auto _ : state) {
    g_simple_atomic.fetch_add(1, std::memory_order_relaxed);
  }
}
BENCHMARK(BM_SimpleAtomic)
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->Threads(32)
    ->Threads(64)
    ->Threads(128)
    ->Threads(256);

ib_counter_t<uint64_t> g_sharded_atomic;
static void BM_ShardedAtomic(benchmark::State &state) {
  auto idx = std::hash<std::thread::id>()(std::this_thread::get_id());
  for (auto _ : state) {
    g_sharded_atomic.add(idx, 1);
  }
}
BENCHMARK(BM_ShardedAtomic)
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->Threads(32)
    ->Threads(64)
    ->Threads(128)
    ->Threads(256);

tls_distributed_counter<uint64_t, 0> g_tls_counter;
static void BM_TlsCounter(benchmark::State &state) {
  for (auto _ : state) {
    g_tls_counter++;
  }
}
BENCHMARK(BM_TlsCounter)
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->Threads(32)
    ->Threads(64)
    ->Threads(128)
    ->Threads(256);

BENCHMARK_MAIN();
