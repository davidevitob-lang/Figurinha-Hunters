#include "raylib.h"
#include "album.h"
#include "map.h"
#include "cadastro.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

Texture2D texMenu;
Texture2D texLogo;
Texture2D texBotaoJogar;
Texture2D texBotaoFig; 
Texture2D texBotaoSair;
Texture2D texAlbumBase;
Texture2D texFigComum;
Texture2D texFigRara;


typedef enum { ESTADO_INTRO = 0, ESTADO_MENU, ESTADO_JOGAR, ESTADO_FIGURINHAS, ESTADO_CADASTRO, ESTADO_ALBUM_VISUAL } EstadoJogo;

int main(void) {
    int total = 0;
    Figurinha *meuAlbum = carregarDadosIniciais(&total);
    EstadoJogo estado = ESTADO_INTRO;
    int indiceEdicao = -1; 
    bool continuarRodando = true;
    int paginaAtual = 0;
    char filtroVisual[20] = "";
    bool buscaFocada = false;

    // Variáveis de busca e scroll
    char busca[50] = "\0";
    char buscaAntiga[50] = "\0";
    float scrollOffset = 0;
    int resultadoBusca = -1;
    bool arrastandoScroll = false;

    InitWindow(800, 600, "Figurinha Hunters 2026 - Pro Soccer Edition");
    InitAudioDevice(); 
    Sound somIntro = LoadSound("assets/introFH.wav");
    Image iconeJanela = LoadImage("assets/capa.png");
    SetWindowIcon(iconeJanela);
    UnloadImage(iconeJanela);
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    InitMap();
    
    texMenu = LoadTexture("assets/menu.png");
    texBotaoJogar = LoadTexture("assets/BotaoJogar.png");
    texBotaoFig = LoadTexture("assets/BotaoFig.png");
    texBotaoSair = LoadTexture("assets/BotaoSair.png");

    BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Carregando intro...", 300, 280, 20, GRAY);
    EndDrawing();

    int animFrames = 0;
    Image imageIntro = LoadImageAnim("assets/introFH.gif", &animFrames);
    Texture2D texIntro = LoadTextureFromImage(imageIntro);
    int frameAtual = 0;
    float tempoFrame = 0.0f;
    float alphaFade = 0.0f;
    bool iniciandoFade = false;

    texAlbumBase = LoadTexture("assets/album.png");
    texFigComum  = LoadTexture("assets/FigComum.png");
    texFigRara   = LoadTexture("assets/FigRara.png");
    
    Rectangle btnNova = { 630, 30, 140, 40 };
    Rectangle btnReset = { 480, 30, 140, 40 };
    Rectangle areaScroll = { 785, 105, 15, 475 }; 

    PlaySound(somIntro);

    while (continuarRodando && !WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();
        if (IsKeyPressed(KEY_F12)) {
            meuAlbum = resetarAlbum(meuAlbum, &total);
            // Se estiver na lista, reseta o scroll e a busca também
            scrollOffset = 0;   
            busca[0] = '\0';    
        }

        switch (estado) {

            case ESTADO_INTRO: {
                if (!iniciandoFade) {
                    tempoFrame += GetFrameTime();
                    if (tempoFrame >= (1.0f / 20.0f)) {
                        tempoFrame = 0.0f;
                        frameAtual++;
                        
                        if (frameAtual >= animFrames) {
                            frameAtual = animFrames - 1; 
                            iniciandoFade = true; 
                        } else {
                            int frameOffset = imageIntro.width * imageIntro.height * 4 * frameAtual;
                            UpdateTexture(texIntro, ((unsigned char *)imageIntro.data) + frameOffset);
                        }
                    }
                    if (IsKeyPressed(KEY_SPACE) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) iniciandoFade = true;
                } else {
                    alphaFade += 1.5f * GetFrameTime(); 
                    
                    SetSoundVolume(somIntro, 1.0f - alphaFade);
                    
                    if (alphaFade >= 1.0f) {
                        alphaFade = 1.0f;
                        StopSound(somIntro);
                        estado = ESTADO_MENU; 
                    }
                }
            } break;

            case ESTADO_MENU:
                if (alphaFade > 0.0f) {
                    alphaFade -= 1.5f * GetFrameTime();
                    if (alphaFade < 0.0f) alphaFade = 0.0f;
                }    
                // caixas de colisão
                Rectangle hitJogar = { 240, 310, 320, 80 };
                Rectangle hitFig   = { 240, 400, 320, 80 };
                Rectangle hitSair  = { 240, 490, 320, 80 };

                if (CheckCollisionPointRec(mouse, hitJogar) && IsMouseButtonReleased(0)) estado = ESTADO_JOGAR;
                if (CheckCollisionPointRec(mouse, hitFig) && IsMouseButtonReleased(0)) estado = ESTADO_FIGURINHAS;
                if (CheckCollisionPointRec(mouse, hitSair) && IsMouseButtonReleased(0)) continuarRodando = false;
                if (IsKeyPressed(KEY_N)) { estado = ESTADO_ALBUM_VISUAL; paginaAtual = 0; }
                break;

            case ESTADO_FIGURINHAS:
                float alturaVisivel = 475.0f;
                float alturaTotal = total * 30.0f;
                float maxScroll = (alturaTotal > alturaVisivel) ? -(alturaTotal - alturaVisivel) : 0;

                // lógica de busca
                int key = GetCharPressed();
                while (key > 0) {
                    if (key >= 32 && key <= 125 && strlen(busca) < 49) {
                        int len = strlen(busca);
                        busca[len] = (char)key; busca[len+1] = '\0';
                    }
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && strlen(busca) > 0) busca[strlen(busca)-1] = '\0';

                // Se a busca mudou, pesquisa e pula para o resultado
                if (strcmp(busca, buscaAntiga) != 0) {
                    resultadoBusca = pesquisarFigurinha(meuAlbum, total, busca);
                    if (resultadoBusca != -1 && strlen(busca) > 0) {
                        // Calcula o scroll para centralizar a figurinha achada no topo
                        scrollOffset = -(resultadoBusca * 30);
                    }
                    strcpy(buscaAntiga, busca);
                }

                // lógica da barra de scroll
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, areaScroll)) {
                    arrastandoScroll = true;
                }
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) arrastandoScroll = false;

                if (arrastandoScroll) {
                    float relY = (mouse.y - 105) / 475.0f; // 0.0 a 1.0
                    if (relY < 0) relY = 0; 
                    if (relY > 1) relY = 1;
                    scrollOffset = relY * maxScroll;
                } else {
                    scrollOffset += GetMouseWheelMove() * 35;
                }

                // Trava o scroll nos limites
                if (scrollOffset > 0) scrollOffset = 0;
                if (scrollOffset < maxScroll) scrollOffset = maxScroll;

                // lógica para os botões da lista
                
                // Botão Nova Figurinha
                if (CheckCollisionPointRec(mouse, btnNova) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    indiceEdicao = -1; 
                    estado = ESTADO_CADASTRO;
                }
                else if (CheckCollisionPointRec(mouse, btnReset) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    meuAlbum = resetarAlbum(meuAlbum, &total);
                    scrollOffset = 0;   
                    busca[0] = '\0';    
                    buscaAntiga[0] = '\0';
                    resultadoBusca = -1;
                }
                // Clique nas linhas da Lista
                else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    for (int i = 0; i < total; i++) {
                        int posY = 120 + (i * 30) + (int)scrollOffset;
                        // Garante que só clica no que está visível na tela
                        if (posY > 105 && posY < 580) {
                            // Clique no botão de lixeira (remover)
                            if (CheckCollisionPointRec(mouse, (Rectangle){750,(float)posY,25,25})) {
                                meuAlbum = removerFigurinha(meuAlbum, &total, i); 
                                break;
                            } 
                            // Clique na linha para editar (Evita colisão se o clique for muito no canto perto do scroll)
                            else if (CheckCollisionPointRec(mouse, (Rectangle){20,(float)posY,700,25})) {
                                indiceEdicao = i; 
                                estado = ESTADO_CADASTRO;
                                break;
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
                UpdateMap(meuAlbum, total);
                if (IsKeyPressed(KEY_ESCAPE)) estado = ESTADO_MENU;
                if (IsKeyPressed(KEY_V)) { estado = ESTADO_ALBUM_VISUAL;paginaAtual = 0;filtroVisual[0] = '\0';while(GetCharPressed() > 0) {}}
                
                break;

                case ESTADO_ALBUM_VISUAL:
                Vector2 mouseVis = GetMousePosition();
                Rectangle hitBarra = { 20, 20, 300, 40 }; // A caixa de busca

                // Liga ou desliga o foco dependendo de onde o jogador clicou
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(mouseVis, hitBarra)) buscaFocada = true;
                    else buscaFocada = false;
                }

                if (buscaFocada) {
                    int keyVis = GetCharPressed();
                    while (keyVis > 0) {
                        if (keyVis >= 32 && keyVis <= 125 && strlen(filtroVisual) < 19) {
                            int len = strlen(filtroVisual);
                            filtroVisual[len] = (char)toupper(keyVis);
                            filtroVisual[len+1] = '\0';
                            paginaAtual = 0;
                        }
                        keyVis = GetCharPressed();
                    }
                    if (IsKeyPressed(KEY_BACKSPACE) && strlen(filtroVisual) > 0) {
                        filtroVisual[strlen(filtroVisual)-1] = '\0';
                        paginaAtual = 0;
                    }
                } else {
                    // Se a barra não estiver clicada, A e D para navegação de páginas funciona
                    while(GetCharPressed() > 0) {} 
                }

                if (IsKeyPressed(KEY_ESCAPE)) estado = ESTADO_JOGAR;
                break;
            }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            // DrawText(TextFormat("Mouse X: %0.f | Y: %0.f", mouse.x, mouse.y), 20, 560, 16, BLUE);

            if (estado == ESTADO_INTRO) {
                // Desenha a animação centralizada na tela
                DrawTexture(texIntro, 400 - texIntro.width/2, 300 - texIntro.height/2, WHITE);
                
                // Desenha o efeito de escurecimento por cima
                if (iniciandoFade) {
                    DrawRectangle(0, 0, 800, 600, Fade(BLACK, alphaFade));
                }
            }
            else if (estado == ESTADO_MENU) {
                DrawTexturePro(texMenu, 
                    (Rectangle){ 0, 0, (float)texMenu.width, (float)texMenu.height }, 
                    (Rectangle){ 0, 0, 800, 600 }, 
                    (Vector2){ 0, 0 }, 0.0f, WHITE);
                
                DrawTexturePro(texBotaoJogar, 
                    (Rectangle){ 0, 0, (float)texBotaoJogar.width, (float)texBotaoJogar.height }, 
                    (Rectangle){ 240, 310, 320, 80 }, 
                    (Vector2){ 0, 0 }, 0.0f, WHITE);

                DrawTexturePro(texBotaoFig, 
                    (Rectangle){ 0, 0, (float)texBotaoFig.width, (float)texBotaoFig.height }, 
                    (Rectangle){ 240, 400, 320, 80 }, 
                    (Vector2){ 0, 0 }, 0.0f, WHITE);

                DrawTexturePro(texBotaoSair, 
                    (Rectangle){ 0, 0, (float)texBotaoSair.width, (float)texBotaoSair.height }, 
                    (Rectangle){ 240, 490, 320, 80 }, 
                    (Vector2){ 0, 0 }, 0.0f, WHITE);

                // Desenha o efeito de clareamento por cima do menu
                if (alphaFade > 0.0f) {
                    DrawRectangle(0, 0, 800, 600, Fade(BLACK, alphaFade));
                }
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

                // barra de rolagem
                float alturaVisivel = 475.0f;
                float alturaTotal = total * 30.0f;
                DrawRectangleRec(areaScroll, LIGHTGRAY);
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
                DrawMap(meuAlbum, total);
                //DrawText("MAPA DE TESTE - ESC para Voltar", 20, 20, 20, DARKGRAY);
            }

            else if (estado == ESTADO_ALBUM_VISUAL) {
            DesenharAlbumGrade(meuAlbum, total, &paginaAtual, filtroVisual, buscaFocada);
            }

        EndDrawing();
    }

    UnloadTexture(texMenu);
    UnloadTexture(texLogo);
    UnloadTexture(texBotaoJogar);
    UnloadTexture(texBotaoFig);
    UnloadTexture(texBotaoSair);
    UnloadTexture(texIntro);
    UnloadImage(imageIntro);
    UnloadTexture(texFigComum);
    UnloadTexture(texFigRara);
    UnloadTexture(texAlbumBase);
    UnloadSound(somIntro);
    CloseAudioDevice();

    salvarDadosBinario(meuAlbum, total);
    if (meuAlbum) free(meuAlbum);
    CloseWindow();
    return 0;
}