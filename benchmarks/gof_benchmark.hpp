
#pragma once
#include "benchmark/registration.h"

static void BM_GOF1(State& state){ 
    char *gen = (char *)malloc(SizeBuf);
    char *gen_next = (char *)malloc(SizeBuf);

    for(auto _ : state) 
        print_gen(gen, gen_next); 
}

BENCHMARK(BM_GOF1);