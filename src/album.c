/**
 * @file album.c
 * @brief Implementação da lógica de dados do álbum (Arquivos e Memória).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "album.h"

/**
 * @brief Remove espaços em branco do início e do fim de uma string (Trim).
 * Essencial para tratar o padding do arquivo CSV fornecido.
 */
static void limparEspacos(char *str) {
    if (str == NULL) return;
    
    // Remove espaços do fim
    int l = strlen(str);
    while (l > 0 && isspace((unsigned char)str[l - 1])) {
        str[--l] = 0;
    }

    // Encontra o primeiro caractere não-espaço
    char *p = str;
    while (*p && isspace((unsigned char)*p)) {
        p++;
    }

    // Move a string para o início
    if (p != str) {
        memmove(str, p, strlen(p) + 1);
    }
}

/**
 * @brief Converte uma string para minúsculo para busca case-insensitive.
 */
static void paraMinusculo(char *dest, const char *origem) {
    int i = 0;
    for (i = 0; origem[i]; i++) {
        dest[i] = tolower((unsigned char)origem[i]);
    }
    dest[i] = '\0';
}

Figurinha* carregarDadosIniciais(int *total) {
    Figurinha *album = NULL;
    *total = 0;

    // 1. TENTA CARREGAR O BINÁRIO (Progresso salvo)
    FILE *bin = fopen("dados.bin", "rb");
    if (bin) {
        fseek(bin, 0, SEEK_END);
        long tam = ftell(bin);
        rewind(bin);
        
        *total = (int)(tam / sizeof(Figurinha));
        album = (Figurinha*) malloc(tam);
        if (album) fread(album, sizeof(Figurinha), *total, bin);
        
        fclose(bin);
        printf("Progresso carregado do arquivo binario (%d itens).\n", *total);
        return album;
    }

    // 2. SE NÃO HÁ BINÁRIO, TENTA O CSV (Primeira Execução)
    FILE *csv = fopen("figurinhas2026.csv", "r");
    if (csv) {
        char linha[512];
        fgets(linha, sizeof(linha), csv); // Pula o cabeçalho: codigo, titulo, secao, grupo, tipo

        while (fgets(linha, sizeof(linha), csv)) {
            // Remove o \n
            linha[strcspn(linha, "\r\n")] = 0;

            (*total)++;
            Figurinha *temp = (Figurinha*) realloc(album, (*total) * sizeof(Figurinha));
            if (!temp) {
                printf("Erro: Falha de memoria ao carregar CSV.\n");
                return album;
            }
            album = temp;
            int i = *total - 1;

            // Parsing com strtok usando a vírgula como delimitador
            char *token = strtok(linha, ",");
            if (token) { limparEspacos(token); strcpy(album[i].codigo, token); }

            token = strtok(NULL, ",");
            if (token) { limparEspacos(token); strcpy(album[i].titulo, token); }

            token = strtok(NULL, ",");
            if (token) { limparEspacos(token); strcpy(album[i].secao, token); }

            token = strtok(NULL, ",");
            if (token) { limparEspacos(token); strcpy(album[i].grupo, token); }

            token = strtok(NULL, ",");
            if (token) { limparEspacos(token); strcpy(album[i].tipo, token); }

            // Campos de controle
            album[i].colada = false;
            album[i].paraTroca = 0;
        }
        fclose(csv);
        printf("Primeira execucao: %d figurinhas carregadas do CSV.\n", *total);
        return album;
    }

    printf("Erro: Arquivo CSV 'figurinhas2026.csv' nao encontrado.\n");
    return NULL;
}

void salvarDadosBinario(Figurinha *album, int total) {
    if (album == NULL || total == 0) return;

    FILE *bin = fopen("dados.bin", "wb");
    if (bin) {
        fwrite(album, sizeof(Figurinha), total, bin);
        fclose(bin);
        printf("Dados salvos com sucesso em dados.bin.\n");
    }
}

Figurinha* alocarEspacoVazio(Figurinha *album, int *total) {
    (*total)++;
    Figurinha *novo = (Figurinha*) realloc(album, (*total) * sizeof(Figurinha));
    if (novo) {
        int i = (*total) - 1;
        memset(&novo[i], 0, sizeof(Figurinha));
        // Valores default para evitar lixo de memória
        strcpy(novo[i].codigo, "NOVO");
        strcpy(novo[i].titulo, "Sem Nome");
        novo[i].colada = false;
        novo[i].paraTroca = 0;
        return novo;
    }
    return album;
}

Figurinha* removerFigurinha(Figurinha *album, int *total, int indice) {
    if (indice < 0 || indice >= *total) return album;

    // Shift dos elementos
    for (int i = indice; i < (*total) - 1; i++) {
        album[i] = album[i + 1];
    }

    (*total)--;
    if (*total > 0) {
        return (Figurinha*) realloc(album, (*total) * sizeof(Figurinha));
    } else {
        free(album);
        return NULL;
    }
}

Figurinha* resetarAlbum(Figurinha *album, int *total) {
    if (album) free(album);
    remove("dados.bin"); // Deleta o arquivo de progresso
    return carregarDadosIniciais(total);
}

int pesquisarFigurinha(Figurinha *album, int total, const char *termo) {
    if (strlen(termo) == 0) return -1;

    char termoM[100], tituloM[100], codigoM[20];
    paraMinusculo(termoM, termo);

    for (int i = 0; i < total; i++) {
        paraMinusculo(tituloM, album[i].titulo);
        paraMinusculo(codigoM, album[i].codigo);

        if (strstr(tituloM, termoM) || strstr(codigoM, termoM)) {
            return i;
        }
    }
    return -1;
}

void exportarCSV(Figurinha *album, int total) {
    FILE *f = fopen("exportado.csv", "w");
    if (!f) return;

    fprintf(f, "codigo,titulo,secao,grupo,tipo,colada,paraTroca\n");
    for (int i = 0; i < total; i++) {
        fprintf(f, "%s,%s,%s,%s,%s,%d,%d\n", 
                album[i].codigo, album[i].titulo, album[i].secao, 
                album[i].grupo, album[i].tipo, album[i].colada, album[i].paraTroca);
    }
    fclose(f);
}