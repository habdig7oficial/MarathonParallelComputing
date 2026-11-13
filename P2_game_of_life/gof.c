#include "limits.h"
#include "stdlib.h"
#include "stdio.h"

#define G 10
#define DimX 2
#define DimY 1

#define SEP "\n=================\n"


const int SizeX = DimX * CHAR_BIT;
const int SizeY = DimY * CHAR_BIT;

const int SizeBuf = DimY * DimX * CHAR_BIT; // size of allocated buffer
const int DimBuf = SizeY * SizeX;

void print(const char *gen, unsigned int dim_y, unsigned int dim_x, unsigned int per_bit){
    printf("%d %d \n", dim_y, dim_y);
    for(int i = 0; i < dim_y; i++){
        for(int j = 0; j < dim_x; j++){
            int byte_index = i * dim_x + j;
            int index = byte_index / per_bit;
            int offset = byte_index % per_bit;

            char curr = gen[index];

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
    unsigned int res;
    unsigned int carry;
};

struct Sum full_adder(unsigned int a, unsigned int b, unsigned int c_in){
    struct Sum s1 = {
        .res = a ^ b,
        .carry = a & b
    };

    struct Sum s2 = {
        .res = s1.res ^ c_in,
        .carry = s1.carry & c_in
    };

    s2.carry |= s1.carry;

    return s2;
}

void next(unsigned int dim_y, unsigned int dim_x, char *gen_next, const char *gen, unsigned int per_bit){
    for(int i = dim_x / per_bit; i < ((dim_y - 1) * dim_x) / per_bit; i++){
        int offset = (dim_x / per_bit); // find nex / prev columns

        printf("(%d %d %d)", upper_plane, middle_plane, down_plane);
    }
}


void process(char *gen_next, const char *gen){
    print(gen, SizeY, SizeX, CHAR_BIT);
    next(SizeY, SizeX, gen_next, gen, CHAR_BIT);
}


int main() {
    printf("%d", SizeX);
    printf("%s Board Size:\n", SEP);
    printf("size x: %d (%d u8)\nsize y: % (%d u8)\n",  SizeX, DimX, SizeY, DimY);
    printf("Total Size: %d (%d u8)%s", DimBuf, SizeBuf, SEP);

    char *gen = (char *)malloc(SizeBuf);
    char *gen_next = (char *)malloc(SizeBuf);

    gen[1] = 8;
    gen[3] = 8;
    gen[5] = 9;

    gen[6] = 254;

    for(int i = 0; i < G; i++){
        printf("%s %dth Generation Begins %s", SEP, i, SEP);

        printf("%p %p", gen, gen_next);
        process(gen_next, gen);

        char *aux;
        aux = gen;
        gen = gen_next;
        gen_next = aux;
    }

    //printf("%d", SizeBuf);


}