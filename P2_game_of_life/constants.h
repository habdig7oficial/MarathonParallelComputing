#define G 10
#define DimX 100
#define DimY 100

#define SEP "\n=================\n"


const int SizeX = DimX * CHAR_BIT;
const int SizeY = DimY * CHAR_BIT;

const int SizeBuf = DimY * DimX * CHAR_BIT; // size of allocated buffer
const int DimBuf = SizeY * SizeX;