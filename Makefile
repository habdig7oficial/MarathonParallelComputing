C = gcc-16
CXX = g++


LIB_FLAGS= /opt/homebrew/opt/libomp/include/
LIB_FLAGS_LINK= /opt/homebrew/opt/libomp/lib/

compile: 
	$(C) P1_pi/pi.c -o pi_elf -c -fPIC
	$(C) P2_game_of_life/gof.c -o life

benchmark: compile
	$(CXX) -Xclang -fopenmp -I$(LIB_FLAGS) -L$(LIB_FLAGS_LINK) -lomp trigger.cpp pi_elf -std=c++17 -march=native -O3 -o benchmark -lbenchmark

run_final: benchmark
	./benchmark --benchmark_min_warmup_time=2 --benchmark_repetitions=40 --benchmark_report_aggregates_only=true --benchmark_out=benchmark.json --benchmark_out_format=json

run_test: benchmark
	./benchmark --benchmark_repetitions=2 --benchmark_report_aggregates_only=true --benchmark_out=benchmark.json --benchmark_out_format=json
