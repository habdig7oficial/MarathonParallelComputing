#!/usr/bin/env python3
import json, re, statistics, sys

ARQ = sys.argv[1] if len(sys.argv) > 1 else "benchmark.json"
NOMES = {"BM_GOF_Unpacked": "Professor (1 celula por byte)", "BM_GOF": "Bits (8 celulas por byte)"}
FATOR = {"ns": 1e-9, "us": 1e-6, "ms": 1e-3, "s": 1.0}

dados = json.load(open(ARQ))
medianas, repeticoes = {}, {}
for b in dados["benchmarks"]:
    m = re.match(r"(BM_GOF_Unpacked|BM_GOF)/(\d+)", b["name"])
    if not m:
        continue
    versao, p = m.group(1), int(m.group(2))
    t = b["real_time"] * FATOR[b.get("time_unit", "ns")]
    if b.get("run_type") == "aggregate":
        if b.get("aggregate_name") == "median":
            medianas[(versao, p)] = t
    else:
        repeticoes.setdefault((versao, p), []).append(t)

for chave, ts in repeticoes.items():          # sem aggregates: descarta a 1a e tira a mediana
    if chave not in medianas:
        medianas[chave] = statistics.median(ts[1:] if len(ts) > 1 else ts)

ps = sorted({p for (v, p) in medianas if (("BM_GOF", p) in medianas and ("BM_GOF_Unpacked", p) in medianas)})
if not ps:
    sys.exit("Nao achei BM_GOF e BM_GOF_Unpacked com o mesmo numero de threads no " + ARQ)

linhas = ["| Threads | Professor (ms) | Bits (ms) | Reducao de tempo | Vezes mais rapido |",
          "|---|---|---|---|---|"]
for p in ps:
    tj, tm = medianas[("BM_GOF_Unpacked", p)], medianas[("BM_GOF", p)]
    linhas.append(f"| {p} | {tj*1e3:.2f} | {tm*1e3:.3f} | {100*(1-tm/tj):.1f}% | {tj/tm:.1f}x |")
ctx = dados.get("context", {})
cab = [f"Arquivo: {ARQ}"]
if "seed" in ctx:
    cab.append(f"Seed: {ctx['seed']}")
txt = "\n".join(cab + ["", *linhas, "",
      "Obs.: so vale comparar se as duas versoes usam a mesma grade e o mesmo numero de geracoes,",
      "e se a saida da versao em bits for igual a do professor."])
print(txt)
open("comparacao_gof.md", "w").write(txt + "\n")

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    fig, ax = plt.subplots(figsize=(7, 4.4))
    larg = 0.38
    xs = range(len(ps))
    tj = [medianas[("BM_GOF_Unpacked", p)] * 1e3 for p in ps]
    tm = [medianas[("BM_GOF", p)] * 1e3 for p in ps]
    b1 = ax.bar([x - larg / 2 for x in xs], tj, larg, label=NOMES["BM_GOF_Unpacked"], color="#9AA5B1")
    b2 = ax.bar([x + larg / 2 for x in xs], tm, larg, label=NOMES["BM_GOF"], color="#0F1A2E")
    for x, a, c in zip(xs, tj, tm):
        ax.annotate(f"{a/c:.0f}x mais rápido", (x + larg / 2, c), textcoords="offset points",
                    xytext=(0, 4), ha="center", fontsize=9)
    ax.set_yscale("log")
    ax.set_xticks(list(xs)); ax.set_xticklabels([f"{p} thread" + ("s" if p > 1 else "") for p in ps])
    ax.set_ylabel("Tempo mediano (ms, escala log)")
    ax.set_title("Jogo da Vida: versão do professor x versão em bits")
    ax.grid(axis="y", alpha=.3, which="both"); ax.legend()
    fig.tight_layout(); fig.savefig("comparacao_gof.png", dpi=200)
    print("\nGrafico salvo em comparacao_gof.png")
except ImportError:
    print("\n(matplotlib nao instalado: grafico nao gerado. Instale com: pip install matplotlib)")
