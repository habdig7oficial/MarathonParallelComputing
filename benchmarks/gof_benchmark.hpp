
#pragma once
#include "benchmark/registration.h"
#include "thread"
#include "stdlib.h"
#include "random"

using namespace std;
using namespace benchmark;

extern unsigned int seed;

static void BM_GOF(State& state){ 

    mt19937 rand(seed);
    uniform_int_distribution<> dist(0, 255);

    char *gen = (char *)malloc(SizeBuf);
    char *gen_next = (char *)malloc(SizeBuf);

    for(int i = 0; i < SizeBuf; i++)
        gen[i] = dist(rand);

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

//BENCHMARK(BM_GOF) -> DenseRange(1, thread::hardware_concurrency()) -> UseRealTime();
BENCHMARK(BM_GOF) -> DenseRange(1, 2) -> UseRealTime();