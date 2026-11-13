#include "P1_pi/pi.h"
#include "benchmark/registration.h"
#include "benchmark/state.h"

using namespace std;
using namespace benchmark;


static void BM_PIC(State& state){
    //cout << "Threads" << opm_get_num_threads() << endl;
    for(auto _ : state)
        pi(1000);
}


BENCHMARK(BM_PIC);
BENCHMARK_MAIN();