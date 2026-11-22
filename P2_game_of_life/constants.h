#define G 10
#define DimX 50
#define DimY 50

#define SEP "\n=================\n"


const int SizeX = DimX * CHAR_BIT;
const int SizeY = DimY * CHAR_BIT;

const int SizeBuf = DimY * DimX * CHAR_BIT; // size of allocated buffer
const int DimBuf = SizeY * SizeX;