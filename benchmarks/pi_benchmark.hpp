#pragma once
#include "benchmark/registration.h"

using namespace benchmark;

static void BM_PIC1(State& state){ for(auto _ : state) pi(1000, 1); }
static void BM_PIC2(State& state){ for(auto _ : state) pi(1000, 2); }
static void BM_PIC4(State& state){ for(auto _ : state) pi(1000, 4); }
static void BM_PIC8(State& state){ for(auto _ : state) pi(1000, 8); }
static void BM_PIC16(State& state){ for(auto _ : state) pi(1000, 16); }

/**
BENCHMARK(BM_PIC1);
BENCHMARK(BM_PIC2);
BENCHMARK(BM_PIC4);
BENCHMARK(BM_PIC8);
BENCHMARK(BM_PIC16); */