// Confere a CORRETUDE da funcao pi() (P1_pi/pi.c) contra o codigo sequencial do
// professor (secao 3.1), com a tolerancia do caderno: |delta| <= 1e-9.
// Linux/WSL: gcc -O3 -march=native -fopenmp validar_pi.c P1_pi/pi.c -o validar_pi -lm && ./validar_pi
#include <stdio.h>
#include <math.h>
#include <omp.h>
double pi(long long n, unsigned int threads);
int main(void){
    const long long N = 1000000000LL;
    double step = 1.0 / (double)N, sum = 0.0;
    for (long long i = 0; i < N; i++){ double x = (i + 0.5) * step; sum += 4.0 / (1.0 + x * x); }
    double seq = step * sum;
    printf("pi sequencial (professor): %.15f\n", seq);
    int ps[] = {1, 2, 4, 6}, ok = 1;
    for (int k = 0; k < 4; k++){
        double par = pi(N, ps[k]), d = fabs(par - seq);
        printf("pi paralelo  P=%d:         %.15f  |delta| = %.2e  %s\n", ps[k], par, d, d <= 1e-9 ? "OK" : "ERRADO");
        if (d > 1e-9) ok = 0;
    }
    return ok ? 0 : 1;
}
