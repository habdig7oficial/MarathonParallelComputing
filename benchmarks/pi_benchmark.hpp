#pragma once
#include "benchmark/registration.h"
#include "benchmark/benchmark.h"
#include "thread"

using namespace benchmark;
using namespace std;

#define SHOTS 1000000000

static void BM_PIC(State& state){ 
    for(auto _ : state) {
        double pi_val = pi(SHOTS, static_cast<int>(state.range(0))); 

        DoNotOptimize(pi_val);
    }
}
// P = 1, 2, 4 e 6 threads, como pede o caderno (Regras do jogo)
BENCHMARK(BM_PIC) -> Arg(1) -> Arg(2) -> Arg(4) -> Arg(6) -> UseRealTime() -> Unit(kMillisecond);
