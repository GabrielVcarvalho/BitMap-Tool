#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char BYTE;
typedef struct
{
    BYTE blue;
    BYTE green;
    BYTE red;
} PIXEL;

#define FILEHEADERSIZE 14
#define INFOHEADERSIZE 40

//File Analyse
int isBmp (BYTE firstByte, BYTE secondByte);
int analyseFileHeader(FILE* bmpFile, int* bmpFileSize, int* bmpOffSetPosition);
int analyseInfoHeader(FILE* bmpFile, int* bmpImageHeight, int* bmpImageWidth, short* bitsByPixel, int* compression, int* bmpImageSize);
int isSupportedBmpFile(FILE* bmpFile, int* bmpFileSize, int* bmpOffSetPosition,
    int* imageBmpHeight, int* imageBmpWidth, short* bitsPerPixel, int* compression, int* bmpSizeImage);

//Filters
int transformGrayScale(FILE* bmpFile ,int bmpOffSetPosition, int bitsPerPixel, int height, int width, int bytesPerRow, int padding);
int horizontalFlip(FILE* bmpFile, int bmpOffSetPosition, int bitsPerPixel, int height, int width, int bytesPerRow, int padding);
int rotate180degrees(FILE* bmpFile, int bmpOffSetPosition, int bitsPerPixel, int height, int width, int bytesPerRow, int padding);

//Helpers
int copyHeader(FILE* referenceBmp, FILE* outPutBmp, int bmpOffSetPosition);
void reversePixelArray(PIXEL* firstPixel, PIXEL* lastPixel);
short constructShort(BYTE* startByte);
int constructInt(BYTE* startByte);

int main(int argc, char *argv[])
{

    FILE *bmpFile;
    int bmpFileSize;
    int bmpOffSetPosition;
    int imageBmpHeight, imageBmpWidth;
    short bitsPerPixel;
    int compression;
    int bmpSizeImage;
    int bytesPerRow;
    int padding;

    if (argc < 3)
    {
        printf("The program structure is: <./program_name> <image.bmp> <filter>");
        return 1;
    }

    bmpFile = fopen(argv[1], "rb");

    if(bmpFile == NULL)
        return 1;

    if(isSupportedBmpFile(bmpFile, &bmpFileSize, &bmpOffSetPosition, &imageBmpHeight, &imageBmpWidth, &bitsPerPixel, &compression, &bmpSizeImage) != 0)
        return 1;

    bytesPerRow = imageBmpWidth * (bitsPerPixel / 8); 
    padding = (4 - (bytesPerRow % 4)) % 4;

    if(strcmp(argv[2], "grayScale") == 0)
    {
        transformGrayScale(
            bmpFile,
            bmpOffSetPosition,
            bitsPerPixel,
            imageBmpHeight,
            imageBmpWidth,
            bytesPerRow,
            padding
        );
            
            printf("Sucess to read and copy image\n");
    }
    else if(strcmp(argv[2], "horizontalFlip") == 0)
    {
        horizontalFlip(
        bmpFile,
        bmpOffSetPosition,
        bitsPerPixel,
        imageBmpHeight,
        imageBmpWidth,
        bytesPerRow,
        padding
    );

    printf("Sucess to horizontal flip image\n");
    }
    else if(strcmp(argv[2], "rotate180Degress") == 0)
    {
        rotate180degrees(
            bmpFile,
            bmpOffSetPosition,
            bitsPerPixel,
            imageBmpHeight,
            imageBmpWidth,
            bytesPerRow,
            padding
    );

    printf("Sucess to apply 180 degrees rotate");
    }
    else
    {
        printf("Filter not found\n");
    }

    fclose(bmpFile);
    return 0;
}

int analyseFileHeader(FILE* bmpFile, int* bmpFileSize, int* bmpOffSetPosition)
{
    BYTE fileHeader[FILEHEADERSIZE];

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

int analyseInfoHeader(FILE* bmpFile, int* bmpImageHeight, int* bmpImageWidth, short* bitsPerPixel, int* compression, int* bmpImageSize)
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

int isSupportedBmpFile(
    FILE* bmpFile,
    int* bmpFileSize,
    int* bmpOffSetPosition,
    int* imageBmpHeight,
    int* imageBmpWidth,
    short* bitsPerPixel,
    int* compression,
    int* bmpSizeImage)
{
    if(analyseFileHeader(bmpFile, bmpFileSize, bmpOffSetPosition) != 0)
        return 1;

    if(analyseInfoHeader(bmpFile, imageBmpHeight, imageBmpWidth, bitsPerPixel, compression, bmpSizeImage) != 0)
        return 1;

    if(*compression != 0)
    {
        printf("Have not suport to compressed file\n");
        return 1;
    }

    if(*bitsPerPixel != 24)
    {
        printf("Images with less or more than three bytes per pixel are not supported\n");
        return 1;
    }

    if(*bmpOffSetPosition < 54)
    {
        printf("Image with not supported header type\n");
        return 1;
    }

    return 0;
}

int transformGrayScale(FILE* bmpFile ,int bmpOffSetPosition, int bitsPerPixel, int height, int width, int bytesPerRow, int padding)
{
    FILE* bmpGrayScale = fopen("imagemGray.bmp", "wb");

    if(copyHeader(bmpFile, bmpGrayScale, bmpOffSetPosition) != 0)
        return 1;
    
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

int horizontalFlip(FILE* bmpFile, int bmpOffSetPosition, int bitsPerPixel, int height, int width, int bytesPerRow, int padding)
{
    FILE* bmpHorizontalFlip = fopen("HorizontalFlipImage.bmp", "wb");

    if(copyHeader(bmpFile, bmpHorizontalFlip, bmpOffSetPosition) != 0)
        return 1;

    PIXEL bmpRowArray[width];

    for(int y = 0; y < height; y++)
    {
        fread(bmpRowArray, bytesPerRow, 1, bmpFile);
        reversePixelArray(bmpRowArray, &bmpRowArray[width - 1]);
        fwrite(bmpRowArray, bytesPerRow, 1, bmpHorizontalFlip);

        for(int i = 0; i < padding; i++)
            fputc(0, bmpHorizontalFlip);

        fseek(bmpFile, padding, SEEK_CUR);
    }

    fclose(bmpHorizontalFlip);

    return 0;
}

int rotate180degrees(FILE* bmpFile, int bmpOffSetPosition, int bitsPerPixel, int height, int width, int bytesPerRow, int padding)
{
    FILE* bmpRotate180degrees = fopen("Rotate180degreesImage.bmp", "wb");

    if(copyHeader(bmpFile, bmpRotate180degrees, bmpOffSetPosition) != 0)
        return 1;

    int atualRowIndex = 0;
    int otherSideRowIndex = height - 1;
    int atualRowPosition = 0;
    int otherSideRowPosition = 0;

    PIXEL atualRowArray[width];
    PIXEL otherSideRowArray[width];

    while (atualRowIndex < otherSideRowIndex)
    {
        atualRowPosition = ((padding + bytesPerRow) * atualRowIndex) + bmpOffSetPosition;
        otherSideRowPosition = ((padding + bytesPerRow) * otherSideRowIndex) + bmpOffSetPosition;

        fseek(bmpFile, atualRowPosition, SEEK_SET);
        fread(atualRowArray, bytesPerRow, 1, bmpFile);

        fseek(bmpFile, otherSideRowPosition, SEEK_SET);
        fread(otherSideRowArray, bytesPerRow, 1, bmpFile);

        reversePixelArray(atualRowArray, &atualRowArray[width - 1]);
        reversePixelArray(otherSideRowArray, &otherSideRowArray[width - 1]);

        fseek(bmpRotate180degrees, atualRowPosition, SEEK_SET);
        fwrite(otherSideRowArray, bytesPerRow, 1, bmpRotate180degrees);
        
        for(int i = 0; i < padding; i++)
        {
            fputc(0, bmpRotate180degrees);
        }

        fseek(bmpRotate180degrees, otherSideRowPosition, SEEK_SET);
        fwrite(atualRowArray, bytesPerRow, 1, bmpRotate180degrees);

        for(int i = 0; i < padding; i++)
        {
            fputc(0, bmpRotate180degrees);
        }

        atualRowIndex++;
        otherSideRowIndex--;
    }

    if((height % 2) != 0)
    {
        int middleRowPosition = ((padding + bytesPerRow) * atualRowIndex) + bmpOffSetPosition;

        fseek(bmpFile, middleRowPosition, SEEK_SET);
        fread(atualRowArray, bytesPerRow, 1, bmpFile);

        reversePixelArray(atualRowArray, &atualRowArray[width - 1]);

        fseek(bmpRotate180degrees, middleRowPosition, SEEK_SET);

        fwrite(
            atualRowArray,
            bytesPerRow,
            1,
            bmpRotate180degrees
        );

        for(int i = 0; i < padding; i++)
            fputc(0, bmpRotate180degrees);
    }
    
    fclose(bmpRotate180degrees);
    return 0;
}

int constructInt(BYTE* startByte)
{
    int byteSize = 8;
    int bytesInOneInt = 4;
    int finalInt = 0;

    for(int i = 0; i < bytesInOneInt; i++)
    {
        finalInt |= (((unsigned int) *(startByte + i)) << (byteSize * i));
    }
    
    return finalInt;
}

int copyHeader(FILE* referenceBmp, FILE* outPutBmp, int bmpOffSetPosition)
{
    BYTE allHeader[bmpOffSetPosition];
    fseek(referenceBmp, 0, SEEK_SET);
    fread(allHeader, bmpOffSetPosition, 1, referenceBmp);

    if(outPutBmp == NULL)
        return 1;

    fwrite(allHeader, bmpOffSetPosition, 1, outPutBmp);

    return 0;
}

void reversePixelArray(PIXEL* firstPixel, PIXEL* lastPixel)
{
    PIXEL* atualPixel = firstPixel;
    PIXEL* otherSidePixel = lastPixel;

    while(atualPixel < otherSidePixel)
    {
        PIXEL temp = *atualPixel;

        *atualPixel = *otherSidePixel;
        *otherSidePixel = temp;

        atualPixel++;
        otherSidePixel--;
    }
}

short constructShort(BYTE* startByte)
{
    return startByte[0] | ((unsigned short)startByte[1] << 8);
}

int isBmp (BYTE firstByte, BYTE secondByte)
{
    return firstByte == 'B' && secondByte == 'M';
}