#include "raylib.h"
#include "map.h"
#include "album.h"
#include <string.h>
#include <stdlib.h>

Texture2D texMapa;
Texture2D texPacote;
Texture2D texFundoCarta;
extern Texture2D texFigComum;
extern Texture2D texFigRara;

// --- VARIÁVEIS DO JOGO E MAPA ---
int energia;
int diaAtual;
int dinheiro;
int pacotesFechados;
int pacotesAbertos;
bool telaGacha;
bool telaMercadoNegro;
int sorteadas[7] = {0};
static int numeroPedido = 0; 
static int figurinhaGanhaMercadoNegro = -1;
static bool mostrarPremioMercadoNegro = false;
static bool telaAjuda = false;


// Retângulos principais do mapa
Rectangle hitLoja;
Rectangle hitBanca;
Rectangle hitMercadoNegro;
Rectangle btnAbrir;

// --- VARIÁVEIS DOS MINIGAMES ---
bool telaTrabalho1 = false; // Restaurante
bool telaTrabalho2 = false; // Chuva de moedas
float tempoTrabalho = 0.0f;
int moedasGanhas = 0;

// Variáveis do Restaurante
int mesaValores[3];
int mesaCerta = 0;

// Variáveis dos Quadrados (moedas)
int gridTrabalho[9] = {0};
float tempoPiscar = 0.0f;

// Hitbox trabalho 2
Rectangle hitU_Esq;
Rectangle hitU_Top;
Rectangle hitU_Dir;

// Hitbox botão de ajuda
Rectangle hitAjuda;

void InitMap(void) {
    texMapa = LoadTexture("assets/mapaFH.png");
    texPacote = LoadTexture("assets/pacote.png");
    texFundoCarta = LoadTexture("assets/fundo_carta.png");
    
    // Configuração do Sistema de Energia
    energia = 5;
    diaAtual = 1;
    
    dinheiro = 0;
    pacotesFechados = 0;
    pacotesAbertos = 0;
    telaGacha = false;
    telaMercadoNegro = false;
    telaTrabalho1 = false;
    telaTrabalho2 = false;
    
    // Hitboxes interativas
    hitLoja = (Rectangle){ 543, 400, 88, 59 };
    hitBanca = (Rectangle){ 543, 467, 88, 83 };
    hitMercadoNegro = (Rectangle){ 630, 0, 145, 65 };
    
    // Hitboxes Trabalho 2
    hitU_Esq = (Rectangle){ 190, 174, 82, 112 };
    hitU_Top = (Rectangle){ 190, 118, 250, 58 };
    hitU_Dir = (Rectangle){ 350, 174, 90, 112 };
    
    btnAbrir = (Rectangle){ 634, 555, 130, 40 }; 
    hitAjuda = (Rectangle){ 40, 15, 25, 25 }; 
}

void UpdateMap(Figurinha *album, int total) {
    Vector2 mouse = GetMousePosition();

    if (telaGacha) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) telaGacha = false;
        return; 
    }

    if (telaMercadoNegro || telaTrabalho1 || telaTrabalho2) {
        return;
    }

        if (CheckCollisionPointRec(mouse, hitAjuda) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            telaAjuda = !telaAjuda;
            return;
        }

    // INTERAÇÃO COM O MAPA PRINCIPAL
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        
        if (CheckCollisionPointRec(mouse, hitLoja)) {
            telaTrabalho1 = true;
            tempoTrabalho = 10.0f;
            moedasGanhas = 0;
            
            // Gera 3 números de mesas distintos e aleatórios
            mesaValores[0] = GetRandomValue(1, 30);
            mesaValores[1] = GetRandomValue(31, 60);
            mesaValores[2] = GetRandomValue(61, 99);
            
            // Sorteia qual dessas 3 mesas será o pedido atual solicitado por texto
            int idxCerto = GetRandomValue(0, 2);
            numeroPedido = mesaValores[idxCerto];
        }
        
        else if (CheckCollisionPointRec(mouse, hitU_Esq) || 
                 CheckCollisionPointRec(mouse, hitU_Top) || 
                 CheckCollisionPointRec(mouse, hitU_Dir)) {
            telaTrabalho2 = true;
            tempoTrabalho = 10.0f;
            moedasGanhas = 0;
            for(int i=0; i<9; i++) gridTrabalho[i] = 0;
        }
        
        else if (CheckCollisionPointRec(mouse, hitMercadoNegro)) {
            telaMercadoNegro = true;
        }

        // Lógica da Banca (Compra de pacotes)
        else if (CheckCollisionPointRec(mouse, hitBanca)) { // Removida a trava de turno
            if (dinheiro >= 10) {
                dinheiro -= 10;
                pacotesFechados++;
                energia--;
            }
        }
        
        // Lógica de Abrir o Pacote
        else if (CheckCollisionPointRec(mouse, btnAbrir) && pacotesFechados > 0) {
            pacotesFechados--;
            pacotesAbertos++;
            telaGacha = true;
            
            for (int i = 0; i < 7; i++) {
                int indexSorteado = GetRandomValue(0, total - 1);
                sorteadas[i] = indexSorteado;
                album[indexSorteado].quantidade++; 
            }
            salvarDadosBinario(album, total);
        }

        // 6. Fim do Dia: Se a energia zerar, passa para o próximo dia
        if (energia <= 0) {
            diaAtual++;
            energia = 5; 
            // Opcional: salvar o jogo ao dormir
            // salvarDadosBinario(album, total); 
        }
    }
}

void DrawMap(Figurinha *meuAlbum, int total) {
    // Fundo
    DrawTexture(texMapa, 0, 0, WHITE);

    // Caixas de debug de hitbox (deixar como comentário após ajustar)
    // DrawRectangleRec(hitLoja, Fade(RED, 0.3f));
    // DrawRectangleRec(hitBanca, Fade(GREEN, 0.3f));
    // DrawRectangleRec(hitMercadoNegro, Fade(PURPLE, 0.3f));
    
    // DrawRectangleRec(hitU_Esq, Fade(BLUE, 0.3f));
    // DrawRectangleRec(hitU_Top, Fade(BLUE, 0.3f));
    // DrawRectangleRec(hitU_Dir, Fade(BLUE, 0.3f));
    Vector2 mouse = GetMousePosition();
    // DrawText(TextFormat("Mouse X: %0.f | Y: %0.f", mouse.x, mouse.y), 20, 250, 16, BLUE);

    DrawRectangleRec(hitAjuda, Fade(RED,0.5));
    
    // HUD
    DrawRectangle(136, 5, 450, 95, Fade(DARKGRAY, 0.9f));
    DrawRectangleLines(136, 5, 450, 95, BLUE);

    DrawText(TextFormat("Dia: %d", diaAtual), 156, 15, 20, WHITE);
    Color corEnergia = (energia > 1) ? BLUE : RED;
    DrawText(TextFormat("Energia: %d / 5", energia), 156, 45, 20, corEnergia);
    
    DrawText(TextFormat("Dinheiro: R$ %d", dinheiro), 156, 75, 20, GREEN);
    
    DrawText(TextFormat("Pacotes Inventário: %d", pacotesFechados), 335, 15, 20, WHITE);
    
    DrawText(TextFormat("Pacotes Abertos: %d", pacotesAbertos), 335, 45, 20, WHITE);
    
    DrawRectangle(36, 540, 110, 50, Fade(DARKGRAY, 0.9f));
    DrawRectangleLines(36, 540, 110, 50, RED);

    DrawText("V para album.", 41, 565, 12, WHITE);
    DrawText("ESC para Menu", 41, 550, 12, WHITE);

    DrawText("?", 45, 19, 15, WHITE);

    if (pacotesFechados > 0) {
        DrawRectangleRec(btnAbrir, DARKPURPLE);
        DrawText("ABRIR PACOTE", btnAbrir.x + 10, btnAbrir.y + 12, 15, WHITE);
    }

    // ===================
    // TELA GACHA
    // ===================
    if (telaGacha) {
        DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.9f)); 
        DrawText("PACOTE ABERTO!", 280, 40, 30, WHITE); 
        DrawTexture(texPacote, 350, 80, WHITE);

        int larguraCarta = 90;
        int alturaCarta = 110;
        int espacamento = 15;
        int margemEsquerda = 40;
        int posicaoY = 240; 

        for (int i = 0; i < 7; i++) {
            int posX = margemEsquerda + (i * (larguraCarta + espacamento));
            int indiceFigurinha = sorteadas[i];
            Figurinha f = meuAlbum[indiceFigurinha];

            Texture2D texturaGacha = texFigComum;
            if (strstr(f.tipo, "Especial") != NULL || strstr(f.tipo, "ESPECIAL") != NULL || strstr(f.tipo, "especial") != NULL) {
                texturaGacha = texFigRara;
            }

            DrawTexturePro(texturaGacha, 
                (Rectangle){ 0, 0, (float)texturaGacha.width, (float)texturaGacha.height }, 
                (Rectangle){ (float)posX, (float)posicaoY, (float)larguraCarta, (float)alturaCarta }, 
                (Vector2){ 0, 0 }, 0.0f, WHITE);
            
            DrawText(TextFormat("#%s", f.codigo), posX + 8, posicaoY + 8, 12, DARKGRAY);
            DrawText(TextFormat("%s", f.titulo), posX, posicaoY + alturaCarta + 10, 10, WHITE); 
            DrawText(TextFormat("%s", f.tipo), posX, posicaoY + alturaCarta + 22, 9, GOLD);
        }

        DrawText("CLIQUE PARA CONTINUAR", 270, 520, 20, YELLOW); 
    }

    // ===========================
    // SISTEMA DE AJUDA
    // ===========================
    if (telaAjuda) {
        DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.4f));

        // Dica da Chuva de Moedas
        DrawRectangle(445, 120, 285, 60, Fade(DARKGRAY, 0.9f));
        DrawRectangleLines(445, 120, 285, 60, LIME);
        DrawText("MINIGAME: ESTÁ CHOVENDO MOEDAS!", 455, 130, 14, LIME);
        DrawText("Clique nos blocos verdes \ne consiga dinheiro!", 455, 150, 12, WHITE);

        // Dica da Banca
        DrawRectangle(347, 470, 180, 60, Fade(DARKGRAY, 0.9f));
        DrawRectangleLines(347, 470, 180, 60, BLUE);
        DrawText("BANCA", 357, 490, 14, BLUE);
        DrawText("Compre pacotes (R$10)", 357, 510, 12, WHITE);

        // Dica do Restaurante
        DrawRectangle(327, 400, 200, 60, Fade(DARKGRAY, 0.9f));
        DrawRectangleLines(327, 400, 200, 60, ORANGE);
        DrawText("MINIGAME: RESTAURANTE", 337, 410, 14, ORANGE);
        DrawText("Encontre a mesa certa!", 337, 430, 12, WHITE);
        
        // Aviso no meio da tela para o jogador saber como sair
        DrawText("CLIQUE NO '?' NOVAMENTE PARA FECHAR A AJUDA", 180, 560, 16, YELLOW);
    }

    // =======================
    // MINIGAME 1: RESTAURANTE
    // =======================
    if (telaTrabalho1) {
        DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.9f));
        DrawText("RESTAURANTE RUSH!", 250, 40, 30, ORANGE);
        
        // Texto dinâmico que indica qual mesa o jogador deve clicar
        DrawText(TextFormat("LEVE O PEDIDO PARA A MESA: %d", numeroPedido), 190, 95, 22, YELLOW);
        
        DrawText(TextFormat("TEMPO: %.1f", tempoTrabalho), 340, 135, 20, tempoTrabalho > 3.0f ? WHITE : RED);
        DrawText(TextFormat("Dinheiro na Bandeja: R$ %d", moedasGanhas), 280, 165, 20, GOLD);

        tempoTrabalho -= GetFrameTime(); 

        if (tempoTrabalho > 0) {
            for (int i = 0; i < 3; i++) {
                Rectangle btnMesa = { 150 + (i * 180), 250, 120, 120 };
                bool hover = CheckCollisionPointRec(GetMousePosition(), btnMesa);
                
                DrawRectangleRec(btnMesa, hover ? LIGHTGRAY : RAYWHITE);
                DrawRectangleLinesEx(btnMesa, 4, DARKGRAY);
                DrawText(TextFormat("%d", mesaValores[i]), btnMesa.x + 40, btnMesa.y + 45, 30, BLACK);

                if (hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    // Valida se clicou na mesa com o número igual ao exibido no texto
                    if (mesaValores[i] == numeroPedido) {
                        moedasGanhas += 2; 
                    }
                    // Sorteia instantaneamente a próxima rodada
                    mesaValores[0] = GetRandomValue(1, 30);
                    mesaValores[1] = GetRandomValue(31, 60);
                    mesaValores[2] = GetRandomValue(61, 99);
                    int idxCerto = GetRandomValue(0, 2);
                    numeroPedido = mesaValores[idxCerto];
                }
            }
        } else {
            DrawText("FIM DE EXPEDIENTE!", 260, 250, 30, WHITE);
            DrawText(TextFormat("+ R$ %d", moedasGanhas), 340, 300, 20, GREEN);
            DrawText("Clique para Voltar", 310, 450, 20, GRAY);
            
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                dinheiro += moedasGanhas; 
                energia--;
                telaTrabalho1 = false;
            }
        }
    }

    // ==========================================
    // MINIGAME 2: CHUVA DE MOEDAS
    // ==========================================
    if (telaTrabalho2) {
        DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.9f));
        DrawText("PEGUE AS MOEDAS DO CHÃO!", 230, 50, 30, GREEN);
        DrawText(TextFormat("TEMPO: %.1f", tempoTrabalho), 340, 100, 20, tempoTrabalho > 3.0f ? WHITE : RED);
        DrawText(TextFormat("Moedas coletadas: R$ %d", moedasGanhas), 280, 140, 20, GOLD);

        tempoTrabalho -= GetFrameTime();
        
        if (tempoTrabalho > 0) {
            
            // Lógica do minigame
            tempoPiscar += GetFrameTime();
            if (tempoPiscar > 0.3f) {
                tempoPiscar = 0.0f;
                for (int k = 0; k < 2; k++) {
                    int alvo = GetRandomValue(0, 8);
                    // 75% de chance de virar Verde (1) contra 25% de Vermelho (2)
                    gridTrabalho[alvo] = (GetRandomValue(1, 100) <= 75) ? 1 : 2; 
                }
            }

            for (int i = 0; i < 9; i++) {
                int col = i % 3;
                int linha = i / 3;
                Rectangle btnCaixa = { 250 + (col * 110), 200 + (linha * 110), 90, 90 };
                
                Color corCaixa = GRAY;
                if (gridTrabalho[i] == 1) corCaixa = LIME;
                if (gridTrabalho[i] == 2) corCaixa = RED;

                DrawRectangleRec(btnCaixa, corCaixa);

                if (CheckCollisionPointRec(GetMousePosition(), btnCaixa) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (gridTrabalho[i] == 1) { 
                        moedasGanhas += 1;
                        gridTrabalho[i] = 0; 
                    } else if (gridTrabalho[i] == 2) {
                        gridTrabalho[i] = 0; 
                    }
                }
            }
        } else {
            DrawText("FIM DE EXPEDIENTE!", 260, 250, 30, WHITE);
            DrawText(TextFormat("+ R$ %d", moedasGanhas), 340, 300, 20, GREEN);
            DrawText("Clique para Voltar", 310, 450, 20, GRAY);
            
if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                dinheiro += moedasGanhas; 
                energia--;
                telaTrabalho2 = false;
            }
        }
    }

    // ==============
    // MERCADO NEGRO 
    // ==============
    if (telaMercadoNegro) {
        Vector2 mousePos = GetMousePosition();
        DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.8f));
        
        Rectangle painel = { 200, 150, 400, 320 };
        DrawRectangleRec(painel, DARKGRAY);
        DrawRectangleLinesEx(painel, 3, RED); 
        
        DrawText("MERCADO NEGRO", 310, 170, 22, RED);
        DrawText("Troque 5 repetidas por 1 NOVA garantida!", 225, 205, 15, LIGHTGRAY);
        DrawText("Custo: 20 moedas", 330, 225, 14, GOLD);

        Rectangle btnComum    = { 220, 260, 360, 40 };
        Rectangle btnEspecial = { 220, 315, 360, 40 };
        Rectangle btnVoltar   = { 350, 390, 100, 35 };

        bool hoverComum = CheckCollisionPointRec(mousePos, btnComum);
        DrawRectangleRec(btnComum, hoverComum ? RAYWHITE : GRAY);
        DrawText("Usar 5 Comuns (10% chance Especial)", btnComum.x + 25, btnComum.y + 12, 15, BLACK);

        bool hoverEspecial = CheckCollisionPointRec(mousePos, btnEspecial);
        DrawRectangleRec(btnEspecial, hoverEspecial ? GOLD : (Color){ 150, 110, 0, 255 });
        DrawText("Usar 5 Especiais (Garante Especial)", btnEspecial.x + 25, btnEspecial.y + 12, 15, BLACK);

        bool hoverVoltar = CheckCollisionPointRec(mousePos, btnVoltar);
        DrawRectangleRec(btnVoltar, hoverVoltar ? RED : MAROON);
        DrawText("VOLTAR", btnVoltar.x + 22, btnVoltar.y + 10, 14, WHITE);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (!mostrarPremioMercadoNegro) {
                if (hoverComum) {
                    int res = ExecutarTrocaMercadoNegro(meuAlbum, total, &dinheiro, 1);
                    if (res != -1) {
                        figurinhaGanhaMercadoNegro = res;
                        mostrarPremioMercadoNegro = true;
                    }
                }
                if (hoverEspecial) {
                    int res = ExecutarTrocaMercadoNegro(meuAlbum, total, &dinheiro, 2);
                    if (res != -1) {
                        figurinhaGanhaMercadoNegro = res;
                        mostrarPremioMercadoNegro = true;
                    }
                }
                if (hoverVoltar) telaMercadoNegro = false; 
            } else {
                mostrarPremioMercadoNegro = false;
                figurinhaGanhaMercadoNegro = -1;
            }
        }

        if (mostrarPremioMercadoNegro && figurinhaGanhaMercadoNegro != -1) {
            DrawRectangle(210, 160, 380, 300, BLACK);
            DrawRectangleLines(210, 160, 380, 300, GOLD);
            
            DrawText("NEGOCIAÇÃO CONCLUÍDA!", 285, 180, 16, LIME);
            DrawText("Item extraído com sucesso:", 295, 205, 14, LIGHTGRAY);
            
            Figurinha f = meuAlbum[figurinhaGanhaMercadoNegro];
            int cardX = 355;
            int cardY = 235;
            
            Texture2D texturaPremio = texFigComum;
            if (strstr(f.tipo, "Especial") != NULL || strstr(f.tipo, "ESPECIAL") != NULL || strstr(f.tipo, "especial") != NULL) {
                texturaPremio = texFigRara;
            }

            DrawTexturePro(texturaPremio, 
                (Rectangle){ 0, 0, (float)texturaPremio.width, (float)texturaPremio.height }, 
                (Rectangle){ (float)cardX, (float)cardY, 90, 110 }, 
                (Vector2){ 0, 0 }, 0.0f, WHITE);
                
            DrawText(TextFormat("#%s", f.codigo), cardX + 8, cardY + 8, 12, DARKGRAY);
            DrawText(TextFormat("%s", f.titulo), 230, cardY + 120, 13, WHITE);
            DrawText(TextFormat("[%s]", f.tipo), 340, cardY + 140, 12, GOLD);
            
            DrawText("Clique em qualquer lugar para fechar", 270, 435, 12, GRAY);
        }
    }
}