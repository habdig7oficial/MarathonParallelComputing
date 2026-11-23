/* P1 - Calculo de pi por integracao numerica (versao PARALELA com OpenMP).
 *
 * A funcao pi(n, threads) e a do repositorio do grupo (Mateus), com as
 * correcoes da revisao: retorno step*sum (antes devolvia so sum, que nao e pi)
 * e schedule(static) explicito. O main segue o formato do enunciado: le N da
 * entrada, imprime pi com 15 casas; o numero de threads vem de OMP_NUM_THREADS.
 *
 * Por que funciona:
 *  - Cada termo 4/(1+x^2) depende so de i -> nao ha dependencia entre
 *    iteracoes, entao elas podem ser divididas entre as threads.
 *  - O unico problema e a variavel "sum": se todas as threads fizessem
 *    sum += ... ao mesmo tempo haveria CONDICAO DE CORRIDA (somas perdidas).
 *  - reduction(+:sum) resolve: cada thread soma numa copia PRIVADA de sum
 *    e, no fim do laco, o OpenMP junta as copias. Sem sincronizacao no laco.
 *  - schedule(static): todas as iteracoes custam o mesmo -> blocos iguais.
 *  - atomic/critical em cada soma deixariam o programa MAIS LENTO que o
 *    sequencial (o caderno diz que isso nao conta como solucao).
 *
 * A reducao muda a ORDEM das somas; em ponto flutuante (a+b)+c != a+(b+c) no
 * ultimo bit, entao o pi paralelo difere do sequencial a partir da ~13a casa.
 * Por isso a comparacao usa |delta| <= 1e-9. */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Calcula pi pela integral de 4/(1+x^2) em [0,1] (metodo dos retangulos, ponto medio).
 *
 * CORRECOES (revisao do grupo, seguindo o caderno do professor):
 *  1. O retorno era "sum", mas a integral e a soma das ALTURAS vezes a LARGURA
 *     de cada retangulo (step). Sem multiplicar por step o resultado era ~N*pi.
 *     Agora retorna step * sum, igual ao codigo sequencial base (secao 3.1).
 *  2. schedule(static) explicito: todas as iteracoes custam o mesmo, entao
 *     blocos iguais por thread ja equilibram a carga (sem custo de distribuicao).
 *
 * reduction(+:sum): cada thread soma numa copia privada de sum e o OpenMP junta
 * as copias no fim. Evita condicao de corrida sem usar atomic/critical. */
double pi(long long n, unsigned int threads){
    double step = 1.0 / (double)n;
    double sum = 0;

    omp_set_num_threads(threads);

    #pragma omp parallel for reduction(+:sum) schedule(static)
    for(long long i = 0; i < n; i++){
        double x = (i + 0.5) * step;
        sum += 4.0 / (1.0 + x * x);
    }

    return step * sum;
}

int main(void) {
    long long N;
    if (scanf("%lld", &N) != 1 || N <= 0) return 1;

    double t0 = omp_get_wtime();
    double r = pi(N, omp_get_max_threads());   /* threads = OMP_NUM_THREADS */
    double t1 = omp_get_wtime();

    printf("%.15f\n", r);
    fprintf(stderr, "tempo_calculo %.6f threads %d\n", t1 - t0, omp_get_max_threads());
    return 0;
}
