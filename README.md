# counter_benchmark
Benchmark for two counters

git clone --recursive git@github.com:kevgs/counter_benchmark.git
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -DBENCHMARK_DOWNLOAD_DEPENDENCIES=ON -DBENCHMARK_ENABLE_GTEST_TESTS=OFF counter_benchmark/
make -j4
./counter-benchmark

Sample results:
```bash
$ cat /proc/cpuinfo | grep 'model name'
model name	: Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz
model name	: Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz
model name	: Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz
model name	: Intel(R) Core(TM) i7-6700HQ CPU @ 2.60GHz
```

```
Run on (4 X 3500 MHz CPU s)
CPU Caches:
  L1 Data 32K (x4)
  L1 Instruction 32K (x4)
  L2 Unified 256K (x4)
  L3 Unified 6144K (x1)
***WARNING*** CPU scaling is enabled, the benchmark real time measurements may be noisy and will incur extra overhead.
--------------------------------------------------------------------------
Benchmark                                   Time           CPU Iterations
--------------------------------------------------------------------------
BM_SimpleWrite1/threads:1                   5 ns          5 ns  124821910
BM_ComplexWrite1/threads:1                  7 ns          7 ns   92465698
BM_SimpleRead1/threads:1                    0 ns          0 ns 1000000000
BM_ComplexRead1/threads:1                  23 ns         23 ns   30860907
BM_SimpleMostlyWrite1/threads:1            53 ns         53 ns   12367504
BM_ComplexMostlyWrite1/threads:1          102 ns        102 ns    7214472
BM_SimpleMostlyRead1/threads:1             12 ns         12 ns   56692014
BM_ComplexMostlyRead1/threads:1           331 ns        331 ns    2056522
BM_SimpleWrite4/threads:4                   1 ns          6 ns  119939072
BM_ComplexWrite4/threads:4                  2 ns          8 ns   86003692
BM_SimpleRead4/threads:4                    0 ns          0 ns 2150001604
BM_ComplexRead4/threads:4                   6 ns         24 ns   28488192
BM_SimpleMostlyWrite4/threads:4            14 ns         56 ns   12363364
BM_ComplexMostlyWrite4/threads:4           25 ns        102 ns    6871392
BM_SimpleMostlyRead4/threads:4              3 ns         12 ns   58406904
BM_ComplexMostlyRead4/threads:4            91 ns        363 ns    1936464
BM_SimpleWrite16/threads:16                 1 ns          6 ns  118809776
BM_ComplexWrite16/threads:16                1 ns          8 ns   85848752
BM_SimpleRead16/threads:16                  0 ns          0 ns 2143611648
BM_ComplexRead16/threads:16                 4 ns         24 ns   28794096
BM_SimpleMostlyWrite16/threads:16           9 ns         56 ns   12259008
BM_ComplexMostlyWrite16/threads:16         17 ns        102 ns    6827280
BM_SimpleMostlyRead16/threads:16            2 ns         12 ns   58239616
BM_ComplexMostlyRead16/threads:16          71 ns        360 ns    1925440
BM_SimpleWrite64/threads:64                 1 ns          6 ns  120233664
BM_ComplexWrite64/threads:64                1 ns          8 ns   85383744
BM_SimpleRead64/threads:64                  0 ns          0 ns 2153523520
BM_ComplexRead64/threads:64                 4 ns         24 ns   28607168
BM_SimpleMostlyWrite64/threads:64           9 ns         56 ns   12382272
BM_ComplexMostlyWrite64/threads:64         18 ns        102 ns    6787968
BM_SimpleMostlyRead64/threads:64            2 ns         12 ns   57988928
BM_ComplexMostlyRead64/threads:64          63 ns        360 ns    1940032

```
