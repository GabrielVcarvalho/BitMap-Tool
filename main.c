#include <stdio.h>
#include <stdlib.h>

typedef unsigned char BYTE;
#define FILEHEADERSIZE 14

int isBmp (BYTE firstByte, BYTE secondByte);
int analyseFileHeader(FILE* bmpFile, int* bmpFileSize, int* bmpOffSetPosition);
int constructInt(BYTE* startByte);

int main(int argc, char *argv[])
{
    FILE *bmpFile = fopen("imagem.bmp", "rb");
    int bmpFileSize;
    int bmpOffSetPosition;

    if(analyseFileHeader(bmpFile, &bmpFileSize, &bmpOffSetPosition) == 1)
        return 1;

    fclose(bmpFile);
    return 0;
}

int analyseFileHeader(FILE* bmpFile, int* bmpFileSize, int* bmpOffSetPosition)
{
    BYTE header[FILEHEADERSIZE];

    if(bmpFile == NULL)
        return 1;

    if(fread(header, FILEHEADERSIZE, 1, bmpFile) != 1)
    {
        printf("The program could not read header\n");
        return 1;
    }

    if(!isBmp(header[0], header[1]))
    {
        printf("You can't open others file formats, just .bmp");
        return 1;
    }

    *bmpFileSize = constructInt(&header[2]);
    *bmpOffSetPosition = constructInt(&header[10]);
}

int constructInt(BYTE* startByte)
{
    int byteSize = 8;
    int bytesInOneInt = 4;
    int finalInt = 0;

    for(int i = 0; i < bytesInOneInt; i++)
    {
        finalInt |= (*(startByte + i) << (byteSize * i));
    }
    
    return finalInt;
}

int isBmp (BYTE firstByte, BYTE secondByte)
{
    return firstByte == 'B' && secondByte == 'M';
}