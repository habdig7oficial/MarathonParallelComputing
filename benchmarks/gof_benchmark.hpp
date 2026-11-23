
#pragma once
#include "benchmark/registration.h"
#include "thread"
#include "stdlib.h"
#include "random"
#include "../P2_game_of_life/jean_gof.h"

using namespace std;
using namespace benchmark;

extern unsigned int seed;

static void fill_random(uint8_t *buf, size_t num_bytes, unsigned int seed) {
    mt19937 rand(seed);
    uniform_int_distribution<> dist(0, 255);
    for (size_t i = 0; i < num_bytes; i++) 
        buf[i] = static_cast<uint8_t>(dist(rand));
    
}

static void BM_GOF(State& state){ 
    uint8_t *gen = (uint8_t *)malloc(SizeBuf);
    uint8_t *gen_next = (uint8_t *)malloc(SizeBuf);

    fill_random(gen, SizeBuf, seed);

    for(auto _ : state) {
        int threads = static_cast<int>(state.range(0));
        print_gen(gen, gen_next, threads);

        DoNotOptimize(gen);
        DoNotOptimize(gen_next);
        ClobberMemory();
    } 

    free(gen);
    free(gen_next);
}

//BENCHMARK(BM_GOF) -> DenseRange(1, thread::hardware_concurrency()) -> UseRealTime() -> Complexity();
BENCHMARK(BM_GOF) -> DenseRange(1, 1) -> UseRealTime() -> Complexity();

static void BM_GOF_Unpacked(State& state) {
    int threads = static_cast<int>(state.range(0));
    
    // Use the exact same fixed dimensions as Version 1
    int n = SizeY; // Assuming SizeY == SizeX (square grid)
    
    // SizeBuf should match what Version 1 uses
    uint8_t *packed_init = (uint8_t *)malloc(SizeBuf);
    fill_random(packed_init, SizeBuf, seed);

    cell_t **cur = alloc_board(n);
    cell_t **nxt = alloc_board(n);

    size_t bit_idx = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            uint8_t byte = static_cast<uint8_t>(packed_init[bit_idx / 8]);
            cur[i][j] = (byte >> (bit_idx % 8)) & 1;
            nxt[i][j] = 0;
            bit_idx++;
        }
    }
    free(packed_init);
    threads = static_cast<int>(state.range(0));

    for (auto _ : state) {
        // Runs for G generations
        for(int i = 0; i < G; i++){
            play(cur, nxt, n, threads);
            swap(cur, nxt);
        }

        DoNotOptimize(cur);
        DoNotOptimize(nxt);
        ClobberMemory();
    }

    free_board(cur, n);
    free_board(nxt, n);
}

//BENCHMARK(BM_GOF_Unpacked) -> DenseRange(1, thread::hardware_concurrency()) -> UseRealTime() -> Complexity();
BENCHMARK(BM_GOF_Unpacked) -> DenseRange(1, 1) -> UseRealTime() -> Complexity();