/* P9 - N-Rainhas por backtracking (versao PARALELA com OpenMP).
 *
 * IDEIA: a arvore de busca e dividida em SUBARVORES INDEPENDENTES.
 * Fixamos as rainhas das duas primeiras linhas: cada par valido
 * (coluna da linha 0, coluna da linha 1) e um "pedaco" de trabalho que
 * nao depende de nenhum outro. Cada thread resolve pedacos inteiros com o
 * mesmo solve() do sequencial e soma quantas solucoes achou.
 *
 * Por que DUAS linhas e nao uma (GRANULARIDADE):
 *  - So a linha 0 daria N pedacos (12 para N=12). Os pedacos tem tamanhos
 *    MUITO diferentes (a arvore e assimetrica: ramos podados cedo x ramos
 *    longos) e 12 pedacos para 6 threads deixa threads ociosas no final.
 *  - Duas linhas dao ate N*N pedacos (144; ~110 validos para N=12): muitos
 *    pedacos pequenos -> facil de equilibrar entre as threads.
 *  - Tres linhas daria pedacos pequenos demais para N=12 (overhead).
 *
 * Por que schedule(dynamic,1): como os pedacos tem custos imprevisiveis,
 * cada thread pega o proximo pedaco livre quando termina o anterior
 * (balanceamento de carga). Com static, uma thread poderia receber so
 * pedacos "pesados" e as outras ficariam esperando.
 *
 * Por que reduction(+:total): cada thread acumula sua contagem numa copia
 * privada; no fim o OpenMP soma as copias. Sem corrida e sem atomic.
 *
 * Por que os vetores cols/d1/d2 sao declarados DENTRO do laco: cada
 * pedaco precisa do seu proprio tabuleiro. Se fossem compartilhados, as
 * threads marcariam/desmarcariam posicoes umas das outras (corrida).
 *
 * Por que nao usamos tasks recursivas em todos os niveis: criar uma task
 * por no da arvore teria overhead enorme (milhoes de nos); o corte em 2
 * niveis (cutoff) cria poucas unidades de trabalho, cada uma grande. */
#include <stdio.h>
#include <omp.h>

static long solve(int row, int n, int *cols, int *d1, int *d2) {
    if (row == n) return 1;
    long count = 0;
    for (int c = 0; c < n; c++) {
        int a = row + c, b = row - c + n - 1;
        if (cols[c] || d1[a] || d2[b]) continue;
        cols[c] = d1[a] = d2[b] = 1;
        count += solve(row + 1, n, cols, d1, d2);
        cols[c] = d1[a] = d2[b] = 0;
    }
    return count;
}

int main(void) {
    int n;
    if (scanf("%d", &n) != 1) return 1;

    double t0 = omp_get_wtime();
    long total = 0;

    if (n < 2) {                      /* N=1: tabuleiro trivial */
        int cols[32] = {0}, d1[64] = {0}, d2[64] = {0};
        total = solve(0, n, cols, d1, d2);
    } else {
        #pragma omp parallel for schedule(dynamic, 1) reduction(+:total)
        for (int k = 0; k < n * n; k++) {
            int c0 = k / n;           /* coluna da rainha na linha 0 */
            int c1 = k % n;           /* coluna da rainha na linha 1 */
            /* par invalido: mesma coluna ou mesma diagonal -> pula */
            if (c0 == c1 || c0 - c1 == 1 || c1 - c0 == 1) continue;

            int cols[32] = {0}, d1[64] = {0}, d2[64] = {0};  /* privados */
            cols[c0] = d1[0 + c0] = d2[0 - c0 + n - 1] = 1;
            cols[c1] = d1[1 + c1] = d2[1 - c1 + n - 1] = 1;
            total += solve(2, n, cols, d1, d2);
        }
    }
    double t1 = omp_get_wtime();

    printf("%ld\n", total);
    fprintf(stderr, "tempo_calculo %.6f threads %d\n", t1 - t0, omp_get_max_threads());
    return 0;
}