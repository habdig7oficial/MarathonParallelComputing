// Gerado com Claude
// Teste do Jogo da Vida: compara a versao em bits (next() de gof.c) com a versao do Jean.
//
// Compilar (na raiz do repositorio):
//   gcc -O2 -fopenmp -IP2_game_of_life test_gof.c P2_game_of_life/gof.c -o test_gof
// Rodar:
//   ./test_gof
//
// Testes:
//  1. Padroes conhecidos (block, blinker, glider) na versao do Jean, para conferir a referencia.
//  2. Os mesmos padroes na versao em bits, no meio de um byte e cruzando a borda entre bytes.
//  3. Grades aleatorias de varios tamanhos, varias geracoes e 1, 2, 4 e 6 threads, celula por celula.
//     As diferencas sao separadas em: borda de byte, primeira/ultima linha e meio.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void next(unsigned int dim_y, unsigned int dim_x, uint8_t *gen_next, const uint8_t *gen,
          uint8_t per_bit, unsigned int threads, unsigned int tile);

/* ---------- versao do Jean (mesma regra do jean_gof.c) ---------- */
static int adjacent_to(const unsigned char *b, int n, int i, int j) {
    int sk = (i > 0) ? i - 1 : i, ek = (i + 1 < n) ? i + 1 : i;
    int sl = (j > 0) ? j - 1 : j, el = (j + 1 < n) ? j + 1 : j;
    int count = 0;
    for (int k = sk; k <= ek; k++)
        for (int l = sl; l <= el; l++) count += b[k * n + l];
    return count - b[i * n + j];
}
static void jean_play(const unsigned char *cur, unsigned char *nxt, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            int a = adjacent_to(cur, n, i, j);
            if (a == 2) nxt[i * n + j] = cur[i * n + j];
            else if (a == 3) nxt[i * n + j] = 1;
            else nxt[i * n + j] = 0;
        }
}
static void jean_run(unsigned char *g, int n, int gens) {
    unsigned char *t = malloc(n * n), *a = g, *b = t;
    for (int s = 0; s < gens; s++) { jean_play(a, b, n); unsigned char *x = a; a = b; b = x; }
    if (a != g) memcpy(g, a, n * n);
    free(t);
}

/* ---------- versao em bits ---------- */
static int get_bit(const uint8_t *g, int n, int i, int j) { int k = i * n + j; return (g[k / 8] >> (k % 8)) & 1; }
static void set_bit(uint8_t *g, int n, int i, int j, int v) {
    int k = i * n + j;
    if (v) g[k / 8] |= (uint8_t)(1 << (k % 8)); else g[k / 8] &= (uint8_t)~(1 << (k % 8));
}
static void bits_run(unsigned char *g, int n, int gens, int threads) {
    int bytes = n * n / 8;
    uint8_t *a = calloc(bytes, 1), *b = calloc(bytes, 1);
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) set_bit(a, n, i, j, g[i * n + j]);
    for (int s = 0; s < gens; s++) {           // troca os buffers igual ao print_gen
        next(n, n, b, a, 8, threads, threads * 32);
        uint8_t *x = a; a = b; b = x;
    }
    for (int i = 0; i < n; i++) for (int j = 0; j < n; j++) g[i * n + j] = get_bit(a, n, i, j);
    free(a); free(b);
}

/* ---------- utilitarios ---------- */
static int falhas = 0;
static void resultado(const char *nome, int ok) {
    printf("  %-58s %s\n", nome, ok ? "OK" : "FALHOU");
    if (!ok) falhas++;
}
static void colocar(unsigned char *g, int n, int i0, int j0, const char *pad[], int linhas) {
    for (int i = 0; i < linhas; i++)
        for (int j = 0; pad[i][j]; j++) g[(i0 + i) * n + j0 + j] = (pad[i][j] == 'x');
}

static const char *BLOCK[]   = {"xx", "xx"};
static const char *BLINKER[] = {"xxx"};
static const char *BLINK_V[] = {"x", "x", "x"};
static const char *GLIDER[]  = {".x.", "..x", "xxx"};

/* testa um padrao: roda 'gens' geracoes e compara com o padrao esperado */
static int testa_padrao(int versao_bits, int n, int i0, int j0, const char *ini[], int li,
                        int gens, const char *esp[], int le, int ei, int ej) {
    unsigned char *g = calloc(n * n, 1), *e = calloc(n * n, 1);
    colocar(g, n, i0, j0, ini, li);
    colocar(e, n, ei, ej, esp, le);
    if (versao_bits) bits_run(g, n, gens, 1); else jean_run(g, n, gens);
    int ok = memcmp(g, e, n * n) == 0;
    free(g); free(e);
    return ok;
}

static void padroes(int versao_bits) {
    const char *v = versao_bits ? "bits" : "Jean";
    char nome[96];
    int n = 32;
    // no meio do byte (colunas 10 a 13) e cruzando a borda entre bytes (colunas 7 e 8)
    int cols[2] = {10, 7};
    const char *onde[2] = {"meio do byte", "borda entre bytes"};
    for (int c = 0; c < 2; c++) {
        int j = cols[c];
        snprintf(nome, sizeof nome, "[%s] block parado (%s)", v, onde[c]);
        resultado(nome, testa_padrao(versao_bits, n, 10, j, BLOCK, 2, 5, BLOCK, 2, 10, j));
        snprintf(nome, sizeof nome, "[%s] blinker vira vertical (%s)", v, onde[c]);
        resultado(nome, testa_padrao(versao_bits, n, 10, j - 1, BLINKER, 1, 1, BLINK_V, 3, 9, j));
        snprintf(nome, sizeof nome, "[%s] glider anda 1 casa em 4 geracoes (%s)", v, onde[c]);
        resultado(nome, testa_padrao(versao_bits, n, 10, j - 1, GLIDER, 3, 4, GLIDER, 3, 11, j));
    }
    snprintf(nome, sizeof nome, "[%s] blinker encostado na 1a linha", v);
    resultado(nome, testa_padrao(versao_bits, n, 0, 10, BLINK_V, 3, 2, BLINK_V, 3, 0, 10));
}

static void aleatorios(void) {
    int tamanhos[] = {16, 64, 200}, geracoes[] = {1, 5, 20}, threads[] = {1, 2, 4, 6};
    srand(12345);
    for (int t = 0; t < 3; t++)
        for (int gi = 0; gi < 3; gi++) {
            int n = tamanhos[t], gens = geracoes[gi];
            unsigned char *ini = malloc(n * n), *ref = malloc(n * n), *bit = malloc(n * n);
            for (int k = 0; k < n * n; k++) ini[k] = (rand() % 100) < 30;
            memcpy(ref, ini, n * n);
            jean_run(ref, n, gens);
            for (int ti = 0; ti < 4; ti++) {
                memcpy(bit, ini, n * n);
                bits_run(bit, n, gens, threads[ti]);
                int borda_byte = 0, linha_extrema = 0, meio = 0, pi = -1, pj = -1;
                for (int i = 0; i < n; i++)
                    for (int j = 0; j < n; j++)
                        if (ref[i * n + j] != bit[i * n + j]) {
                            if (pi < 0) { pi = i; pj = j; }
                            if (i == 0 || i == n - 1) linha_extrema++;
                            else if (j % 8 == 0 || j % 8 == 7) borda_byte++;
                            else meio++;
                        }
                int total = borda_byte + linha_extrema + meio;
                char nome[128];
                snprintf(nome, sizeof nome, "N=%-3d G=%-2d threads=%d", n, gens, threads[ti]);
                printf("  %-26s ", nome);
                if (total == 0) printf("OK\n");
                else {
                    printf("FALHOU: %d celulas diferentes (borda de byte %d, 1a/ultima linha %d, meio %d), "
                           "1a em (%d, %d)\n", total, borda_byte, linha_extrema, meio, pi, pj);
                    falhas++;
                }
            }
            free(ini); free(ref); free(bit);
        }
}

int main(void) {
    printf("1) Padroes conhecidos na versao do Jean (confere a referencia)\n");
    padroes(0);
    printf("\n2) Padroes conhecidos na versao em bits\n");
    padroes(1);
    printf("\n3) Grades aleatorias: bits x Jean, celula por celula\n");
    aleatorios();
    printf("\n%s (%d teste(s) falharam)\n", falhas ? "RESULTADO: FALHOU" : "RESULTADO: TUDO OK", falhas);
    return falhas ? 1 : 0;
}