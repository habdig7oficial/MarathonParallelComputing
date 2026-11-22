// Modified baseline done by Gemini https://share.gemini.google/NbPW0c2ZsR3V

/* gol_baseline.h */
#ifndef GOL_BASELINE_H
#define GOL_BASELINE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char cell_t;

cell_t **alloc_board(int n);
void free_board(cell_t **b, int n);
void play(cell_t **cur, cell_t **nxt, int n);

#ifdef __cplusplus
}
#endif

#endif /* GOL_BASELINE_H */