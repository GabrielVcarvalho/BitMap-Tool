#include <stdio.h>
#include <stdlib.h>

typedef unsigned char BYTE;
#define HEADERSIZE 54

int isBmp (BYTE firstByte, BYTE secondByte);

int main(int argc, char *argv[])
{
    FILE *bmpFile = fopen("imagem.bmp", "rb");
    BYTE header[HEADERSIZE];

    if(bmpFile == NULL)
        return 1;

    if(fread(header, HEADERSIZE, 1, bmpFile) != 1)
    {
        printf("Erro ao ler o header\n");
        return 1;
    }

    if(!isBmp(header[0], header[1]))
    {
        printf("You can't open others file formats, just .bmp");
        return 1;
    }

    fclose(bmpFile);
    return 0;
}

int isBmp (BYTE firstByte, BYTE secondByte)
{
    return firstByte == 'B' && secondByte == 'M';
}