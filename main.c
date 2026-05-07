#include <stdio.h>
#include <stdlib.h>

typedef unsigned char BYTE;
typedef struct
{
    BYTE blue;
    BYTE green;
    BYTE red;
} PIXEL;

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
    short bitsPerPixel;
    int compression;
    int bmpSizeImage;

    if(analyseFileHeader(bmpFile, &bmpFileSize, &bmpOffSetPosition) != 0)
        return 1;

    if(analyseInfoHeader(bmpFile, &imageBmpHeight, &imageBmpWidth, &bitsPerPixel, &compression, &bmpSizeImage) != 0)
        return 1;

    fclose(bmpFile);
    return 0;
}

int transformGrayScale(FILE* bmpFile ,int bmpOffSetPosition, int bitsPerPixel, int height, int width)
{
    BYTE allHeader[bmpOffSetPosition];
    fseek(bmpFile, 0, SEEK_SET);
    fread(allHeader, bmpOffSetPosition, 1, bmpFile);
    FILE* bmpGrayScale = fopen("imagemGray.bmp", "wb");
    int bytesPerRow;
    int padding;

    bytesPerRow = width * (bitsPerPixel / 8); 
    padding = (4 - (bytesPerRow % 4)) % 4;

    if(bmpGrayScale == NULL)
        return 1;

    fwrite(allHeader, bmpOffSetPosition, 1, bmpGrayScale);
    
    for (int line = 0; line < height; line++)
    {
        PIXEL imageLineArray[width];

        fread(imageLineArray, bytesPerRow, 1, bmpFile);

        for(int i = 0; i < width; i++)
        {
            PIXEL pixel = imageLineArray[i];
            int gray = (pixel.red + pixel.green + pixel.blue) / 3;
            
            imageLineArray[i].blue = gray;
            imageLineArray[i].green = gray;
            imageLineArray[i].red = gray;
        }

        fwrite(imageLineArray, bytesPerRow, 1, bmpGrayScale);

        for(int i = 0; i < padding; i++)
            fputc(0, bmpGrayScale);

        fseek(bmpFile, padding, SEEK_CUR);
    }
    fclose(bmpGrayScale);
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

int analyseInfoHeader(FILE* bmpFile, int* bmpImageHeight, int* bmpImageWidth, int* bitsPerPixel, int* compression, int* bmpImageSize)
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
    
    *bitsPerPixel = constructShort(&infoHeader[14]);
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