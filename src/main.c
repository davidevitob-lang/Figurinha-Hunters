#include "raylib.h"
#include "album.h"
#include "map.h"
#include "cadastro.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef enum { ESTADO_MENU = 0, ESTADO_JOGAR, ESTADO_FIGURINHAS, ESTADO_CADASTRO } EstadoJogo;

int main(void) {
    int total = 0;
    Figurinha *meuAlbum = carregarDadosIniciais(&total);
    EstadoJogo estado = ESTADO_MENU;
    int indiceEdicao = -1; 
    bool continuarRodando = true;

    // Variáveis de busca e scroll
    char busca[50] = "\0";
    char buscaAntiga[50] = "\0"; // Para detectar mudança na busca
    float scrollOffset = 0;
    int resultadoBusca = -1;
    bool arrastandoScroll = false; // Controle da barra de rolagem

    InitWindow(800, 600, "Figurinha Hunters 2026 - Pro Edition");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    InitMap();

    Rectangle btnNova = { 630, 30, 140, 40 };
    Rectangle btnReset = { 480, 30, 140, 40 };
    Rectangle areaScroll = { 785, 105, 15, 475 }; // Área da barra de rolagem

    while (continuarRodando && !WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();

        switch (estado) {
            case ESTADO_MENU:
                if (CheckCollisionPointRec(mouse, (Rectangle){300,200,200,50}) && IsMouseButtonReleased(0)) estado = ESTADO_JOGAR;
                if (CheckCollisionPointRec(mouse, (Rectangle){300,270,200,50}) && IsMouseButtonReleased(0)) estado = ESTADO_FIGURINHAS;
                if (CheckCollisionPointRec(mouse, (Rectangle){300,340,200,50}) && IsMouseButtonReleased(0)) continuarRodando = false;
                break;

            case ESTADO_FIGURINHAS:
                float alturaVisivel = 475.0f;
                float alturaTotal = total * 30.0f;
                float maxScroll = (alturaTotal > alturaVisivel) ? -(alturaTotal - alturaVisivel) : 0;

                // --- LÓGICA DE BUSCA ---
                int key = GetCharPressed();
                while (key > 0) {
                    if (key >= 32 && key <= 125 && strlen(busca) < 49) {
                        int len = strlen(busca);
                        busca[len] = (char)key; busca[len+1] = '\0';
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && strlen(busca) > 0) busca[strlen(busca)-1] = '\0';

                // Se a busca mudou, pesquisa e "PULA" para o resultado
                if (strcmp(busca, buscaAntiga) != 0) {
                    resultadoBusca = pesquisarFigurinha(meuAlbum, total, busca);
                    if (resultadoBusca != -1 && strlen(busca) > 0) {
                        // Calcula o scroll para centralizar a figurinha achada no topo
                        scrollOffset = -(resultadoBusca * 30);
                    }
                    strcpy(buscaAntiga, busca);
                }

                // --- LÓGICA DE INTERAÇÃO COM A BARRA DE SCROLL ---
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, areaScroll)) {
                    arrastandoScroll = true;
                }
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) arrastandoScroll = false;

                if (arrastandoScroll) {
                    float relY = (mouse.y - 105) / 475.0f; // 0.0 a 1.0
                    if (relY < 0) relY = 0; if (relY > 1) relY = 1;
                    scrollOffset = relY * maxScroll;
                } else {
                    scrollOffset += GetMouseWheelMove() * 35;
                }

                // Trava o scroll nos limites
                if (scrollOffset > 0) scrollOffset = 0;
                if (scrollOffset < maxScroll) scrollOffset = maxScroll;

                // Botões e Clique na Lista (Mesma lógica anterior)
                if (CheckCollisionPointRec(mouse, btnNova) && IsMouseButtonReleased(0)) {
                    indiceEdicao = -1; estado = ESTADO_CADASTRO;
                }
                if (IsMouseButtonPressed(0)) {
                    for (int i = 0; i < total; i++) {
                        int posY = 120 + (i * 30) + (int)scrollOffset;
                        if (posY > 105 && posY < 580) {
                            if (CheckCollisionPointRec(mouse, (Rectangle){750,(float)posY,25,25})) {
                                meuAlbum = removerFigurinha(meuAlbum, &total, i); break;
                            } else if (CheckCollisionPointRec(mouse, (Rectangle){20,(float)posY,700,25})) {
                                indiceEdicao = i; estado = ESTADO_CADASTRO; break;
                            }
                        }
                    }
                }
                if (IsKeyPressed(KEY_ESCAPE)) { estado = ESTADO_MENU; scrollOffset = 0; }
                break;

            case ESTADO_CADASTRO:
                UpdateDrawCadastro(&meuAlbum, &total, (int*)&estado, indiceEdicao);
                break;

            case ESTADO_JOGAR:
                UpdateMap();
                if (IsKeyPressed(KEY_ESCAPE)) estado = ESTADO_MENU;
                break;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);

            if (estado == ESTADO_MENU) {
                DrawText("FIGURINHA HUNTERS", 240, 80, 30, DARKBLUE);
                DrawRectangle(300, 200, 200, 50, BLUE); DrawText("JOGAR", 370, 215, 20, WHITE);
                DrawRectangle(300, 270, 200, 50, BLUE); DrawText("FIGURINHAS", 345, 285, 20, WHITE);
                DrawRectangle(300, 340, 200, 50, RED);  DrawText("SAIR", 375, 355, 20, WHITE);
            } 
            else if (estado == ESTADO_FIGURINHAS) {
                BeginScissorMode(0, 105, 800, 475);
                    for (int i = 0; i < total; i++) {
                        int posY = 120 + (i * 30) + (int)scrollOffset;
                        if (posY > 50 && posY < 650) {
                            Color cor = (i == resultadoBusca) ? GREEN : BLACK;
                            DrawRectangle(750, posY, 25, 25, MAROON); DrawText("X", 757, posY + 3, 18, WHITE);
                            DrawText(TextFormat("%d. %s [%s]", i+1, meuAlbum[i].titulo, meuAlbum[i].codigo), 20, posY, 20, cor);
                        }
                    }
                EndScissorMode();

                // DESENHO DA BARRA DE ROLAGEM
                float alturaVisivel = 475.0f;
                float alturaTotal = total * 30.0f;
                DrawRectangleRec(areaScroll, LIGHTGRAY); // Fundo
                if (alturaTotal > alturaVisivel) {
                    float tamBarra = (alturaVisivel / alturaTotal) * alturaVisivel;
                    float maxScroll = -(alturaTotal - alturaVisivel);
                    float posBarra = 105 + (scrollOffset / maxScroll) * (alturaVisivel - tamBarra);
                    DrawRectangle(785, (int)posBarra, 15, (int)tamBarra, arrastandoScroll ? DARKBLUE : DARKGRAY);
                }

                // Cabeçalho
                DrawRectangle(0, 0, 800, 105, LIGHTGRAY);
                DrawText(TextFormat("Busca: %s", busca), 20, 60, 20, BLUE);
                DrawRectangleRec(btnNova, GREEN); DrawText("NOVA (+)", 665, 40, 18, WHITE);
                DrawRectangleRec(btnReset, BLACK); DrawText("RESET (!)", 515, 40, 18, WHITE);
            }
            else if (estado == ESTADO_JOGAR) {
                DrawMap();
                DrawText("MAPA DE TESTE - ESC para Voltar", 20, 20, 20, DARKGRAY);
            }
        EndDrawing();
    }

    salvarDadosBinario(meuAlbum, total);
    if (meuAlbum) free(meuAlbum);
    CloseWindow();
    return 0;
}