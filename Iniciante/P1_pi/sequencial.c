/* P1 - Calculo de pi por integracao numerica (versao SEQUENCIAL).
 * Codigo base do professor, sem alteracoes no calculo.
 * Unica adicao: medicao do tempo com omp_get_wtime(), impressa em stderr
 * (stdout continua so com o resultado, para a validacao funcionar). */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(void) {
    long long N;
    if (scanf("%lld", &N) != 1 || N <= 0) return 1;

    double t0 = omp_get_wtime();
    double step = 1.0 / (double)N, sum = 0.0;
    for (long long i = 0; i < N; i++) {
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }
    double t1 = omp_get_wtime();

    printf("%.15f\n", step * sum);
    fprintf(stderr, "tempo_calculo %.6f\n", t1 - t0);
    return 0;
}
