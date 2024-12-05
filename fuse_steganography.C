#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Estruturas cabeçalho BMP
#pragma pack(push, 1)  
typedef struct {
    unsigned short bfType;
    unsigned int bfSize;
    unsigned short bfReserved1;
    unsigned short bfReserved2;
    unsigned int bfOffBits;
} BITMAPFILEHEADER;

typedef struct {
    unsigned int biSize;
    int biWidth;
    int biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int biCompression;
    unsigned int biSizeImage;
    int biXPelsPerMeter;
    int biYPelsPerMeter;
    unsigned int biClrUsed;
    unsigned int biClrImportant;
} BITMAPINFOHEADER;
#pragma pack(pop)

// Função para ler os dados de um BMP
void ler_bmp(const char *filename, BITMAPFILEHEADER *fileHeader, BITMAPINFOHEADER *infoHeader, unsigned char **imageData) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Não foi possível abrir o arquivo");
        return;
    }

    // Ler cabeçalhos
    fread(fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
    fread(infoHeader, sizeof(BITMAPINFOHEADER), 1, file);

    // Mover para o início dos dados da imagem
    fseek(file, fileHeader->bfOffBits, SEEK_SET);

    // Calcular o número total de pixels
    int width = infoHeader->biWidth;
    int height = infoHeader->biHeight;
    int size = width * height * 3;  

    // Alocação de memória 
    *imageData = (unsigned char*) malloc(size);
    fread(*imageData, 1, size, file);

    fclose(file);
}

// Função para esconder os dados na imagem BMP
void esconder_dados(unsigned char *imageData, const char *data, int dataSize) {
    int byteIndex = 0;
    int bitIndex = 0;

    for (int i = 0; i < dataSize; i++) {
        unsigned char byte = data[i];
        for (int j = 7; j >= 0; j--) {
            // Substitui o LSB do pixel com o bit do dado
            int pixelIndex = byteIndex * 3;  
            unsigned char mask = 1 << j;
            unsigned char bit = (byte & mask) >> j;

            // Modificar o LSB do pixel
            imageData[pixelIndex] &= 0xFE;  // Limpa o LSB
            imageData[pixelIndex] |= bit;   // Coloca o bit do dado no LSB

            byteIndex++;
            if (byteIndex == dataSize) break;
        }
    }
}

// Função para salvar o BMP com os dados ocultos
void salvar_bmp(const char *filename, unsigned char *imageData, BITMAPFILEHEADER *fileHeader, BITMAPINFOHEADER *infoHeader) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Não foi possível salvar o arquivo");
        return;
    }

    // Escrever cabeçalhos
    fwrite(fileHeader, sizeof(BITMAPFILEHEADER), 1, file);
    fwrite(infoHeader, sizeof(BITMAPINFOHEADER), 1, file);

    // Escrever os dados da imagem
    fwrite(imageData, 1, infoHeader->biWidth * infoHeader->biHeight * 3, file);

    fclose(file);
}

// Função para extrair os dados ocultos do BMP
void extrair_dados(unsigned char *imageData, int dataSize) {
    int byteIndex = 0;
    int bitIndex = 0;

    // Alocar memória
    char *extractedData = (char*) malloc(dataSize);

    for (int i = 0; i < dataSize; i++) {
        unsigned char byte = 0;
        for (int j = 7; j >= 0; j--) {
            // Pega o LSB do pixel e coloca no bit correspondente
            int pixelIndex = byteIndex * 3;  
            byte |= ((imageData[pixelIndex] & 0x01) << j);  // Pega o LSB do pixel e insere no byte
            byteIndex++;
        }
        extractedData[i] = byte;
    }

    // Imprimir os dados extraídos
    printf("Dados extraídos:\n%s\n", extractedData);
    free(extractedData);
}

int main() {
    const char *inputFilename = "input.bmp";   // Nome da imagem BMP de entrada
    const char *outputFilename = "output.bmp"; // Nome da imagem BMP de saída
    const char *fileToHide = "file.txt";       // Nome do arquivo a ser escondido

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    unsigned char *imageData = NULL;

    // Ler a imagem BMP
    ler_bmp(inputFilename, &fileHeader, &infoHeader, &imageData);

    // Ler o arquivo a ser escondido
    FILE *file = fopen(fileToHide, "rb");
    if (!file) {
        perror("Erro ao abrir o arquivo a ser ocultado");
        return 1;
    }
    fseek(file, 0, SEEK_END);
    int fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Alocar memória para os dados do arquivo
    char *fileData = (char*) malloc(fileSize);
    fread(fileData, 1, fileSize, file);
    fclose(file);

    // Embutir os dados no BMP
    esconder_dados(imageData, fileData, fileSize);

    // Salvar a imagem modificada
    salvar_bmp(outputFilename, imageData, &fileHeader, &infoHeader);

    // Limpeza
    free(imageData);
    free(fileData);

    printf("Arquivo oculto com sucesso.\n");

    // Extrair dados ocultos
    printf("\nVerificando a extração dos dados...\n");
    unsigned char *extractedData = (unsigned char*) malloc(fileSize); 
    ler_bmp(outputFilename, &fileHeader, &infoHeader, &imageData); 
    extrair_dados(imageData, fileSize);

    // Limpeza final
    free(extractedData);
    free(imageData);

    return 0;
}
