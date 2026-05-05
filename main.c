#include <stdio.h>
#include <stdlib.h>

typedef unsigned char BYTE;
#define FILEHEADERSIZE 14
#define INFOHEADERSIZE 40

int isBmp (BYTE firstByte, BYTE secondByte);
int analyseFileHeader(FILE* bmpFile, int* bmpFileSize, int* bmpOffSetPosition);
int analyseInfoHeader(FILE* bmpFile, int* bmpImageHeight, int* bmpImageWidth, int* bitsByPixel, int* compression, int* bmpImageSize);
short constructShort(BYTE* startByte);
int constructInt(BYTE* startByte);

int main(int argc, char *argv[])
{
    FILE *bmpFile = fopen("imagem.bmp", "rb");
    int bmpFileSize;
    int bmpOffSetPosition;
    int imageBmpHeight, imageBmpWidth;
    short bitsByPixel;
    int compression;
    int bmpSizeImage;

    if(analyseFileHeader(bmpFile, &bmpFileSize, &bmpOffSetPosition) != 0)
        return 1;

    if(analyseInfoHeader(bmpFile, &imageBmpHeight, &imageBmpWidth, &bitsByPixel, &compression, &bmpSizeImage) != 0)
        return 1;

    fclose(bmpFile);
    return 0;
}

int analyseFileHeader(FILE* bmpFile, int* bmpFileSize, int* bmpOffSetPosition)
{
    BYTE fileHeader[FILEHEADERSIZE];

    if(bmpFile == NULL)
        return 1;

    if(fread(fileHeader, FILEHEADERSIZE, 1, bmpFile) != 1)
    {
        printf("The program could not read header\n");
        return 1;
    }

    if(!isBmp(fileHeader[0], fileHeader[1]))
    {
        printf("You can't open others file formats, just .bmp\n");
        return 1;
    }

    *bmpFileSize = constructInt(&fileHeader[2]);
    *bmpOffSetPosition = constructInt(&fileHeader[10]);

    return 0;
}

int analyseInfoHeader(FILE* bmpFile, int* bmpImageHeight, int* bmpImageWidth, int* bitsByPixel, int* compression, int* bmpImageSize)
{
    BYTE infoHeader[INFOHEADERSIZE];
    
    if(fread(infoHeader, INFOHEADERSIZE, 1, bmpFile) != 1)
    {
        printf("The program could not read info header\n");
        return 1;
    }

    //Fread passa a ler a partir do primeiro BYTE ainda não lido
    //Após ler até o header[13] fread começa do [14]
    //ImageWidth está em header[18] -> 18 - 14 = 4
    //ImageHeight está em header[22] -> 22 - 14 = 8
    *bmpImageWidth = constructInt(&infoHeader[4]);
    *bmpImageHeight = constructInt(&infoHeader[8]);
    
    *bitsByPixel = constructShort(&infoHeader[14]);
    *compression = constructInt(&infoHeader[16]);
    *bmpImageSize = constructInt(&infoHeader[20]);
    
    return 0;
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

short constructShort(BYTE* startByte)
{
    return startByte[0] | ((unsigned int)startByte[1] << 8);
}

int isBmp (BYTE firstByte, BYTE secondByte)
{
    return firstByte == 'B' && secondByte == 'M';
}