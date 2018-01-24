# counter_benchmark
Benchmark for thread shared counters

```bash
git clone --recursive git@github.com:kevgs/counter_benchmark.git
cd counter_benchmark
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON -DBENCHMARK_ENABLE_GTEST_TESTS=OFF ..
cmake --build .
./counter-benchmark
```

Sample results:
```bash
$ cat /proc/cpuinfo | grep 'model name'
model name	: Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz
model name	: Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz
model name	: Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz
model name	: Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz
```

```
2020-06-12 15:47:32
Running ./counter-benchmark
Run on (4 X 3500 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 6144 KiB (x1)
Load Average: 1.36, 1.58, 1.98
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
-----------------------------------------------------------------------
Benchmark                             Time             CPU   Iterations
-----------------------------------------------------------------------
BM_SimpleAtomic/threads:1          5.46 ns         5.46 ns    125184034
BM_SimpleAtomic/threads:2          22.7 ns         45.3 ns     16149760
BM_SimpleAtomic/threads:4          21.9 ns         86.3 ns      8240708
BM_SimpleAtomic/threads:8          19.6 ns         90.0 ns      9558280
BM_SimpleAtomic/threads:16         18.4 ns         90.0 ns      8421008
BM_SimpleAtomic/threads:32         18.1 ns         90.4 ns      8019040
BM_SimpleAtomic/threads:64         13.6 ns         90.3 ns      6400000
BM_SimpleAtomic/threads:128        10.4 ns         91.4 ns      8697856
BM_SimpleAtomic/threads:256        4.61 ns         89.8 ns      8169472
BM_ShardedAtomic/threads:1         5.46 ns         5.45 ns    124874178
BM_ShardedAtomic/threads:2         2.88 ns         5.75 ns    120535942
BM_ShardedAtomic/threads:4         4.96 ns         19.8 ns     36480860
BM_ShardedAtomic/threads:8         1.31 ns         5.82 ns    120172920
BM_ShardedAtomic/threads:16        1.32 ns         6.05 ns    120051856
BM_ShardedAtomic/threads:32        1.17 ns         6.09 ns    120159904
BM_ShardedAtomic/threads:64       0.814 ns         5.91 ns    111187648
BM_ShardedAtomic/threads:128      0.569 ns         6.15 ns     97840128
BM_ShardedAtomic/threads:256      0.304 ns         6.43 ns    107348736
BM_TlsCounter/threads:1            1.61 ns         1.61 ns    430596335
BM_TlsCounter/threads:2           0.848 ns         1.70 ns    412619732
BM_TlsCounter/threads:4           0.435 ns         1.72 ns    405170372
BM_TlsCounter/threads:8           0.373 ns         1.72 ns    405357912
BM_TlsCounter/threads:16          0.362 ns         1.72 ns    405965584
BM_TlsCounter/threads:32          0.322 ns         1.72 ns    403633760
BM_TlsCounter/threads:64          0.312 ns         1.72 ns    404524480
BM_TlsCounter/threads:128         0.231 ns         1.72 ns    405150848
BM_TlsCounter/threads:256         0.061 ns         1.72 ns    405948160
```
