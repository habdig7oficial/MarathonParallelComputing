
#pragma once
#include "benchmark/registration.h"
#include "thread"
#include "stdlib.h"
#include "random"
#include "../P2_game_of_life/jean_gof.h"

using namespace std;
using namespace benchmark;

extern unsigned int seed;

static void fill_random(char *buf, size_t num_bytes, unsigned int seed) {
    mt19937 rand(seed);
    uniform_int_distribution<> dist(0, 255);
    for (size_t i = 0; i < num_bytes; i++) 
        buf[i] = static_cast<char>(dist(rand));
    
}

static void BM_GOF(State& state){ 
    char *gen = (char *)malloc(SizeBuf);
    char *gen_next = (char *)malloc(SizeBuf);

    fill_random(gen, SizeBuf, seed);

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
BENCHMARK(BM_GOF) -> DenseRange(1, 1) -> UseRealTime();


static void BM_GOF_Unpacked(benchmark::State& state) {
    int threads = static_cast<int>(state.range(0));
    int n = threads * 32;

    /* Generate the exact same packed bytes first, then unpack into cur[i][j] */
    char *packed_init = (char *)std::malloc(SizeBuf);
    fill_random(packed_init, SizeBuf, seed);

    cell_t **cur = alloc_board(n);
    cell_t **nxt = alloc_board(n);

    size_t bit_idx = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            unsigned char byte = static_cast<unsigned char>(packed_init[bit_idx / 8]);
            /* LSB-first extraction; use (7 - (bit_idx % 8)) if your packer is MSB-first */
            cur[i][j] = (byte >> (bit_idx % 8)) & 1;
            nxt[i][j] = 0;
            bit_idx++;
        }
    }
    std::free(packed_init);

    for (auto _ : state) {
        play(cur, nxt, n);
        std::swap(cur, nxt);

        benchmark::DoNotOptimize(cur);
        benchmark::DoNotOptimize(nxt);
        benchmark::ClobberMemory();
    }

    free_board(cur, n);
    free_board(nxt, n);
}

BENCHMARK(BM_GOF_Unpacked) -> DenseRange(1, thread::hardware_concurrency()) -> UseRealTime();