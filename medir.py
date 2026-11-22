#!/usr/bin/env python3
"""
Medicao automatica do Projeto 2 (P1, P5, P9), seguindo o protocolo do caderno:

  * compila tudo com  gcc -O3 -march=native -fopenmp   (Makefile de cada pasta)
  * confere a CORRETUDE do paralelo contra o sequencial
      - P1: |pi_paralelo - pi_sequencial| <= 1e-9   (tolerancia do Apendice B)
      - P5: saida identica byte a byte
      - P9: mesmo numero de solucoes
  * mede o sequencial e o paralelo com P = 1, 2, 4, 6 threads
      - OMP_NUM_THREADS fixado explicitamente em cada execucao
      - 6 execucoes por configuracao, descarta a 1a (warm-up), usa a MEDIANA das 5
  * calcula S(P) = T(1) / T(P)   e   E(P) = S(P) / P   (template do Apendice C.3;
    T(1) = versao paralela com 1 thread). T_seq do sequencial tambem e reportado.
  * salva resultados/<problema>.csv, resultados/tabelas.md e os graficos
    resultados/<problema>_speedup.png e resultados/<problema>_eficiencia.png

Uso:
  python3 medir.py              -> mede os 3 problemas (P5 demora: ~30-60 min)
  python3 medir.py P1 P9        -> so os problemas escolhidos
  python3 medir.py --rapido     -> teste rapido com entradas menores (nao usar no relatorio)
  python3 medir.py --cc gcc-14  -> escolhe o compilador (no Mac: gcc do Homebrew)

O tempo usado e o "tempo_calculo" que cada programa imprime em stderr
(medido com omp_get_wtime() so na parte de calculo). O tempo TOTAL do processo
(com leitura/escrita) tambem e salvo no CSV, para a discussao da Lei de Amdahl.
"""
import os, sys, subprocess, statistics, time, platform, csv

RAIZ = os.path.dirname(os.path.abspath(__file__))
THREADS = [1, 2, 4, 6]
REPETICOES = 6          # a primeira e descartada (warm-up) -> mediana de 5

PROBLEMAS = {
    "P1": {"pasta": "Iniciante/P1_pi",       "entrada": "1000000000\n", "rapido": "100000000\n"},
    "P5": {"pasta": "Intermediario/P5_life", "entrada": "judge.in",     "rapido": "judge50.in"},
    "P9": {"pasta": "Avancado/P9_nqueens",   "entrada": "12\n",         "rapido": "12\n"},
}


def ambiente():
    linhas = [f"Sistema: {platform.system()} {platform.release()} ({platform.machine()})"]
    try:
        if platform.system() == "Linux":
            out = subprocess.run(["lscpu"], capture_output=True, text=True).stdout
            for l in out.splitlines():
                if l.split(":")[0].strip() in ("Model name", "CPU(s)", "Core(s) per socket",
                                               "Thread(s) per core", "Socket(s)"):
                    linhas.append(l.strip())
        elif platform.system() == "Darwin":
            for k in ("machdep.cpu.brand_string", "hw.physicalcpu", "hw.perflevel0.physicalcpu",
                      "hw.perflevel1.physicalcpu"):
                v = subprocess.run(["sysctl", "-n", k], capture_output=True, text=True).stdout.strip()
                if v:
                    linhas.append(f"{k}: {v}")
        else:
            linhas.append(f"CPUs logicas: {os.cpu_count()}")
    except Exception as e:
        linhas.append(f"(nao foi possivel ler a CPU: {e})")
    if os.path.exists("/proc/version") and "microsoft" in open("/proc/version").read().lower():
        linhas.append("ATENCAO: WSL detectado -> declarar no relatorio")
    return linhas


def rodar(binario, pasta, entrada, threads=None):
    """Roda uma vez e devolve (stdout, tempo_calculo, tempo_total)."""
    env = dict(os.environ)
    if threads is not None:
        env["OMP_NUM_THREADS"] = str(threads)
    if entrada.endswith(".in"):
        stdin = open(os.path.join(pasta, entrada), "rb")
        dados = None
    else:
        stdin = subprocess.PIPE
        dados = entrada.encode()
    t0 = time.perf_counter()
    p = subprocess.run([os.path.join(pasta, binario)], input=dados, stdin=None if dados else stdin,
                       capture_output=True, env=env)
    total = time.perf_counter() - t0
    if p.returncode != 0:
        raise RuntimeError(f"{binario} falhou: {p.stderr.decode()}")
    tcalc = None
    for l in p.stderr.decode().splitlines():
        if l.startswith("tempo_calculo"):
            tcalc = float(l.split()[1])
    return p.stdout, tcalc, total


def mediana(binario, pasta, entrada, threads=None):
    calc, tot, saida = [], [], None
    for k in range(REPETICOES):
        saida, tc, tt = rodar(binario, pasta, entrada, threads)
        if k == 0:
            continue                       # warm-up descartado
        calc.append(tc); tot.append(tt)
    return statistics.median(calc), statistics.median(tot), saida


def correto(prob, s_seq, s_par):
    if prob == "P1":
        return abs(float(s_seq) - float(s_par)) <= 1e-9
    return s_seq == s_par


def graficos(prob, linhas, pasta_res):
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError:
        print("  (matplotlib nao instalado: graficos nao gerados. pip install matplotlib)")
        return
    P = [l["P"] for l in linhas]
    S = [l["S"] for l in linhas]
    E = [l["E"] for l in linhas]
    nomes = {"P1": "P1 - Calculo de pi", "P5": "P5 - Jogo da Vida", "P9": "P9 - N-Rainhas"}

    fig, ax = plt.subplots(figsize=(6, 4.2))
    ax.plot(P, P, "--", color="gray", label="Ideal (y = P)")
    ax.plot(P, S, "o-", color="#0F1A2E", linewidth=2, label="Medido")
    for x, y in zip(P, S):
        ax.annotate(f"{y:.2f}", (x, y), textcoords="offset points", xytext=(0, 8), ha="center")
    ax.set_xlabel("Numero de threads (P)"); ax.set_ylabel("Speedup S(P)")
    ax.set_title(f"{nomes[prob]}: Speedup x P"); ax.set_xticks(P); ax.grid(alpha=.3); ax.legend()
    fig.tight_layout(); fig.savefig(os.path.join(pasta_res, f"{prob}_speedup.png"), dpi=200); plt.close(fig)

    fig, ax = plt.subplots(figsize=(6, 4.2))
    ax.axhline(1, linestyle="--", color="gray", label="Ideal (y = 1)")
    ax.plot(P, E, "o-", color="#E67E22", linewidth=2, label="Medido")
    for x, y in zip(P, E):
        ax.annotate(f"{y:.2f}", (x, y), textcoords="offset points", xytext=(0, 8), ha="center")
    ax.set_xlabel("Numero de threads (P)"); ax.set_ylabel("Eficiencia E(P)")
    ax.set_ylim(0, 1.2); ax.set_title(f"{nomes[prob]}: Eficiencia x P")
    ax.set_xticks(P); ax.grid(alpha=.3); ax.legend()
    fig.tight_layout(); fig.savefig(os.path.join(pasta_res, f"{prob}_eficiencia.png"), dpi=200); plt.close(fig)


def main():
    args = sys.argv[1:]
    rapido = "--rapido" in args
    cc = None
    if "--cc" in args:
        cc = args[args.index("--cc") + 1]
    probs = [a for a in args if a in PROBLEMAS] or list(PROBLEMAS)

    pasta_res = os.path.join(RAIZ, "resultados")
    os.makedirs(pasta_res, exist_ok=True)
    amb = ambiente()
    print("\n".join(amb))
    md = ["# Resultados das medicoes", "", "Ambiente:", ""] + [f"- {l}" for l in amb] + [""]
    if rapido:
        md += ["**ATENCAO: modo --rapido (entradas reduzidas). Nao usar no relatorio.**", ""]

    for prob in probs:
        cfg = PROBLEMAS[prob]
        pasta = os.path.join(RAIZ, cfg["pasta"])
        entrada = cfg["rapido"] if rapido else cfg["entrada"]
        print(f"\n=== {prob} ({cfg['pasta']}) ===")
        subprocess.run(["make", "-s", "-C", pasta] + ([f"CC={cc}"] if cc else []), check=True)
        if prob == "P5":
            if not os.path.exists(os.path.join(pasta, "judge.in")):
                print("  gerando judge.in ...")
                subprocess.run(["make", "-s", "-C", pasta, "judge.in"], check=True)
            if rapido:
                with open(os.path.join(pasta, "judge.in")) as f, open(os.path.join(pasta, "judge50.in"), "w") as g:
                    f.readline(); g.write("5000 50\n"); g.write(f.read())

        print("  sequencial ...", flush=True)
        t_seq, tot_seq, s_seq = mediana("sequencial", pasta, entrada)
        print(f"    T_seq = {t_seq:.4f} s (total do processo {tot_seq:.4f} s)")

        linhas = []
        t1 = None
        for P in THREADS:
            print(f"  paralelo P={P} ...", flush=True)
            t, tot, s_par = mediana("paralelo", pasta, entrada, P)
            ok = correto(prob, s_seq, s_par)
            if t1 is None:
                t1 = t                      # T(1): paralelo com 1 thread (template C.3)
            S = t1 / t
            linhas.append({"P": P, "T": t, "T_total": tot, "S": S, "E": S / P, "correto": ok,
                           "S_seq": t_seq / t})
            print(f"    T({P}) = {t:.4f} s  S = {S:.2f}  E = {S/P:.2f}  (Tseq/T = {t_seq/t:.2f})  saida correta: {'SIM' if ok else 'NAO !!!'}")

        with open(os.path.join(pasta_res, f"{prob}.csv"), "w", newline="") as f:
            w = csv.writer(f)
            w.writerow(["P", "T_calculo_s", "T_total_s", "Speedup_T1", "Eficiencia", "Speedup_vs_Tseq", "saida_correta"])
            w.writerow(["seq", f"{t_seq:.6f}", f"{tot_seq:.6f}", "-", "-", "1.00", "-"])
            for l in linhas:
                w.writerow([l["P"], f"{l['T']:.6f}", f"{l['T_total']:.6f}", f"{l['S']:.2f}", f"{l['E']:.2f}", f"{l['S_seq']:.2f}",
                            "sim" if l["correto"] else "NAO"])

        md += [f"## {prob}", "", f"Sequencial: T_seq = {t_seq:.4f} s (processo inteiro: {tot_seq:.4f} s)", "",
               "| P | T(P) (s) | S(P) = T(1)/T(P) | E(P) = S(P)/P | Tseq/T(P) | T total processo (s) | saida correta |",
               "|---|---|---|---|---|---|---|"]
        for l in linhas:
            md.append(f"| {l['P']} | {l['T']:.4f} | {l['S']:.2f} | {l['E']:.2f} | {l['S_seq']:.2f} | {l['T_total']:.4f} | "
                      f"{'sim' if l['correto'] else 'NAO'} |")
        if prob == "P1":
            md += ["", f"pi sequencial: {s_seq.decode().strip()}  |  pi paralelo (P=6): {s_par.decode().strip()}  |  "
                   f"|delta| = {abs(float(s_seq) - float(s_par)):.3e}"]
        md.append("")
        graficos(prob, linhas, pasta_res)

    with open(os.path.join(pasta_res, "tabelas.md"), "w") as f:
        f.write("\n".join(md) + "\n")
    print(f"\nPronto. Veja a pasta {pasta_res}")


if __name__ == "__main__":
    main()
