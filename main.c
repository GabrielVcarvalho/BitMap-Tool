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

typedef struct
{
    FILE* bmpFile;

    int bmpFileSize;
    int bmpOffSetPosition;

    int imageBmpHeight;
    int imageBmpWidth;

    short bitsPerPixel;

    int compression;
    int bmpSizeImage;

    int bytesPerRow;
    int padding;

} BMPFILE;

#define FILEHEADERSIZE 14
#define INFOHEADERSIZE 40

//File Analyse
int isBmp (BYTE firstByte, BYTE secondByte);
int analyseFileHeader(BMPFILE* bmpFileObj);
int analyseInfoHeader(BMPFILE* bmpFileObj);
int isSupportedBmpFile(BMPFILE* bmpFileObj);

//Filters
int transformGrayScale(BMPFILE* bmpFileObj);
int horizontalFlip(BMPFILE* bmpFileObj);
int rotate180degrees(BMPFILE* bmpFileObj);
int rotate90DegreesClockWise(BMPFILE* bmpFileObj);

//Helpers
void storeIntInByteArray(BYTE* startByte, int value);
int copyHeader(FILE* referenceBmp, FILE* outPutBmp, int bmpOffSetPosition);
void reversePixelArray(PIXEL* firstPixel, PIXEL* lastPixel);
short constructShort(BYTE* startByte);
int constructInt(BYTE* startByte);

int main(int argc, char *argv[])
{
    BMPFILE bmpFileObj;

    if(argc < 3)
    {
        printf("The program structure is: <./program_name> <image.bmp> <filter>");
        return 1;
    }

    bmpFileObj.bmpFile = fopen(argv[1], "rb");

    if(bmpFileObj.bmpFile == NULL)
        return 1;

    if(isSupportedBmpFile(&bmpFileObj) != 0)
        return 1;

    bmpFileObj.bytesPerRow =
        bmpFileObj.imageBmpWidth * (bmpFileObj.bitsPerPixel / 8);

    bmpFileObj.padding =
        (4 - (bmpFileObj.bytesPerRow % 4)) % 4;

    if(strcmp(argv[2], "grayScale") == 0)
    {
        transformGrayScale(&bmpFileObj);

        printf("Sucess to read and copy image\n");
    }
    else if(strcmp(argv[2], "horizontalFlip") == 0)
    {
        horizontalFlip(&bmpFileObj);

        printf("Sucess to horizontal flip image\n");
    }
    else if(strcmp(argv[2], "rotate180Degrees") == 0)
    {
        rotate180degrees(&bmpFileObj);

        printf("Sucess to apply 180 degrees rotate");
    }
    else if(strcmp(argv[2], "rotate90DegreesClockWise") == 0)
    {
        rotate90DegreesClockWise(&bmpFileObj);

        printf("Sucess to apply 90 degrees clock wise rotate");
    }
    else
    {
        printf("Filter not found\n");
    }

    fclose(bmpFileObj.bmpFile);

    return 0;
}

int analyseFileHeader(BMPFILE* bmpFileObj)
{
    BYTE fileHeader[FILEHEADERSIZE];

    if(fread(fileHeader, FILEHEADERSIZE, 1, bmpFileObj->bmpFile) != 1)
    {
        printf("The program could not read header\n");
        return 1;
    }

    if(!isBmp(fileHeader[0], fileHeader[1]))
    {
        printf("You can't open others file formats, just .bmp\n");
        return 1;
    }

    bmpFileObj->bmpFileSize =
        constructInt(&fileHeader[2]);

    bmpFileObj->bmpOffSetPosition =
        constructInt(&fileHeader[10]);

    return 0;
}

int analyseInfoHeader(BMPFILE* bmpFileObj)
{
    BYTE infoHeader[INFOHEADERSIZE];

    if(fread(infoHeader, INFOHEADERSIZE, 1, bmpFileObj->bmpFile) != 1)
    {
        printf("The program could not read info header\n");
        return 1;
    }

    bmpFileObj->imageBmpWidth =
        constructInt(&infoHeader[4]);

    bmpFileObj->imageBmpHeight =
        constructInt(&infoHeader[8]);

    bmpFileObj->bitsPerPixel =
        constructShort(&infoHeader[14]);

    bmpFileObj->compression =
        constructInt(&infoHeader[16]);

    bmpFileObj->bmpSizeImage =
        constructInt(&infoHeader[20]);

    return 0;
}

int isSupportedBmpFile(BMPFILE* bmpFileObj)
{
    if(analyseFileHeader(bmpFileObj) != 0)
        return 1;

    if(analyseInfoHeader(bmpFileObj) != 0)
        return 1;

    if(bmpFileObj->compression != 0)
    {
        printf("Have not suport to compressed file\n");
        return 1;
    }

    if(bmpFileObj->bitsPerPixel != 24)
    {
        printf("Images with less or more than three bytes per pixel are not supported\n");
        return 1;
    }

    if(bmpFileObj->bmpOffSetPosition < 54)
    {
        printf("Image with not supported header type\n");
        return 1;
    }

    return 0;
}

int transformGrayScale(BMPFILE* bmpFileObj)
{
    FILE* bmpGrayScale = fopen("imagemGray.bmp", "wb");

    if(copyHeader(
        bmpFileObj->bmpFile,
        bmpGrayScale,
        bmpFileObj->bmpOffSetPosition) != 0)
    {
        return 1;
    }

    for(int line = 0; line < bmpFileObj->imageBmpHeight; line++)
    {
        PIXEL imageLineArray[bmpFileObj->imageBmpWidth];

        fread(
            imageLineArray,
            bmpFileObj->bytesPerRow,
            1,
            bmpFileObj->bmpFile
        );

        for(int i = 0; i < bmpFileObj->imageBmpWidth; i++)
        {
            PIXEL pixel = imageLineArray[i];

            int gray =
                (pixel.red + pixel.green + pixel.blue) / 3;

            imageLineArray[i].blue = gray;
            imageLineArray[i].green = gray;
            imageLineArray[i].red = gray;
        }

        fwrite(
            imageLineArray,
            bmpFileObj->bytesPerRow,
            1,
            bmpGrayScale
        );

        for(int i = 0; i < bmpFileObj->padding; i++)
            fputc(0, bmpGrayScale);

        fseek(
            bmpFileObj->bmpFile,
            bmpFileObj->padding,
            SEEK_CUR
        );
    }

    fclose(bmpGrayScale);

    return 0;
}

int horizontalFlip(BMPFILE* bmpFileObj)
{
    FILE* bmpHorizontalFlip =
        fopen("HorizontalFlipImage.bmp", "wb");

    if(copyHeader(
        bmpFileObj->bmpFile,
        bmpHorizontalFlip,
        bmpFileObj->bmpOffSetPosition) != 0)
    {
        return 1;
    }

    PIXEL bmpRowArray[bmpFileObj->imageBmpWidth];

    for(int y = 0; y < bmpFileObj->imageBmpHeight; y++)
    {
        fread(
            bmpRowArray,
            bmpFileObj->bytesPerRow,
            1,
            bmpFileObj->bmpFile
        );

        reversePixelArray(
            bmpRowArray,
            &bmpRowArray[bmpFileObj->imageBmpWidth - 1]
        );

        fwrite(
            bmpRowArray,
            bmpFileObj->bytesPerRow,
            1,
            bmpHorizontalFlip
        );

        for(int i = 0; i < bmpFileObj->padding; i++)
            fputc(0, bmpHorizontalFlip);

        fseek(
            bmpFileObj->bmpFile,
            bmpFileObj->padding,
            SEEK_CUR
        );
    }

    fclose(bmpHorizontalFlip);

    return 0;
}

int rotate180degrees(BMPFILE* bmpFileObj)
{
    FILE* bmpRotate180degrees =
        fopen("Rotate180degreesImage.bmp", "wb");

    if(copyHeader(
        bmpFileObj->bmpFile,
        bmpRotate180degrees,
        bmpFileObj->bmpOffSetPosition) != 0)
    {
        return 1;
    }

    int atualRowIndex = 0;

    int otherSideRowIndex =
        bmpFileObj->imageBmpHeight - 1;

    int atualRowPosition = 0;
    int otherSideRowPosition = 0;

    PIXEL atualRowArray[bmpFileObj->imageBmpWidth];

    PIXEL otherSideRowArray[bmpFileObj->imageBmpWidth];

    while(atualRowIndex < otherSideRowIndex)
    {
        atualRowPosition =
            ((bmpFileObj->padding +
            bmpFileObj->bytesPerRow)
            * atualRowIndex)
            + bmpFileObj->bmpOffSetPosition;

        otherSideRowPosition =
            ((bmpFileObj->padding +
            bmpFileObj->bytesPerRow)
            * otherSideRowIndex)
            + bmpFileObj->bmpOffSetPosition;

        fseek(
            bmpFileObj->bmpFile,
            atualRowPosition,
            SEEK_SET
        );

        fread(
            atualRowArray,
            bmpFileObj->bytesPerRow,
            1,
            bmpFileObj->bmpFile
        );

        fseek(
            bmpFileObj->bmpFile,
            otherSideRowPosition,
            SEEK_SET
        );

        fread(
            otherSideRowArray,
            bmpFileObj->bytesPerRow,
            1,
            bmpFileObj->bmpFile
        );

        reversePixelArray(
            atualRowArray,
            &atualRowArray[bmpFileObj->imageBmpWidth - 1]
        );

        reversePixelArray(
            otherSideRowArray,
            &otherSideRowArray[bmpFileObj->imageBmpWidth - 1]
        );

        fseek(
            bmpRotate180degrees,
            atualRowPosition,
            SEEK_SET
        );

        fwrite(
            otherSideRowArray,
            bmpFileObj->bytesPerRow,
            1,
            bmpRotate180degrees
        );

        for(int i = 0; i < bmpFileObj->padding; i++)
        {
            fputc(0, bmpRotate180degrees);
        }

        fseek(
            bmpRotate180degrees,
            otherSideRowPosition,
            SEEK_SET
        );

        fwrite(
            atualRowArray,
            bmpFileObj->bytesPerRow,
            1,
            bmpRotate180degrees
        );

        for(int i = 0; i < bmpFileObj->padding; i++)
        {
            fputc(0, bmpRotate180degrees);
        }

        atualRowIndex++;
        otherSideRowIndex--;
    }

    if((bmpFileObj->imageBmpHeight % 2) != 0)
    {
        int middleRowPosition =
            ((bmpFileObj->padding +
            bmpFileObj->bytesPerRow)
            * atualRowIndex)
            + bmpFileObj->bmpOffSetPosition;

        fseek(
            bmpFileObj->bmpFile,
            middleRowPosition,
            SEEK_SET
        );

        fread(
            atualRowArray,
            bmpFileObj->bytesPerRow,
            1,
            bmpFileObj->bmpFile
        );

        reversePixelArray(
            atualRowArray,
            &atualRowArray[bmpFileObj->imageBmpWidth - 1]
        );

        fseek(
            bmpRotate180degrees,
            middleRowPosition,
            SEEK_SET
        );

        fwrite(
            atualRowArray,
            bmpFileObj->bytesPerRow,
            1,
            bmpRotate180degrees
        );

        for(int i = 0; i < bmpFileObj->padding; i++)
        {
            fputc(0, bmpRotate180degrees);
        }
    }

    fclose(bmpRotate180degrees);

    return 0;
}

int rotate90DegreesClockWise(BMPFILE* bmpFileObj)
{
    FILE* bmpRotate90 =
        fopen("Rotate90ClockWiseImage.bmp", "wb");

    if(bmpRotate90 == NULL)
        return 1;

    if(copyHeader(
        bmpFileObj->bmpFile,
        bmpRotate90,
        bmpFileObj->bmpOffSetPosition) != 0)
    {
        return 1;
    }

    int newWidth =
        bmpFileObj->imageBmpHeight;

    int newHeight =
        bmpFileObj->imageBmpWidth;

    int newBytesPerRow =
        newWidth * (bmpFileObj->bitsPerPixel / 8);

    int newPadding =
        (4 - (newBytesPerRow % 4)) % 4;

    /*
        Header width position  -> byte 18
        Header height position -> byte 22
    */

    fseek(bmpRotate90, 18, SEEK_SET);

    BYTE widthAndHeightBytes[8];

    storeIntInByteArray(
        widthAndHeightBytes,
        newWidth
    );

    storeIntInByteArray(
        &widthAndHeightBytes[4],
        newHeight
    );

    fwrite(widthAndHeightBytes, 8, 1, bmpRotate90);

    PIXEL rowOriginalImageArray[bmpFileObj->imageBmpWidth];

    int originalRowPosition = 0;
    int destinationPixelPosition = 0;

    for(int y = 0; y < bmpFileObj->imageBmpHeight; y++)
    {
        originalRowPosition =
            ((bmpFileObj->bytesPerRow +
            bmpFileObj->padding) * y)
            + bmpFileObj->bmpOffSetPosition;

        fseek(
            bmpFileObj->bmpFile,
            originalRowPosition,
            SEEK_SET
        );

        fread(
            rowOriginalImageArray,
            bmpFileObj->bytesPerRow,
            1,
            bmpFileObj->bmpFile
        );

        for(int x = 0; x < bmpFileObj->imageBmpWidth; x++)
        {
            destinationPixelPosition =
                bmpFileObj->bmpOffSetPosition +

                ((x * (newBytesPerRow + newPadding))) +

                ((newWidth - 1 - y) * sizeof(PIXEL));

            fseek(
                bmpRotate90,
                destinationPixelPosition,
                SEEK_SET
            );

            fwrite(
                &rowOriginalImageArray[x],
                sizeof(PIXEL),
                1,
                bmpRotate90
            );
        }
    }

    BYTE paddingBytes[3] = {0, 0, 0};

    for(int row = 0; row < newHeight; row++)
    {
        int paddingPosition =
            bmpFileObj->bmpOffSetPosition +

            (row * (newBytesPerRow + newPadding)) +

            newBytesPerRow;

        fseek(
            bmpRotate90,
            paddingPosition,
            SEEK_SET
        );

        fwrite(
            paddingBytes,
            newPadding,
            1,
            bmpRotate90
        );
    }

    fclose(bmpRotate90);

    return 0;

}

void storeIntInByteArray(BYTE* startByte, int value)
{
    startByte[0] = value & 0xFF;

    startByte[1] =
        (value >> 8) & 0xFF;

    startByte[2] =
        (value >> 16) & 0xFF;

    startByte[3] =
        (value >> 24) & 0xFF;
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