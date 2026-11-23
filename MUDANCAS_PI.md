# Branch `sofia-pi` · Problema 1 (π)

Feita a partir da `main` no commit `aaad25c` ("first test", 22/11 16:43).
**A `main` não foi alterada.** Esta branch só mexe no π. O Jogo da Vida ficou igual.

O que já estava certo na `main` e **não foi mexido**: `pi.c` (com `return sum * step` e
`schedule(static)`), `SHOTS 1000000000`, `-O3 -march=native` em tudo, `DoNotOptimize` e `UseRealTime()`.

## 1. Mudanças em arquivos que já existiam (2 arquivos)

### `Makefile` (alvo `run_final`)

```diff
-	./benchmark --benchmark_min_warmup_time=2 --benchmark_repetitions=40 --benchmark_report_aggregates_only=true --benchmark_out=benchmark.json --benchmark_out_format=json
+	./benchmark --benchmark_min_warmup_time=2 --benchmark_repetitions=6 --benchmark_out=benchmark.json --benchmark_out_format=json
+	python3 resumir.py
```

- O caderno (Regras do jogo) pede **mediana de 5 execuções, descartando a 1ª (warm-up)**.
- Com 40 repetições e `report_aggregates_only`, o JSON só trazia média e mediana **das 40**,
  sem os tempos individuais, e não dava para descartar a 1ª.
- Agora são 6 repetições, o JSON guarda as 6 e o `resumir.py` descarta a 1ª e tira a mediana das 5.
- O warm-up de 2 s continua.
- O `run_test` não foi mexido.

### `benchmarks/pi_benchmark.hpp`

```diff
-BENCHMARK(BM_PIC) -> DenseRange(1, thread::hardware_concurrency()) -> UseRealTime();
+BENCHMARK(BM_PIC) -> Arg(1) -> Arg(2) -> Arg(4) -> Arg(6) -> UseRealTime() -> Unit(kMillisecond);
```

- O caderno pede P = 1, 2, 4 e 6. O `DenseRange` rodava de 1 até o número de núcleos.
- `Unit(kMillisecond)` só muda a unidade mostrada (o padrão era nanossegundos).

## 2. Arquivos novos (não mudam o código existente)

| Arquivo | Para que serve |
|---|---|
| `resumir.py` | Lê o `benchmark.json`, descarta a 1ª repetição, tira a mediana das 5, calcula S(P) = T(1)/T(P) e E(P) = S(P)/P e gera `resultados_benchmark/tabela.md` e os 2 gráficos com as retas ideais. |
| `validar_pi.c` | Confere a corretude: compara `pi()` com o sequencial do professor para P = 1, 2, 4, 6 (\|Δ\| ≤ 10⁻⁹). |
| `Iniciante/P1_pi/` | Pasta no formato de entrega do caderno: `sequencial.c` (código base do professor + `omp_get_wtime`), `paralelo.c` (a mesma `pi()` da `main` + `main` que lê N da entrada e imprime com 15 casas), `Makefile` e `entrada.in` (N = 10⁹). |
| `medir.py` | Mede o sequencial e o paralelo da pasta de entrega: 6 execuções por P, descarta a 1ª, mediana de 5, confere a saída. Rodar com `python3 medir.py P1`. |
| `resultados_pi/` | Resultados medidos no notebook da Sofia (i5-12450H, WSL2) com o `medir.py`: tabela e os 2 gráficos que estão no relatório. |

## Resultados atuais (`resultados_pi/`)

| P | T(P) (s) | S(P) | E(P) |
|---|---|---|---|
| 1 | 0,9726 | 1,00 | 1,00 |
| 2 | 0,5148 | 1,89 | 0,94 |
| 4 | 0,3415 | 2,85 | 0,71 |
| 6 | 0,3086 | 3,15 | 0,53 |

π sequencial 3,141592653589971 · π paralelo (P = 6) 3,141592653589682 · |Δ| = 2,9 × 10⁻¹³.

Se o grupo decidir usar o `make run_final` como medição oficial, a tabela e os gráficos do
relatório precisam ser refeitos com os números dele (`resultados_benchmark/`).

## Como rodar

```bash
make run_final                                                            # benchmark + resumo
gcc -O3 -march=native -fopenmp validar_pi.c P1_pi/pi.c -o validar_pi -lm && ./validar_pi
cd Iniciante/P1_pi && make && echo 1000000000 | ./paralelo               # versão de entrega
python3 medir.py P1                                                       # medição da pasta de entrega
```
