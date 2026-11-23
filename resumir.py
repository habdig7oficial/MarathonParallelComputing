#!/usr/bin/env python3
"""Le benchmark.json do Google Benchmark e aplica o protocolo do caderno:
descarta a 1a repeticao (warm-up), mediana das 5 seguintes, S = T(1)/T(P), E = S/P.
Gera resultados_benchmark/tabela.md e os graficos de speedup e eficiencia."""
import json, statistics, os, re
d = json.load(open("benchmark.json"))
runs = {}
for b in d["benchmarks"]:
    if b.get("run_type") != "iteration": continue
    m = re.match(r"BM_PIC/(\d+)", b["name"])
    if not m: continue
    p = int(m.group(1))
    fator = {"ns": 1e-9, "us": 1e-6, "ms": 1e-3, "s": 1.0}[b.get("time_unit", "ns")]
    runs.setdefault(p, []).append(b["real_time"] * fator)       # -> segundos
os.makedirs("resultados_benchmark", exist_ok=True)
P = sorted(runs); T = {p: statistics.median(runs[p][1:]) for p in P}   # descarta a 1a
L = ["| P | T(P) (s) | S(P) = T(1)/T(P) | E(P) = S(P)/P |", "|---|---|---|---|"]
for p in P:
    S = T[P[0]] / T[p]; L.append(f"| {p} | {T[p]:.4f} | {S:.2f} | {S/p:.2f} |")
ctx = d.get("context", {})
txt = "\n".join([f"Maquina: {ctx.get('num_cpus')} CPUs, {ctx.get('mhz_per_cpu')} MHz", "", *L, ""])
open("resultados_benchmark/tabela.md", "w").write(txt); print(txt)
try:
    import matplotlib; matplotlib.use("Agg"); import matplotlib.pyplot as plt
    S = [T[P[0]] / T[p] for p in P]; E = [s / p for s, p in zip(S, P)]
    for nome, y, ideal, cor, yl in (("speedup", S, P, "#0F1A2E", "Speedup S(P)"), ("eficiencia", E, [1]*len(P), "#E67E22", "Eficiencia E(P)")):
        f, a = plt.subplots(figsize=(6, 4.2))
        a.plot(P, ideal, "--", color="gray", label="Ideal"); a.plot(P, y, "o-", color=cor, lw=2, label="Medido")
        for x, v in zip(P, y): a.annotate(f"{v:.2f}", (x, v), textcoords="offset points", xytext=(0, 8), ha="center")
        a.set_xlabel("Numero de threads (P)"); a.set_ylabel(yl); a.set_xticks(P); a.grid(alpha=.3); a.legend()
        if nome == "eficiencia": a.set_ylim(0, 1.2)
        a.set_title(f"P1 - Calculo de pi: {yl.split()[0]} x P"); f.tight_layout()
        f.savefig(f"resultados_benchmark/P1_{nome}.png", dpi=200); plt.close(f)
except ImportError:
    print("(matplotlib nao instalado: graficos nao gerados)")
