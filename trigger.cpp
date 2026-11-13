#include "P1_pi/pi.h"
#include "benchmark/registration.h"
#include "benchmark/state.h"

using namespace std;
using namespace benchmark;


static void BM_PIC(State& state){
    for(auto _ : state)
        pi(1000);
}


BENCHMARK(BM_PIC);
BENCHMARK_MAIN();