#include "P1_pi/pi.h"
#include "P2_game_of_life/gof.h"
#include "P2_game_of_life/constants.h"

#include "benchmark/registration.h"
#include "benchmark/state.h"
#include "omp.h"
#include "random"


#include "benchmarks/pi_benchmark.hpp"
#include "benchmarks/gof_benchmark.hpp"

using namespace std;
using namespace benchmark;


unsigned int seed;

int main(int argc, char *argv[]){
    Initialize(&argc, argv);

    random_device rd;
    seed = rd();

    AddCustomContext("seed", to_string(seed));

    RunSpecifiedBenchmarks();
    Shutdown();
}