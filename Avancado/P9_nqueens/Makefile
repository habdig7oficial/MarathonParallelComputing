# Compilacao conforme o Apendice B.2 do caderno.
# No Mac (sem OpenMP no clang padrao), use o gcc do Homebrew: make CC=gcc-14
CC     = gcc
CFLAGS  = -O3 -march=native -fopenmp

all: sequencial paralelo

sequencial: sequencial.c
	$(CC) $(CFLAGS) sequencial.c -o sequencial

paralelo: paralelo.c
	$(CC) $(CFLAGS) paralelo.c -o paralelo

clean:
	rm -f sequencial paralelo 
