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

void pause() noexcept {
#if defined(_MSC_VER) && (defined(_M_AMD64) || defined(_M_IX86))
  _mm_pause();
#elif defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
  __asm__ __volatile__("pause;");
#endif
}

class shared_spin_mutex {
  enum {
    unlocked_state = 0,
    reader_mask = 0x7fffffff,
    locked_state = 0x80000000
  };

  shared_spin_mutex(shared_spin_mutex const &) = delete;
  shared_spin_mutex &operator=(shared_spin_mutex const &) = delete;

public:
  shared_spin_mutex() noexcept : state(uint32_t(unlocked_state)) {}

  ~shared_spin_mutex() noexcept { assert(state == unlocked_state); }

  void lock() {
    for (;;) {
      while (state.load(std::memory_order_relaxed) != unlocked_state)
        pause();

      uint32_t expected = unlocked_state;
      if (state.compare_exchange_weak(expected, locked_state,
                                      std::memory_order_acquire,
                                      std::memory_order_relaxed))
        break;
    }
  }

  bool try_lock() noexcept {
    uint32_t expected = unlocked_state;
    if (state.compare_exchange_strong(expected, locked_state,
                                      std::memory_order_acquire,
                                      std::memory_order_relaxed))
      return true;
    else
      return false;
  }

  void unlock() noexcept {
    assert(state.load(std::memory_order_relaxed) == locked_state);
    state.store(unlocked_state, std::memory_order_release);
  }

  void lock_shared() noexcept {
    for (;;) {
      while (state.load(std::memory_order_relaxed) == locked_state)
        pause();

      /* with the mask, the cas will fail, locked exclusively */
      uint32_t current_state =
          state.load(std::memory_order_acquire) & reader_mask;
      const uint32_t next_state = current_state + 1;

      if (state.compare_exchange_weak(current_state, next_state,
                                      std::memory_order_acquire,
                                      std::memory_order_relaxed))
        break;
      pause();
    }
  }

  bool try_lock_shared() noexcept {
    /* with the mask, the cas will fail, locked exclusively */
    uint32_t current_state =
        state.load(std::memory_order_acquire) & reader_mask;
    const uint32_t next_state = current_state + 1;

    if (state.compare_exchange_strong(current_state, next_state,
                                      std::memory_order_acquire,
                                      std::memory_order_relaxed))
      return true;
    else
      return false;
  }

  bool try_lock_shared2() noexcept {
    return !(state.fetch_add(1, std::memory_order_acquire) & locked_state);
  }

  void unlock_shared() noexcept {
    state.fetch_sub(1, std::memory_order_release);
  }

private:
  std::atomic<uint32_t> state;
};

size_t g_counter;
shared_spin_mutex g_lock;


static void BM_SharedLock1(benchmark::State &state) {
  size_t tmp;

  for (auto _ : state) {
    g_lock.lock();
    g_counter++;
    g_lock.unlock();


    for (int i = 0; i < 3; i++) {
      if (g_lock.try_lock_shared()) {
        tmp = g_counter;
        g_lock.unlock_shared();
      }
    }
  }

  benchmark::DoNotOptimize(tmp);
}
BENCHMARK(BM_SharedLock1)
    ->Threads(1)
    ->Threads(2)
    ->Threads(4)
    ->Threads(8)
    ->Threads(16)
    ->Threads(32)
    ->Threads(64)
    ->Threads(128)
    ->Threads(256);

static void BM_SharedLock2(benchmark::State &state) {
  size_t tmp;

  for (auto _ : state) {
    g_lock.lock();
    g_counter++;
    g_lock.unlock();

    for (int i = 0; i < 3; i++) {
      if (g_lock.try_lock_shared2()) {
        tmp = g_counter;
        g_lock.unlock_shared();
      }
    }

    benchmark::DoNotOptimize(tmp);
  }
}
BENCHMARK(BM_SharedLock2)
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
