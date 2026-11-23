/* P5 - Jogo da Vida de Conway (versao PARALELA com OpenMP).
 *
 * ONDE ESTA O PARALELISMO: dentro de UMA geracao, cada celula de nxt
 * depende so de celulas de cur (geracao anterior), que ninguem escreve
 * durante a geracao. Entao todas as celulas de uma geracao podem ser
 * calculadas ao mesmo tempo -> dividimos as colunas da grade entre as
 * threads (laco "i" de play()).
 *
 * O QUE NAO PODE SER PARALELIZADO: as geracoes entre si. A geracao t+1
 * precisa da geracao t COMPLETA (dependencia RAW entre geracoes). Por isso
 * existe uma BARREIRA ao fim de cada geracao: nenhuma thread comeca a
 * geracao seguinte antes de todas terminarem a atual. O "omp for" ja tem
 * essa barreira implicita no final.
 *
 * ESCOLHAS:
 *  - Uma unica regiao "omp parallel" FORA do laco de geracoes: as threads
 *    sao criadas uma vez so (e nao 500 vezes, uma por geracao), evitando o
 *    overhead de fork/join repetido. Dentro dela, "omp for" divide o
 *    trabalho de cada geracao.
 *  - firstprivate(cur, nxt): cada thread tem sua copia dos PONTEIROS e faz
 *    a troca cur<->nxt sozinha depois da barreira. Como todas trocam do
 *    mesmo jeito, todas continuam apontando para as mesmas grades. Assim
 *    nao precisamos de "omp single" para a troca.
 *  - schedule(static): toda celula custa o mesmo -> blocos iguais de
 *    colunas por thread. Blocos contiguos tambem sao bons para a cache e
 *    cada thread escreve em colunas diferentes de nxt (sem false sharing,
 *    exceto nas fronteiras entre blocos, que sao poucas).
 *  - Leitura da entrada e impressao continuam sequenciais (I/O em ordem);
 *    essa e a parte sequencial "f" da Lei de Amdahl neste problema.
 *  - O calculo de cada celula e IDENTICO ao sequencial (so inteiros), entao
 *    a saida tem que ser igual byte a byte. */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
typedef unsigned char cell_t;

static cell_t **alloc_board(int n) {
    cell_t **b = malloc(sizeof(cell_t *) * n);
    for (int i = 0; i < n; i++) b[i] = malloc(sizeof(cell_t) * n);
    return b;
}
static void free_board(cell_t **b, int n) {
    for (int i = 0; i < n; i++) free(b[i]);
    free(b);
}
static int adjacent_to(cell_t **b, int n, int i, int j) {
    int sk = (i > 0) ? i - 1 : i, ek = (i + 1 < n) ? i + 1 : i;
    int sl = (j > 0) ? j - 1 : j, el = (j + 1 < n) ? j + 1 : j;
    int count = 0;
    for (int k = sk; k <= ek; k++)
        for (int l = sl; l <= el; l++) count += b[k][l];
    return count - b[i][j];
}
int main(void) {
    int n, g;
    if (scanf("%d %d", &n, &g) != 2) return 1;
    cell_t **cur = alloc_board(n), **nxt = alloc_board(n);
    char *line = malloc(n + 16);
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {}
    for (int j = 0; j < n; j++) {
        if (!fgets(line, n + 16, stdin)) return 1;
        size_t len = strcspn(line, "\r\n");          /* mesma correcao do sequencial */
        for (int i = 0; i < n; i++)
            cur[i][j] = ((size_t)i < len && line[i] == 'x');
    }

    double t0 = omp_get_wtime();
    #pragma omp parallel firstprivate(cur, nxt)
    {
        for (int s = 0; s < g; s++) {
            #pragma omp for schedule(static)
            for (int i = 0; i < n; i++)
                for (int j = 0; j < n; j++) {
                    int a = adjacent_to(cur, n, i, j);
                    if (a == 2) nxt[i][j] = cur[i][j];
                    else if (a == 3) nxt[i][j] = 1;
                    else nxt[i][j] = 0;
                }
            /* barreira implicita do omp for: todas terminaram a geracao */
            cell_t **t = nxt; nxt = cur; cur = t;   /* cada thread troca a sua copia */
        }
    }
    double t1 = omp_get_wtime();

    /* Apos g trocas, o resultado esta em cur se g e par, ou em nxt se e impar
     * (a troca feita dentro da regiao foi nas copias privadas). */
    if (g % 2 == 1) { cell_t **t = nxt; nxt = cur; cur = t; }

    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++) putchar(cur[i][j] ? 'x' : ' ');
        putchar('\n');
    }
    fprintf(stderr, "tempo_calculo %.6f threads %d\n", t1 - t0, omp_get_max_threads());
    free_board(cur, n); free_board(nxt, n); free(line);
    return 0;
}
