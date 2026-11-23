#!/usr/bin/env python3
"""Gera judge.in para o P5 (Game of Life). Copiado do Apendice B.5 do caderno.
Uso: python3 gen_life.py > judge.in
"""
import random, sys

def main():
    N, G = 5000, 500
    random.seed(42)
    dens = 0.15
    out = sys.stdout
    out.write(f"{N} {G}\n")
    for _ in range(N):
        linha = ''.join('x' if random.random() < dens else ' ' for _ in range(N))
        out.write(linha + '\n')

if __name__ == "__main__":
    main()
