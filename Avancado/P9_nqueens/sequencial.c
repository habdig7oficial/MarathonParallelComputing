/* P9 - N-Rainhas por backtracking (versao SEQUENCIAL).
 * Codigo base do professor, sem alteracoes no algoritmo.
 * Adicao: tempo medido com omp_get_wtime(), impresso em stderr. */
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
    int cols[32] = {0}, d1[64] = {0}, d2[64] = {0};
    double t0 = omp_get_wtime();
    long total = solve(0, n, cols, d1, d2);
    double t1 = omp_get_wtime();
    printf("%ld\n", total);
    fprintf(stderr, "tempo_calculo %.6f\n", t1 - t0);
    return 0;
}
