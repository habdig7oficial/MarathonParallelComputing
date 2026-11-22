#pragma once
#include "benchmark/registration.h"
#include "benchmark/benchmark.h"
#include "thread"

using namespace benchmark;
using namespace std;

#define SHOTS 1000000

static void BM_PIC(State& state){ 
    for(auto _ : state) {
        double pi_val = pi(SHOTS); 

        DoNotOptimize(pi_val);
    }
}
BENCHMARK(BM_PIC) -> ThreadRange(1, thread::hardware_concurrency());
