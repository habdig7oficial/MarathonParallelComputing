#define G 500
#define DimX 5000
#define DimY 5000

#define SEP "\n=================\n"


const int SizeX = DimX * CHAR_BIT;
const int SizeY = DimY * CHAR_BIT;

const int SizeBuf = DimY * DimX * CHAR_BIT; // size of allocated buffer
const int DimBuf = SizeY * SizeX;