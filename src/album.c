/**
 * @file album.c
 * @brief Implementação da lógica de dados do álbum (Arquivos e Memória).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "album.h"
#include "raylib.h"

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

    // com progresso salvo o binário é carregado
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

    // Primeira Execução onde sem binário usa o csv
    FILE *csv = fopen("figurinhas2026.csv", "r");
    if (csv) {
        char linha[512];
        fgets(linha, sizeof(linha), csv); // Pula o cabeçalho: codigo, titulo, secao, grupo, tipo

        while (fgets(linha, sizeof(linha), csv)) {
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
            album[i].quantidade = 0;
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

void DesenharAlbumGrade(Figurinha *album, int total, int *paginaAtual, char *filtro, bool buscaFocada) {
    Vector2 mouse = GetMousePosition();
    int desenhados = 0;
    int count = 0;
    int indexInicial = (*paginaAtual) * 8;

    // cabeçalho com a Caixa de Busca Clicável
    DrawRectangle(0, 0, 800, 80, DARKBLUE);
    
    Rectangle barraBusca = { 20, 20, 300, 40 };
    DrawRectangleRec(barraBusca, buscaFocada ? RAYWHITE : LIGHTGRAY);
    DrawRectangleLines(barraBusca.x, barraBusca.y, barraBusca.width, barraBusca.height, buscaFocada ? RED : GRAY);

    if (filtro[0] == '\0') DrawText("Clique aqui para buscar...", 30, 32, 18, GRAY);
    else DrawText(filtro, 30, 30, 20, BLACK);

    DrawText(TextFormat("Página: %d", (*paginaAtual) + 1), 650, 30, 20, LIGHTGRAY);

    // Renderização
    for (int i = 0; i < total; i++) {
        if (filtro[0] == '\0' || strstr(album[i].codigo, filtro) != NULL) {
            if (count >= indexInicial && desenhados < 8) {
                int coluna = desenhados % 4;
                int linha = desenhados / 4;
                int posX = 40 + (coluna * 185);
                int posY = 110 + (linha * 210);
                Rectangle cardArea = { (float)posX, (float)posY, 160, 190 };

                bool colada = album[i].colada;
                int qtd = album[i].quantidade; 

                if (colada) {
                    DrawRectangleRec(cardArea, RAYWHITE);
                    DrawRectangleLines(posX, posY, 160, 190, BLUE);
                    DrawText("COLADA", posX + 50, posY + 85, 14, DARKBLUE);
                    DrawText(TextFormat("Repetidas: %d", qtd), posX + 10, posY + 165, 14, GREEN);
                } 
                else if (qtd > 0) {
                    DrawRectangleRec(cardArea, YELLOW);
                    DrawRectangleLines(posX, posY, 160, 190, ORANGE);
                    DrawText("CLIQUE P/", posX + 45, posY + 80, 14, BLACK);
                    DrawText("COLAR", posX + 55, posY + 100, 14, BLACK);

                    if (CheckCollisionPointRec(mouse, cardArea) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        album[i].colada = true;
                        album[i].quantidade--; 
                        salvarDados(album, total); 
                    }
                } 
                else {
                    DrawRectangleRec(cardArea, Fade(LIGHTGRAY, 0.5f));
                    DrawRectangleLines(posX, posY, 160, 190, GRAY);
                    DrawText("BLOQUEADO", posX + 35, posY + 85, 14, GRAY);
                }

                DrawText(album[i].codigo, posX + 10, posY + 15, 16, (colada || qtd>0) ? DARKGRAY : GRAY);
                DrawText(TextSubtext(album[i].titulo, 0, 12), posX + 10, posY + 50, 14, (colada || qtd>0) ? BLACK : DARKGRAY);

                desenhados++;
            }
            count++; 
        }
    }

    // Rodapé com Botões de Página
    DrawRectangle(0, 540, 800, 60, LIGHTGRAY);
    DrawText("ESC para Menu", 340, 560, 18, DARKGRAY);

    Rectangle btnAnt = { 40, 550, 150, 40 };
    Rectangle btnProx = { 610, 550, 150, 40 };
    
    bool podeVoltar = (*paginaAtual > 0);
    bool podeAvancar = (count > indexInicial + 8);

    // Desenha os botões
    DrawRectangleRec(btnAnt, podeVoltar ? DARKGRAY : GRAY);
    DrawText("<- ANTERIOR (A)", btnAnt.x + 10, btnAnt.y + 12, 15, WHITE);

    DrawRectangleRec(btnProx, podeAvancar ? DARKGRAY : GRAY);
    DrawText("PROXIMA (D) ->", btnProx.x + 15, btnProx.y + 12, 15, WHITE);

    // lógica de click para mudar de página
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (podeVoltar && CheckCollisionPointRec(mouse, btnAnt)) (*paginaAtual)--;
        if (podeAvancar && CheckCollisionPointRec(mouse, btnProx)) (*paginaAtual)++;
    }

    // lógica para funcionar o A e D apenas se não estiver digitando
    if (!buscaFocada) {
        if (IsKeyPressed(KEY_A) && podeVoltar) (*paginaAtual)--;
        if (IsKeyPressed(KEY_D) && podeAvancar) (*paginaAtual)++;
    }
}

// Função simples para salvar o estado atual do álbum no arquivo binário
void salvarDados(Figurinha *album, int total) {
    FILE *bin = fopen("dados.bin", "wb");
    if (bin != NULL) {
        fwrite(album, sizeof(Figurinha), total, bin);
        fclose(bin);
    }
}