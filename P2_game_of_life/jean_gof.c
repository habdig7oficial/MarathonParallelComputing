#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

typedef unsigned char cell_t;

cell_t **alloc_board(int n) {
    int i;
    cell_t **b = (cell_t **)malloc(sizeof(cell_t *) * n);
    for (i = 0; i < n; i++) {
        b[i] = (cell_t *)malloc(sizeof(cell_t) * n);
    }
    return b;
}

void free_board(cell_t **b, int n) {
    int i;
    for (i = 0; i < n; i++) {
        free(b[i]);
    }
    free(b);
}

int adjacent_to(cell_t **b, int n, int i, int j) {
    int sk = (i > 0) ? i - 1 : i;
    int ek = (i + 1 < n) ? i + 1 : i;
    int sl = (j > 0) ? j - 1 : j;
    int el = (j + 1 < n) ? j + 1 : j;
    int count = 0;
    int k, l;

    for (k = sk; k <= ek; k++) {
        for (l = sl; l <= el; l++) {
            count += b[k][l];
        }
    }
    return count - b[i][j];
}

void play(cell_t **cur, cell_t **nxt, int n, unsigned int threads) {
    int i, j;
      
    omp_set_num_threads(threads);

    #pragma omp parallel for collapse(2) private(i, j)
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            int a = adjacent_to(cur, n, i, j);
            if (a == 2) {
                nxt[i][j] = cur[i][j];
            } else if (a == 3) {
                nxt[i][j] = 1;
            } else {
                nxt[i][j] = 0;
            }
        }
    }
}