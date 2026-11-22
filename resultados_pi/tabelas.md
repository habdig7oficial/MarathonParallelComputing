# Resultados das medicoes

Ambiente:

- Sistema: Linux 6.18.58.1-microsoft-standard-WSL2 (x86_64)
- CPU(s):                                  12
- Model name:                              12th Gen Intel(R) Core(TM) i5-12450H
- Thread(s) per core:                      2
- Core(s) per socket:                      6
- Socket(s):                               1
- ATENCAO: WSL detectado -> declarar no relatorio

## P1

Sequencial: T_seq = 0.9744 s (processo inteiro: 0.9779 s)

| P | T(P) (s) | S(P) = T(1)/T(P) | E(P) = S(P)/P | Tseq/T(P) | T total processo (s) | saida correta |
|---|---|---|---|---|---|---|
| 1 | 0.9726 | 1.00 | 1.00 | 1.00 | 0.9748 | sim |
| 2 | 0.5148 | 1.89 | 0.94 | 1.89 | 0.5233 | sim |
| 4 | 0.3415 | 2.85 | 0.71 | 2.85 | 0.3433 | sim |
| 6 | 0.3086 | 3.15 | 0.53 | 3.16 | 0.3143 | sim |

pi sequencial: 3.141592653589971  |  pi paralelo (P=6): 3.141592653589682  |  |delta| = 2.891e-13

