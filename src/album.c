#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "album.h"
#include "raylib.h"

extern Texture2D texAlbumBase;
extern Texture2D texFigComum;
extern Texture2D texFigRara;

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

    // Desenha a arte do album.
    DrawTexturePro(texAlbumBase, 
        (Rectangle){ 0, 0, (float)texAlbumBase.width, (float)texAlbumBase.height }, 
        (Rectangle){ 0, 0, 800, 600 }, 
        (Vector2){ 0, 0 }, 0.0f, WHITE);

    // CABEÇALHO TRANSPARENTE E BUSCA

    Rectangle barraBusca = { 20, 20, 300, 40 };

    if (buscaFocada || filtro[0] != '\0') {
        
        // Desenha o fundo
        DrawRectangleRec(barraBusca, BLUE); 
        
        if (buscaFocada) {
            DrawRectangleLinesEx(barraBusca, 2, DARKBLUE); 
        }
        
        if (filtro[0] != '\0') {
            DrawText(filtro, 30, 30, 20, WHITE); 
        } else if (buscaFocada) {
            DrawText("_", 30, 30, 20, DARKGRAY); 
        }
    }

    // ==========================================
    // LÓGICA DO CONTADOR DE COLADAS
    // ==========================================
    int totalColadas = 0;
    for (int i = 0; i < total; i++) {
        if (album[i].colada) totalColadas++;
    }
    
    DrawText(TextFormat("Coladas: %d / %d", totalColadas, total), 340, 15, 18, BLACK);

    // ==========================================
    // BOTÃO COLAR TODAS
    // ==========================================
    Rectangle btnColarTodas = { 510, 3, 130, 40 };
    bool hoverColar = CheckCollisionPointRec(mouse, btnColarTodas); // Checa se o mouse está em cima
    
    // Fica verde escuro quando passa o mouse por cima para dar feedback visual
    DrawRectangleRec(btnColarTodas, hoverColar ? DARKGREEN : GREEN);
    DrawText("COLAR TODAS", btnColarTodas.x + 12, btnColarTodas.y + 12, 14, hoverColar ? WHITE : BLACK);

    if (hoverColar && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        bool colouAlguma = false;
        
        // Varre o álbum inteiro procurando cartas repetidas não coladas
        for (int i = 0; i < total; i++) {
            if (!album[i].colada && album[i].quantidade > 0) {
                album[i].colada = true;
                album[i].quantidade--;
                colouAlguma = true; 
            }
        }
        
        if (colouAlguma) {
            salvarDadosBinario(album, total);
        }
    }

    DrawText(TextFormat("Página: %d", (*paginaAtual) + 1), 660, 15, 20, WHITE);

    // RENDERIZAÇÃO DAS FIGURINHAS
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

                // Lógica de desenhar a figurinha
                if (colada || qtd > 0) {
                    // Escolhe a textura baseada na raridade
                    Texture2D texturaUsada = texFigComum;
                    if (strstr(album[i].tipo, "Especial") != NULL || 
                        strstr(album[i].tipo, "ESPECIAL") != NULL || 
                        strstr(album[i].tipo, "especial") != NULL) {
                        texturaUsada = texFigRara;
                    }

                    // Desenha a figurinha esticando para 160x190
                    DrawTexturePro(texturaUsada, 
                        (Rectangle){ 0, 0, (float)texturaUsada.width, (float)texturaUsada.height }, 
                        cardArea, 
                        (Vector2){ 0, 0 }, 0.0f, WHITE);

                    // Desenha os textos por cima da arte
                    DrawText(album[i].codigo, posX + 10, posY + 15, 16, DARKGRAY);
                    DrawText(TextSubtext(album[i].titulo, 0, 12), posX + 10, posY + 50, 14, BLACK);

                    if (qtd > 0 && !colada) {
                        // Se tem repetida e não está colada, mostra o botão "Colar"
                        DrawRectangle(posX + 30, posY + 80, 100, 30, Fade(YELLOW, 0.8f));
                        DrawText("COLAR", posX + 55, posY + 88, 14, BLACK);

                        if (CheckCollisionPointRec(mouse, cardArea) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            album[i].colada = true;
                            album[i].quantidade--; 
                            salvarDadosBinario(album, total); 
                        }
                    } else if (colada) {
                        DrawText("COLADA", posX + 50, posY + 85, 14, DARKBLUE);
                        if (qtd > 0) {
                            DrawText(TextFormat("Repetidas: %d", qtd), posX + 10, posY + 165, 14, GREEN);
                        }
                    }
                } 
                else {
                    // O jogador não tem a figurinha.
                    DrawText(album[i].codigo, posX + 60, posY + 85, 20, Fade(GRAY, 0.7f));
                }

                desenhados++;
            }
            count++; 
        }
    }

    // RODAPÉ COM BOTÕES DE PÁGINA
    DrawText("ESC para Menu", 235, 555, 18, BLACK);

    Rectangle btnAnt = { 40, 550, 150, 40 };
    Rectangle btnProx = { 610, 550, 150, 40 };
    
    bool podeVoltar = (*paginaAtual > 0);
    bool podeAvancar = (count > indexInicial + 8);

    // Desenha os botões
    DrawRectangleRec(btnAnt, podeVoltar ? Fade(DARKGRAY, 0.8f) : Fade(BLACK, 0.5f));
    DrawText("<- ANTERIOR (A)", btnAnt.x + 10, btnAnt.y + 12, 15, WHITE);

    DrawRectangleRec(btnProx, podeAvancar ? Fade(DARKGRAY, 0.8f) : Fade(BLACK, 0.5f));
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

// Função para salvar o estado atual do álbum no arquivo binário
void salvarDados(Figurinha *album, int total) {
    FILE *bin = fopen("dados.bin", "wb");
    if (bin != NULL) {
        fwrite(album, sizeof(Figurinha), total, bin);
        fclose(bin);
    }
}
// Função que processa a troca do Mercado Negro
int ExecutarTrocaMercadoNegro(Figurinha *album, int total, int *moedas, int tipoTroca) {
    if (*moedas < 20) return -1; // Retorna -1 se não tiver dinheiro

    int totalRepetidas = 0;
    for (int i = 0; i < total; i++) {
        bool ehEspecial = (strstr(album[i].tipo, "Especial") != NULL || 
                           strstr(album[i].tipo, "ESPECIAL") != NULL || 
                           strstr(album[i].tipo, "especial") != NULL);
        if ((tipoTroca == 1 && !ehEspecial) || (tipoTroca == 2 && ehEspecial)) {
            totalRepetidas += album[i].quantidade;
        }
    }

    if (totalRepetidas < 5) return -1; // Retorna -1 se não tiver repetidas suficientes

    *moedas -= 20;

    // Remove 5 repetidas
    int remover = 5;
    for (int i = 0; i < total && remover > 0; i++) {
        bool ehEspecial = (strstr(album[i].tipo, "Especial") != NULL || 
                           strstr(album[i].tipo, "ESPECIAL") != NULL || 
                           strstr(album[i].tipo, "especial") != NULL);
        if ((tipoTroca == 1 && !ehEspecial) || (tipoTroca == 2 && ehEspecial)) {
            while (album[i].quantidade > 0 && remover > 0) {
                album[i].quantidade--;
                remover--;
            }
        }
    }

    // Sorteio de raridade (10% de chance de virar especial usando comuns)
    bool ganharEspecial = (tipoTroca == 2) || (GetRandomValue(1, 100) <= 10);

    // Filtra figurinhas não coladas da raridade sorteada
    int candidatas[1000];
    int totalCandidatas = 0;
    for (int i = 0; i < total; i++) {
        bool ehEspecial = (strstr(album[i].tipo, "Especial") != NULL || 
                           strstr(album[i].tipo, "ESPECIAL") != NULL || 
                           strstr(album[i].tipo, "especial") != NULL);
        if (!album[i].colada && (ehEspecial == ganharEspecial)) {
            candidatas[totalCandidatas] = i;
            totalCandidatas++;
        }
    }

    // Se já completou tudo daquela raridade, libera uma repetida
    if (totalCandidatas == 0) {
        for (int i = 0; i < total; i++) {
            bool ehEspecial = (strstr(album[i].tipo, "Especial") != NULL || 
                               strstr(album[i].tipo, "ESPECIAL") != NULL || 
                               strstr(album[i].tipo, "especial") != NULL);
            if (ehEspecial == ganharEspecial) {
                candidatas[totalCandidatas] = i;
                totalCandidatas++;
            }
        }
    }

    int sorteada = candidatas[GetRandomValue(0, totalCandidatas - 1)];
    album[sorteada].colada = true; 
    
    salvarDadosBinario(album, total); 
    return sorteada; // Retorna o ID da figurinha sorteada
}