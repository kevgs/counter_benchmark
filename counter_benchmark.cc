#include <benchmark/benchmark.h>

#include <atomic>

#define CACHE_LINE_SIZE (64)
#define MY_ALIGNED(A) alignas(A)
#define UNIV_NOTHROW

using ulint = size_t;
using ulonglong = uint64_t;

#define UT_ARR_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/** Default number of slots to use in ib_counter_t */
#define IB_N_SLOTS		64

ulonglong my_timer_cycles(void)
{
#if defined(__GNUC__) && defined(__i386__)
  /* This works much better if compiled with "gcc -O3". */
  ulonglong result;
  __asm__ __volatile__ ("rdtsc" : "=A" (result));
  return result;
#elif defined(__SUNPRO_C) && defined(__i386)
  __asm("rdtsc");
#elif defined(__GNUC__) && defined(__x86_64__)
  ulonglong result;
  __asm__ __volatile__ ("rdtsc\n\t" \
                        "shlq $32,%%rdx\n\t" \
                        "orq %%rdx,%%rax"
                        : "=a" (result) :: "%edx");
  return result;
#elif defined(HAVE_ASM_MSR_H) && defined(HAVE_RDTSCLL)
  {
    ulonglong result;
    rdtscll(result);
    return result;
  }
#elif defined(_WIN32) && defined(_M_IX86)
  __asm {rdtsc};
#elif defined(_WIN64) && defined(_M_X64)
  /* For 64-bit Windows: unsigned __int64 __rdtsc(); */
  return __rdtsc();
#elif defined(__INTEL_COMPILER) && defined(__ia64__) && defined(HAVE_IA64INTRIN_H)
  return (ulonglong) __getReg(_IA64_REG_AR_ITC); /* (3116) */
#elif defined(__GNUC__) && defined(__ia64__)
  {
    ulonglong result;
    __asm __volatile__ ("mov %0=ar.itc" : "=r" (result));
    return result;
  }
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__POWERPC__) || (defined(_POWER) && defined(_AIX52))) && (defined(__64BIT__) || defined(_ARCH_PPC64))
  {
    ulonglong result;
    __asm __volatile__ ("mftb %0" : "=r" (result));
    return result;
  }
#elif defined(__GNUC__) && (defined(__powerpc__) || defined(__POWERPC__) || (defined(_POWER) && defined(_AIX52))) && (!defined(__64BIT__) && !defined(_ARCH_PPC64))
  {
    /*
      mftbu means "move from time-buffer-upper to result".
      The loop is saying: x1=upper, x2=lower, x3=upper,
      if x1!=x3 there was an overflow so repeat.
    */
    unsigned int x1, x2, x3;
    ulonglong result;
    for (;;)
    {
       __asm __volatile__ ( "mftbu %0" : "=r"(x1) );
       __asm __volatile__ ( "mftb %0" : "=r"(x2) );
       __asm __volatile__ ( "mftbu %0" : "=r"(x3) );
       if (x1 == x3) break;
    }
    result = x1;
    return ( result << 32 ) | x2;
  }
#elif (defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(__sparcv9) && defined(_LP64) && !defined(__SunOS_5_7)
  return (my_timer_cycles_il_sparc64());
#elif (defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(_ILP32) && !defined(__SunOS_5_7)
  return (my_timer_cycles_il_sparc32());
#elif (defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(__i386) && defined(_ILP32)
  /* This is probably redundant for __SUNPRO_C. */
  return (my_timer_cycles_il_i386());
#elif (defined(__SUNPRO_CC) || defined(__SUNPRO_C)) && defined(__x86_64) && defined(_LP64)
  return (my_timer_cycles_il_x86_64());
#elif defined(__GNUC__) && defined(__sparcv9) && defined(_LP64)  && (__GNUC__>2)
  {
    ulonglong result;
    __asm __volatile__ ("rd %%tick,%0" : "=r" (result));
    return result;
  }
#elif defined(__GNUC__) && defined(__sparc__) && !defined(_LP64) && (__GNUC__>2)
  {
      union {
              ulonglong wholeresult;
              struct {
                      ulong high;
                      ulong low;
              }       splitresult;
      } result;
    __asm __volatile__ ("rd %%tick,%1; srlx %1,32,%0" : "=r" (result.splitresult.high), "=r" (result.splitresult.low));
    return result.wholeresult;
  }
#elif defined(__sgi) && defined(HAVE_CLOCK_GETTIME) && defined(CLOCK_SGI_CYCLE)
  {
    struct timespec tp;
    clock_gettime(CLOCK_SGI_CYCLE, &tp);
    return (ulonglong) tp.tv_sec * 1000000000 + (ulonglong) tp.tv_nsec;
  }
#elif defined(__GNUC__) && defined(__s390__)
  /* covers both s390 and s390x */
  {
    ulonglong result;
    __asm__ __volatile__ ("stck %0" : "=Q" (result) : : "cc");
    return result;
  }
#elif defined(HAVE_SYS_TIMES_H) && defined(HAVE_GETHRTIME)
  /* gethrtime may appear as either cycle or nanosecond counter */
  return (ulonglong) gethrtime();
#else
  return 0;
#endif
}

typedef pthread_t		os_thread_id_t;

os_thread_id_t
os_thread_get_curr_id(void)
/*=======================*/
{
	return(pthread_self());
}


/** Get the offset into the counter array. */
template <typename Type, int N>
struct generic_indexer_t {
        /** @return offset within m_counter */
        static size_t offset(size_t index) UNIV_NOTHROW
	{
                return(((index % N) + 1) * (CACHE_LINE_SIZE / sizeof(Type)));
        }
};

/** Use the result of my_timer_cycles(), which mainly uses RDTSC for cycles,
to index into the counter array. See the comments for my_timer_cycles() */
template <typename Type=ulint, int N=1>
struct counter_indexer_t : public generic_indexer_t<Type, N> {
	/** @return result from RDTSC or similar functions. */
	static size_t get_rnd_index() UNIV_NOTHROW
	{
		size_t	c = static_cast<size_t>(my_timer_cycles());

		if (c != 0) {
			return(c);
		} else {
			/* We may go here if my_timer_cycles() returns 0,
			so we have to have the plan B for the counter. */
			return(size_t(os_thread_get_curr_id()));
		}
	}

	/** @return a random offset to the array */
	static size_t get_rnd_offset() UNIV_NOTHROW
	{
		return(generic_indexer_t<Type, N>::offset(get_rnd_index()));
	}
};

#define	default_indexer_t	counter_indexer_t

/** Class for using fuzzy counters. The counter is not protected by any
mutex and the results are not guaranteed to be 100% accurate but close
enough. Creates an array of counters and separates each element by the
CACHE_LINE_SIZE bytes */
template <
	typename Type,
	int N = IB_N_SLOTS,
	template<typename, int> class Indexer = default_indexer_t>
struct MY_ALIGNED(CACHE_LINE_SIZE) ib_counter_t
{
	/** Increment the counter by 1. */
	void inc() UNIV_NOTHROW { add(1); }

	/** Increment the counter by 1.
	@param[in]	index	a reasonably thread-unique identifier */
	void inc(size_t index) UNIV_NOTHROW { add(index, 1); }

	/** Add to the counter.
	@param[in]	n	amount to be added */
	void add(Type n) UNIV_NOTHROW { add(m_policy.get_rnd_offset(), n); }

	/** Add to the counter.
	@param[in]	index	a reasonably thread-unique identifier
	@param[in]	n	amount to be added */
	void add(size_t index, Type n) UNIV_NOTHROW {
		size_t	i = m_policy.offset(index);

		m_counter[i] += n;
	}

	/* @return total value - not 100% accurate, since it is not atomic. */
	operator Type() const UNIV_NOTHROW {
		Type	total = 0;

		for (size_t i = 0; i < N; ++i) {
			total += m_counter[m_policy.offset(i)];
		}

		return(total);
	}

private:
	/** Indexer into the array */
	Indexer<Type, N>m_policy;

        /** Slot 0 is unused. */
	Type		m_counter[(N + 1) * (CACHE_LINE_SIZE / sizeof(Type))];
};

struct alignas(CACHE_LINE_SIZE) SimpleCounter {
  SimpleCounter() : m_counter(0) {}

  void inc() { add(1); }

  void add(uint64_t n) { m_counter.fetch_add(n, std::memory_order_seq_cst); }

  operator uint64_t() const {
    return m_counter.load(std::memory_order_relaxed);
  }

private:
  std::atomic<uint64_t> m_counter;
};

using ComplexCounter = ib_counter_t<uint64_t>;

template <class STATE, class COUNTER>
void Write(STATE &state, COUNTER &counter) {
  for (auto _ : state) {
    counter.inc();
  }
}

template <class STATE, class COUNTER>
void Read(STATE &state, COUNTER &counter) {
  uint64_t __;
  for (auto _ : state) {
    benchmark::DoNotOptimize(__ = counter);
    benchmark::ClobberMemory();
  }
}

template <class STATE, class COUNTER>
void MostlyWrite(STATE &state, COUNTER &counter) {
  uint64_t __;
  for (auto _ : state) {
    for (size_t i = 0; i < 9; i++)
      counter.inc();
    benchmark::DoNotOptimize(__ = counter);
  }
}

template <class STATE, class COUNTER>
void MostlyRead(STATE &state, COUNTER &counter) {
  uint64_t __;
  for (auto _ : state) {
    for (size_t i = 0; i < 9; i++)
      benchmark::DoNotOptimize(__ = counter);
    counter.inc();
  }
}

#define REGISTER_BENCHMARK(COUNTER_TYPE, FUNCTION, THREADS)                    \
  static void BM_##COUNTER_TYPE##FUNCTION##THREADS(benchmark::State &state) {           \
    COUNTER_TYPE##Counter counter;                                             \
    FUNCTION(state, counter);                                                  \
  }                                                                            \
  BENCHMARK(BM_##COUNTER_TYPE##FUNCTION##THREADS)->Threads(THREADS)

REGISTER_BENCHMARK(Simple, Write, 1);
REGISTER_BENCHMARK(Complex, Write, 1);
REGISTER_BENCHMARK(Simple, Read, 1);
REGISTER_BENCHMARK(Complex, Read, 1);
REGISTER_BENCHMARK(Simple, MostlyWrite, 1);
REGISTER_BENCHMARK(Complex, MostlyWrite, 1);
REGISTER_BENCHMARK(Simple, MostlyRead, 1);
REGISTER_BENCHMARK(Complex, MostlyRead, 1);

REGISTER_BENCHMARK(Simple, Write, 4);
REGISTER_BENCHMARK(Complex, Write, 4);
REGISTER_BENCHMARK(Simple, Read, 4);
REGISTER_BENCHMARK(Complex, Read, 4);
REGISTER_BENCHMARK(Simple, MostlyWrite, 4);
REGISTER_BENCHMARK(Complex, MostlyWrite, 4);
REGISTER_BENCHMARK(Simple, MostlyRead, 4);
REGISTER_BENCHMARK(Complex, MostlyRead, 4);

REGISTER_BENCHMARK(Simple, Write, 16);
REGISTER_BENCHMARK(Complex, Write, 16);
REGISTER_BENCHMARK(Simple, Read, 16);
REGISTER_BENCHMARK(Complex, Read, 16);
REGISTER_BENCHMARK(Simple, MostlyWrite, 16);
REGISTER_BENCHMARK(Complex, MostlyWrite, 16);
REGISTER_BENCHMARK(Simple, MostlyRead, 16);
REGISTER_BENCHMARK(Complex, MostlyRead, 16);

REGISTER_BENCHMARK(Simple, Write, 64);
REGISTER_BENCHMARK(Complex, Write, 64);
REGISTER_BENCHMARK(Simple, Read, 64);
REGISTER_BENCHMARK(Complex, Read, 64);
REGISTER_BENCHMARK(Simple, MostlyWrite, 64);
REGISTER_BENCHMARK(Complex, MostlyWrite, 64);
REGISTER_BENCHMARK(Simple, MostlyRead, 64);
REGISTER_BENCHMARK(Complex, MostlyRead, 64);

BENCHMARK_MAIN();
