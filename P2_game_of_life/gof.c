#include "limits.h"
#include "stdlib.h"
#include "stdio.h"
#include "constants.h"
#include "omp.h"

#include "stdbool.h"
#include <stdint.h>


void print(const uint8_t *gen, unsigned int dim_y, unsigned int dim_x, uint8_t per_bit){
    printf("%d %d \n", dim_y, dim_y);
    for(int i = 0; i < dim_y; i++){
        for(int j = 0; j < dim_x; j++){
            int byte_index = i * dim_x + j;
            int index = byte_index / per_bit;
            int offset = byte_index % per_bit;

            uint8_t curr = gen[index];

            bool is_alive = (curr & (1 << offset)) > 0;

            if(is_alive)
                printf("1 ");
            else 
                printf("0 ");
        }
        printf("\n");
    }
    printf("%s", SEP);
}


struct Sum{
    uint8_t res;
    uint8_t carry;
};

struct Sum full_adder(uint8_t a, uint8_t b, uint8_t c_in){
    uint8_t ab = a ^ b;
    struct Sum s = {
        .res = ab ^ c_in,
        .carry =  (a & b) | (ab & c_in)
    };


    return s;
}

void next(unsigned int dim_y, unsigned int dim_x, uint8_t *gen_next, const uint8_t *gen, uint8_t per_bit, unsigned int threads){
   
    omp_set_num_threads(threads);
    int offset = (dim_x / per_bit);

    // ==========================================
    // SPECIAL CASE: offset == 1 (Board width is 1 byte)
    // ==========================================
    if (offset == 1) {
        // Main board (Rows 1 to dim_y - 2)
        #pragma omp parallel for schedule(guided)
        for(int row = 1; row < dim_y - 1; row++){
            int i = row;
            struct Sum s1 = full_adder(gen[i - 1] << 1, gen[i - 1], gen[i - 1] >> 1);
            struct Sum s2 = full_adder(gen[i] << 1, gen[i] >> 1, gen[i + 1] << 1);
            struct Sum s3 = full_adder(gen[i + 1], gen[i + 1] >> 1, 0);
            
            struct Sum stage2 = full_adder(s1.res, s2.res, s3.res);
            struct Sum stage3 = full_adder(s1.carry, s2.carry, s3.carry);
            struct Sum stage4 = full_adder(stage3.res, stage2.carry, 0);

            uint8_t c0 = stage2.res;
            uint8_t c1 = stage4.res;
            uint8_t c2 = stage4.carry | stage3.carry;
            int is2or3 = ~c2 & c1;
            gen_next[i] = (gen[i] & is2or3) | (~gen[i] & (is2or3 & c0));
        }

        // Border Top (row = 0)
        #pragma omp parallel for schedule(static)
        for(int row = 0; row < 1; row++) {
            int i = 0;
            struct Sum s2 = full_adder(gen[i] << 1, gen[i] >> 1, gen[i + 1] << 1);
            struct Sum s3 = full_adder(gen[i + 1], gen[i + 1] >> 1, 0);
            struct Sum stage2 = full_adder(0, s2.res, s3.res);
            struct Sum stage3 = full_adder(0, s2.carry, s3.carry);
            struct Sum stage4 = full_adder(stage3.res, stage2.carry, 0);
            uint8_t c0 = stage2.res, c1 = stage4.res, c2 = stage4.carry | stage3.carry;
            int is2or3 = ~c2 & c1;
            gen_next[i] = (gen[i] & is2or3) | (~gen[i] & (is2or3 & c0));
        }

        // Border Bottom (row = dim_y - 1)
        #pragma omp parallel for schedule(static)
        for(int row = dim_y - 1; row < dim_y; row++) {
            int base = (dim_y - 1); // Fixed: base defined correctly for offset == 1
            int i = base;
            struct Sum s1 = full_adder(gen[i - 1] << 1, gen[i - 1], gen[i - 1] >> 1);
            struct Sum s2 = full_adder(gen[i] << 1, gen[i] >> 1, 0);
            struct Sum stage2 = full_adder(s1.res, s2.res, 0);
            struct Sum stage3 = full_adder(s1.carry, s2.carry, 0);
            struct Sum stage4 = full_adder(stage3.res, stage2.carry, 0);
            uint8_t c0 = stage2.res, c1 = stage4.res, c2 = stage4.carry | stage3.carry;
            int is2or3 = ~c2 & c1;
            gen_next[i] = (gen[i] & is2or3) | (~gen[i] & (is2or3 & c0));
        }
        return;
    }

    // ==========================================
    // GENERAL CASE: offset >= 2 (Fully branch-free inner columns)
    // ==========================================

    // Main board (Rows 1 to dim_y - 2)
    #pragma omp parallel for schedule(guided)
    for(int row = 1; row < dim_y - 1; row++){
        int base = row * offset;

        // --- Left Boundary (col = 0) ---
        {
            int i = base;
            struct Sum s1 = full_adder(
                gen[i - offset] << 1, 
                gen[i - offset], 
                (gen[i - offset] >> 1) | (gen[i - offset + 1] << 7)
            );
            struct Sum s2 = full_adder(
                gen[i] << 1, 
                (gen[i] >> 1) | (gen[i + 1] << 7), 
                gen[i + offset] << 1
            );
            struct Sum s3 = full_adder(
                gen[i + offset], 
                (gen[i + offset] >> 1) | (gen[i + offset + 1] << 7), 
                0
            );
            struct Sum stage2 = full_adder(s1.res, s2.res, s3.res);
            struct Sum stage3 = full_adder(s1.carry, s2.carry, s3.carry);
            struct Sum stage4 = full_adder(stage3.res, stage2.carry, 0);
            uint8_t c0 = stage2.res, c1 = stage4.res, c2 = stage4.carry | stage3.carry;
            int is2or3 = ~c2 & c1;
            gen_next[i] = (gen[i] & is2or3) | (~gen[i] & (is2or3 & c0));
        }

        // --- Middle Columns (col = 1 to offset - 2) ---
        for(int col = 1; col < offset - 1; col++){
            int i = base + col;
            struct Sum s1 = full_adder(
                (gen[i - offset] << 1) | (gen[i - offset - 1] >> 7), 
                gen[i - offset], 
                (gen[i - offset] >> 1) | (gen[i - offset + 1] << 7)
            );
            struct Sum s2 = full_adder(
                (gen[i] << 1) | (gen[i - 1] >> 7), 
                (gen[i] >> 1) | (gen[i + 1] << 7), 
                (gen[i + offset] << 1) | (gen[i + offset - 1] >> 7)
            );
            struct Sum s3 = full_adder(
                gen[i + offset], 
                (gen[i + offset] >> 1) | (gen[i + offset + 1] << 7), 
                0
            );
            struct Sum stage2 = full_adder(s1.res, s2.res, s3.res);
            struct Sum stage3 = full_adder(s1.carry, s2.carry, s3.carry);
            struct Sum stage4 = full_adder(stage3.res, stage2.carry, 0);
            uint8_t c0 = stage2.res, c1 = stage4.res, c2 = stage4.carry | stage3.carry;
            int is2or3 = ~c2 & c1;
            gen_next[i] = (gen[i] & is2or3) | (~gen[i] & (is2or3 & c0));
        }

        // --- Right Boundary (col = offset - 1) ---
        {
            int i = base + offset - 1;
            struct Sum s1 = full_adder(
                (gen[i - offset] << 1) | (gen[i - offset - 1] >> 7), 
                gen[i - offset], 
                gen[i - offset] >> 1
            );
            struct Sum s2 = full_adder(
                (gen[i] << 1) | (gen[i - 1] >> 7), 
                gen[i] >> 1, 
                (gen[i + offset] << 1) | (gen[i + offset - 1] >> 7)
            );
            struct Sum s3 = full_adder(
                gen[i + offset], 
                gen[i + offset] >> 1, 
                0
            );
            struct Sum stage2 = full_adder(s1.res, s2.res, s3.res);
            struct Sum stage3 = full_adder(s1.carry, s2.carry, s3.carry);
            struct Sum stage4 = full_adder(stage3.res, stage2.carry, 0);
            uint8_t c0 = stage2.res, c1 = stage4.res, c2 = stage4.carry | stage3.carry;
            int is2or3 = ~c2 & c1;
            gen_next[i] = (gen[i] & is2or3) | (~gen[i] & (is2or3 & c0));
        }
    }

    // Border Top (row = 0)
    #pragma omp parallel for schedule(static)
    for(int row = 0; row < 1; row++) {
        // Left boundary col = 0
        {
            int i = 0;
            struct Sum s2 = full_adder(
                gen[i] << 1, 
                (gen[i] >> 1) | (gen[i + 1] << 7), 
                gen[i + offset] << 1
            );
            struct Sum s3 = full_adder(
                gen[i + offset], 
                (gen[i + offset] >> 1) | (gen[i + offset + 1] << 7), 
                0
            );
            struct Sum stage2 = full_adder(0, s2.res, s3.res);
            struct Sum stage3 = full_adder(0, s2.carry, s3.carry);
            struct Sum stage4 = full_adder(stage3.res, stage2.carry, 0);
            uint8_t c0 = stage2.res, c1 = stage4.res, c2 = stage4.carry | stage3.carry;
            int is2or3 = ~c2 & c1;
            gen_next[i] = (gen[i] & is2or3) | (~gen[i] & (is2or3 & c0));
        }

        // Middle columns
        for(int col = 1; col < offset - 1; col++) {
            int i = col;
            struct Sum s2 = full_adder(
                (gen[i] << 1) | (gen[i - 1] >> 7), 
                (gen[i] >> 1) | (gen[i + 1] << 7), 
                (gen[i + offset] << 1) | (gen[i + offset - 1] >> 7)
            );
            struct Sum s3 = full_adder(
                gen[i + offset], 
                (gen[i + offset] >> 1) | (gen[i + offset + 1] << 7), 
                0
            );
            struct Sum stage2 = full_adder(0, s2.res, s3.res);
            struct Sum stage3 = full_adder(0, s2.carry, s3.carry);
            struct Sum stage4 = full_adder(stage3.res, stage2.carry, 0);
            uint8_t c0 = stage2.res, c1 = stage4.res, c2 = stage4.carry | stage3.carry;
            int is2or3 = ~c2 & c1;
            gen_next[i] = (gen[i] & is2or3) | (~gen[i] & (is2or3 & c0));
        }

        // Right boundary col = offset - 1
        {
            int i = offset - 1;
            struct Sum s2 = full_adder(
                (gen[i] << 1) | (gen[i - 1] >> 7), 
                gen[i] >> 1, 
                (gen[i + offset] << 1) | (gen[i + offset - 1] >> 7)
            );
            struct Sum s3 = full_adder(
                gen[i + offset], 
                gen[i + offset] >> 1, 
                0
            );
            struct Sum stage2 = full_adder(0, s2.res, s3.res);
            struct Sum stage3 = full_adder(0, s2.carry, s3.carry);
            struct Sum stage4 = full_adder(stage3.res, stage2.carry, 0);
            uint8_t c0 = stage2.res, c1 = stage4.res, c2 = stage4.carry | stage3.carry;
            int is2or3 = ~c2 & c1;
            gen_next[i] = (gen[i] & is2or3) | (~gen[i] & (is2or3 & c0));
        }
    }

    // Border Bottom (row = dim_y - 1)
    #pragma omp parallel for schedule(static)
    for(int row = dim_y - 1; row < dim_y; row++) {
        int base = (dim_y - 1) * offset;

        // Left boundary col = 0
        {
            int i = base;
            struct Sum s1 = full_adder(
                gen[i - offset] << 1, 
                gen[i - offset], 
                (gen[i - offset] >> 1) | (gen[i - offset + 1] << 7)
            );
            struct Sum s2 = full_adder(
                gen[i] << 1, 
                (gen[i] >> 1) | (gen[i + 1] << 7), 
                0
            );
            struct Sum stage2 = full_adder(s1.res, s2.res, 0);
            struct Sum stage3 = full_adder(s1.carry, s2.carry, 0);
            struct Sum stage4 = full_adder(stage3.res, stage2.carry, 0);
            uint8_t c0 = stage2.res, c1 = stage4.res, c2 = stage4.carry | stage3.carry;
            int is2or3 = ~c2 & c1;
            gen_next[i] = (gen[i] & is2or3) | (~gen[i] & (is2or3 & c0));
        }

        // Middle columns
        for(int col = 1; col < offset - 1; col++) {
            int i = base + col;
            struct Sum s1 = full_adder(
                (gen[i - offset] << 1) | (gen[i - offset - 1] >> 7), 
                gen[i - offset], 
                (gen[i - offset] >> 1) | (gen[i - offset + 1] << 7)
            );
            struct Sum s2 = full_adder(
                (gen[i] << 1) | (gen[i - 1] >> 7), 
                (gen[i] >> 1) | (gen[i + 1] << 7), 
                0
            );
            struct Sum stage2 = full_adder(s1.res, s2.res, 0);
            struct Sum stage3 = full_adder(s1.carry, s2.carry, 0);
            struct Sum stage4 = full_adder(stage3.res, stage2.carry, 0);
            uint8_t c0 = stage2.res, c1 = stage4.res, c2 = stage4.carry | stage3.carry;
            int is2or3 = ~c2 & c1;
            gen_next[i] = (gen[i] & is2or3) | (~gen[i] & (is2or3 & c0));
        }

        // Right boundary col = offset - 1
        {
            int i = base + offset - 1;
            struct Sum s1 = full_adder(
                (gen[i - offset] << 1) | (gen[i - offset - 1] >> 7), 
                gen[i - offset], 
                gen[i - offset] >> 1
            );
            struct Sum s2 = full_adder(
                (gen[i] << 1) | (gen[i - 1] >> 7), 
                gen[i] >> 1, 
                0
            );
            struct Sum stage2 = full_adder(s1.res, s2.res, 0);
            struct Sum stage3 = full_adder(s1.carry, s2.carry, 0);
            struct Sum stage4 = full_adder(stage3.res, stage2.carry, 0);
            uint8_t c0 = stage2.res, c1 = stage4.res, c2 = stage4.carry | stage3.carry;
            int is2or3 = ~c2 & c1;
            gen_next[i] = (gen[i] & is2or3) | (~gen[i] & (is2or3 & c0));
        }
    }
}

void process(uint8_t *gen_next, const uint8_t *gen, unsigned int threads){
    //print(gen, SizeY, SizeX, CHAR_BIT);
    next(SizeY, SizeX, gen_next, gen, CHAR_BIT, threads);
}


void print_gen(uint8_t *gen, uint8_t *gen_next, unsigned int threads){
    for(int i = 0; i < G; i++){
        //printf("%s %dth Generation Begins %s", SEP, i, SEP);

        //printf("%p %p", gen, gen_next);
        process(gen_next, gen, threads);

        uint8_t *aux;
        aux = gen;
        gen = gen_next;
        gen_next = aux;
    }

}
