
#pragma once
#include "benchmark/registration.h"
#include "thread"
#include "stdlib.h"

using namespace std;
using namespace benchmark;

static void BM_GOF(State& state){ 
    char *gen = (char *)malloc(SizeBuf);
    char *gen_next = (char *)malloc(SizeBuf);

    for(auto _ : state) 
        print_gen(gen, gen_next); 

    free(gen);
    free(gen_next);
}

//BENCHMARK(BM_GOF) -> ThreadRange(1, thread::hardware_concurrency());

