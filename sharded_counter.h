#include <atomic>
#include <cstdint>

static const size_t CACHE_LINE_SIZE = 64;

/** Default number of slots to use in ib_counter_t */
#define IB_N_SLOTS 128

template <typename Type, int N = IB_N_SLOTS> struct ib_counter_t {
  /** Increment the counter by 1. */
  void inc() { add(1); }

  /** Increment the counter by 1.
  @param[in]	index	a reasonably thread-unique identifier */
  void inc(size_t index) { add(index, 1); }

  /** Add to the counter.
  @param[in]	index	a reasonably thread-unique identifier
  @param[in]	n	amount to be added */
  void add(size_t index, Type n) {
    index = index % N;

    m_counter[index].value.fetch_add(n, std::memory_order_relaxed);
  }

  /* @return total value - not 100% accurate, since it is relaxed atomic*/
  operator Type() const {
    Type total = 0;

    for (const auto &counter : m_counter) {
      total += counter.value.load(std::memory_order_relaxed);
    }

    return total;
  }

private:
  /** Atomic which occupies whole CPU cache line.
  Note: We rely on the default constructor of std::atomic and
  do not explicitly initialize the contents. This works for us,
  because ib_counter_t is only intended for usage with global
  memory that is allocated from the .bss and thus guaranteed to
  be zero-initialized by the run-time environment.
  @see srv_stats
  @see rw_lock_stats */
  struct ib_counter_element_t {
    alignas(CACHE_LINE_SIZE) std::atomic<Type> value;
  };
  static_assert(sizeof(ib_counter_element_t) == CACHE_LINE_SIZE, "");

  /** Array of counter elements */
  alignas(CACHE_LINE_SIZE) ib_counter_element_t m_counter[N];
};
