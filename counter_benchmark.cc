#include <benchmark/benchmark.h>

#include "distributable_counter.h"
#include "sharded_counter.h"

#include <atomic>
#include <thread>
#if defined(_ARCH_PWR8)
#include <sys/platform/ppc.h>
#endif

namespace {

using Integer = uint64_t;

std::atomic<Integer> g_simple_atomic;
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

ib_counter_t<Integer> g_sharded_atomic;
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

tls_distributed_counter<Integer, 0> g_tls_counter;
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
    ->Threads(256)
    ->Threads(512)
    ->Threads(1024)
    ->Threads(2048);

singleton_counter_array<Integer, 32> g_counter_array;
static void BM_SingletonCounterArray(benchmark::State &state) {
  for (auto _ : state) {
    g_counter_array[20]++;
  }
}
BENCHMARK(BM_SingletonCounterArray)
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->Threads(32)
    ->Threads(64)
    ->Threads(128)
    ->Threads(256)
    ->Threads(512)
    ->Threads(1024)
    ->Threads(2048);

} // namespace

BENCHMARK_MAIN();
