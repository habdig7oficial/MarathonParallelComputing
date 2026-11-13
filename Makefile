C = gcc-16
CXX = g++

compile: 
	$(C) P1_pi/pi.c -o pi_elf -c -fPIC

benchmark: compile
	$(CXX) trigger.cpp pi_elf -std=c++17 -march=native -O3 -o benchmark -lbenchmark

run_final: benchmark
	./benchmark --benchmark_min_warmup_time=2 --benchmark_repetitions=40 --benchmark_report_aggregates_only=true --benchmark_out=benchmark.json --benchmark_out_format=json
