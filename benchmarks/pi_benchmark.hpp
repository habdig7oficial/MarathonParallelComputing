#pragma once
#include "benchmark/registration.h"
#include "thread"

using namespace benchmark;
using namespace std;

#define SHOTS 100000

static void BM_PIC(State& state){ for(auto _ : state) pi(SHOTS); }
BENCHMARK(BM_PIC) -> ThreadRange(1, thread::hardware_concurrency());
