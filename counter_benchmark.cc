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

ib_counter_t<Integer> g_sharded_atomic_read;
static void BM_ShardedAtomicRead(benchmark::State &state) {
  auto idx = std::hash<std::thread::id>()(std::this_thread::get_id());
  for (auto _ : state) {
    auto res = static_cast<Integer>(g_sharded_atomic_read);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(BM_ShardedAtomicRead)
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

singleton_counter_array<Integer, 1> g_counter_array_read;
static void BM_SingletonCounterArrayRead(benchmark::State &state) {
  g_counter_array_read[0]++;
  for (auto _ : state) {
    auto res = g_counter_array_read.load(0);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(BM_SingletonCounterArrayRead)
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


ib_counter_t<Integer> g_sharded_atomic_reads[5];
static void BM_ShardedAtomicRead5(benchmark::State &state) {
  for (auto _ : state) {
    for (auto &counter : g_sharded_atomic_reads) {
      auto res = static_cast<Integer>(counter);
      benchmark::DoNotOptimize(res);
    }
  }
}
BENCHMARK(BM_ShardedAtomicRead5)
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

singleton_counter_array<Integer, 5> g_counter_array_read5;
static void BM_SingletonCounterArrayRead5(benchmark::State &state) {
  for (size_t i = 0; i < 5; i++)
    g_counter_array_read5[i]++;

  for (auto _ : state) {
    for (size_t i = 0; i < 5; i++) {
      auto res = g_counter_array_read5.load(i);
      benchmark::DoNotOptimize(res);
    }
  }
}
BENCHMARK(BM_SingletonCounterArrayRead5)
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

singleton_counter_array<Integer, 5> g_counter_array_read_all;
static void BM_SingletonCounterArrayRead5All(benchmark::State &state) {
  for (size_t i = 0; i < 5; i++)
    g_counter_array_read_all[i]++;

  for (auto _ : state) {
    auto res = g_counter_array_read_all.load_all();
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(BM_SingletonCounterArrayRead5All)
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

BENCHMARK_MAIN();
