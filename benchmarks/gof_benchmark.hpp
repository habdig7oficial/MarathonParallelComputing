
#pragma once
#include "benchmark/registration.h"
#include "thread"
#include "stdlib.h"

using namespace std;
using namespace benchmark;

static void BM_GOF(State& state){ 
    char *gen = (char *)malloc(SizeBuf);
    char *gen_next = (char *)malloc(SizeBuf);

    for(auto _ : state) {
        int threads = static_cast<int>(state.range(0));
        print_gen(gen, gen_next, threads, threads * 32);

        DoNotOptimize(gen);
        DoNotOptimize(gen_next);
        ClobberMemory();
    } 

    free(gen);
    free(gen_next);
}

BENCHMARK(BM_GOF) -> DenseRange(1, thread::hardware_concurrency()) -> UseRealTime();