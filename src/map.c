#include "raylib.h"
#include "map.h"
#include "album.h"

// Variáveis do Mapa
static int turnoAtual = 1;
static int dinheiro = 0;
static int pacotesFechados = 0;
static int pacotesAbertos = 0;

static Rectangle btnTrabalho;
static Rectangle btnBanca;
static Rectangle btnAbrir;

// Variáveis do Sorteio
static bool telaGacha = false;
static int sorteadas[5] = {0};

// Variáveis do obstáculo
static bool telaObstaculo = false;
static float timerObstaculo = 0.0f;
static int mesaAlvo = 0;
static int mesaEsq = 0;
static int mesaDir = 0;
static Rectangle btnMesaEsq;
static Rectangle btnMesaDir;

void InitMap(void) {
    turnoAtual = 1;
    dinheiro = 0;
    pacotesFechados = 0;
    pacotesAbertos = 0;
    telaGacha = false;
    
    btnTrabalho = (Rectangle){ 100, 350, 150, 150 };
    btnBanca = (Rectangle){ 550, 250, 120, 100 };
    btnAbrir = (Rectangle){ 650, 500, 130, 40 }; // Fica no canto inferior direito
    btnMesaEsq = (Rectangle){ 200, 300, 150, 150 };
    btnMesaDir = (Rectangle){ 450, 300, 150, 150 };
}

void UpdateMap(Figurinha *album, int total) {
    Vector2 mouse = GetMousePosition();

    if (telaGacha) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) telaGacha = false;
        return; 
    }

    // lógica do obstáculo
    if (telaObstaculo) {
        // Reduz o tempo; GetFrameTime calcula o tempo exato desde o último frame
        timerObstaculo -= GetFrameTime();

        // Se o tempo acabar perdeu a chance
        if (timerObstaculo <= 0.0f) {
            telaObstaculo = false;
            turnoAtual++;
            return;
        }

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            // Se escolheu a mesa da Esquerda
            if (CheckCollisionPointRec(mouse, btnMesaEsq)) {
                if (mesaEsq == mesaAlvo) dinheiro += (turnoAtual >= 5) ? 25 : 15; 
                telaObstaculo = false;
                turnoAtual++;
            } 
            // Se escolheu a mesa da Direita
            else if (CheckCollisionPointRec(mouse, btnMesaDir)) {
                if (mesaDir == mesaAlvo) dinheiro += (turnoAtual >= 5) ? 25 : 15; 
                telaObstaculo = false;
                turnoAtual++;
            }
        }
        return; // Pausa o resto do mapa enquanto o evento acontece
    }

    // eventos do mapa
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        
        // Lógica do Trabalho
        if (CheckCollisionPointRec(mouse, btnTrabalho)) {
            int chance = GetRandomValue(1, 100);
            
            // 25% de chance de dar problema
            if (chance <= 25) {
                telaObstaculo = true;
                timerObstaculo = 5.0f; // 5 segundos cravados
                mesaAlvo = GetRandomValue(1, 20); // Mesa procurada de 1 a 20
                
                // Sorteia aleatoriamente em qual lado a mesa certa vai ficar
                if (GetRandomValue(0, 1) == 0) {
                    mesaEsq = mesaAlvo;
                    mesaDir = GetRandomValue(21, 40); // A outra mesa recebe um número falso
                } else {
                    mesaDir = mesaAlvo;
                    mesaEsq = GetRandomValue(21, 40); 
                }
            } 
            else { // 75% de chance de ser um dia normal
                dinheiro += (turnoAtual >= 5) ? 25 : 15; 
                turnoAtual++;
            }
        }
        
        // Lógica da Banca
        else if (CheckCollisionPointRec(mouse, btnBanca) && turnoAtual <= 4) {
            if (dinheiro >= 10) {
                dinheiro -= 10;
                pacotesFechados++;
                turnoAtual++;
            }
        }
        // Lógica de Abrir o Pacote
        else if (CheckCollisionPointRec(mouse, btnAbrir) && pacotesFechados > 0) {
            pacotesFechados--;
            pacotesAbertos++;
            telaGacha = true;
            
            // Sorteia 5 figurinhas
            for (int i = 0; i < 5; i++) {
                int indexSorteado = GetRandomValue(0, total - 1);
                sorteadas[i] = indexSorteado;
                album[indexSorteado].quantidade++; 
            }

            // salva o progresso
            salvarDados(album, total);
        }

        if (turnoAtual > 7) turnoAtual = 1;
    }
}

void DrawMap(Figurinha *meuAlbum) {
    // Desenha Fundo
    if (turnoAtual <= 3) ClearBackground(SKYBLUE);
    else if (turnoAtual == 4) ClearBackground(ORANGE);
    else ClearBackground(DARKBLUE);

    DrawRectangle(0, 450, 800, 150, DARKGREEN);

    // Prédios
    DrawRectangleRec(btnTrabalho, GRAY);
    DrawText("TRABALHO", btnTrabalho.x + 10, btnTrabalho.y + 60, 20, WHITE);

    DrawRectangleRec(btnBanca, RED);
    DrawText("BANCA", btnBanca.x + 20, btnBanca.y + 40, 20, WHITE);
    if (turnoAtual > 4) DrawText("FECHADO", btnBanca.x + 10, btnBanca.y + 65, 20, BLACK);

    // HUD (Dinheiro e Turno)
    DrawText(TextFormat("Turno: %d/7", turnoAtual), 20, 20, 20, WHITE);
    DrawText(TextFormat("Dinheiro: R$ %d", dinheiro), 20, 50, 20, GREEN);
    DrawText("ESC para Menu", 650, 20, 15, LIGHTGRAY);
    DrawText(TextFormat("Aperte V para abrir o album."), 20, 110, 20, WHITE);
    
    // Mostra o inventário de pacotes e o botão se tiver algum fechado
    DrawText(TextFormat("Pacotes Inventário: %d", pacotesFechados), 20, 80, 20, PURPLE);
    DrawText(TextFormat("Pacotes Abertos: %d", pacotesAbertos), 20, 565, 15, WHITE);
    if (pacotesFechados > 0) {
        DrawRectangleRec(btnAbrir, PURPLE);
        DrawText("ABRIR PACOTE", btnAbrir.x + 10, btnAbrir.y + 12, 15, WHITE);
    }

    // tela de abrir pacote de figurinha (Desenhada por cima de tudo)
    if (telaGacha) {
        // Dentro da lógica de desenhar o Gacha (quando telaGacha for true)
        DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.9f)); // Fundo escuro para dar destaque

        DrawText("PACOTE ABERTO!", 280, 100, 30, WHITE); // Título centralizado no topo

        int larguraCarta = 130;
        int alturaCarta = 155;
        int espacamento = 20;
        int margemEsquerda = 35;
        int posicaoY = 220; 

        // desenha as 5 figurinhas
        for (int i = 0; i < 5; i++) {
            int posX = margemEsquerda + (i * (larguraCarta + espacamento));
            
            int indiceFigurinha = sorteadas[i];

            Figurinha f = meuAlbum[indiceFigurinha];
            
            // Desenha o slot da figurinha
            DrawRectangle(posX, posicaoY, larguraCarta, alturaCarta, LIGHTGRAY); 
            DrawText(TextFormat("#%s", f.codigo), posX + 10, posicaoY + 10, 15, GRAY); //mostra o código da figurinha
            DrawText(TextFormat("%s", f.titulo), posX, posicaoY + alturaCarta + 10, 12, WHITE); // Mostra o nome/título da figurinha
            DrawText(TextFormat("Tipo: %s", f.tipo), posX, posicaoY + alturaCarta + 25, 11, GOLD); // Mostra raridade da figurinha
            DrawText(TextFormat("%s", f.secao), posX, posicaoY + alturaCarta + 38, 10, LIGHTGRAY);
        }

        DrawText("CLIQUE PARA CONTINUAR", 270, 500, 20, YELLOW);
    }
        // tela de obstaculo
    if (telaObstaculo) {
        // Escurece o fundo
        DrawRectangle(0, 0, 800, 600, Fade(BLACK, 0.9f)); 
        
        DrawText("IMPREVISTO NO TRABALHO!", 240, 100, 25, RED);
        DrawText(TextFormat("Pedido para mesa %d, mas onde fica essa mesa?", mesaAlvo), 120, 150, 20, WHITE);
        
        // A cor do tempo fica vermelha quando faltar menos de 2 segundos
        Color corTempo = (timerObstaculo > 2.0f) ? GREEN : RED;
        DrawText(TextFormat("Tempo: %.1f", timerObstaculo), 350, 200, 25, corTempo);

        // Desenha a Mesa Esquerda
        DrawRectangleRec(btnMesaEsq, DARKGRAY);
        DrawRectangleLines(btnMesaEsq.x, btnMesaEsq.y, btnMesaEsq.width, btnMesaEsq.height, LIGHTGRAY);
        DrawText(TextFormat("Mesa %d", mesaEsq), btnMesaEsq.x + 35, btnMesaEsq.y + 65, 25, WHITE);

        // Desenha a Mesa Direita
        DrawRectangleRec(btnMesaDir, DARKGRAY);
        DrawRectangleLines(btnMesaDir.x, btnMesaDir.y, btnMesaDir.width, btnMesaDir.height, LIGHTGRAY);
        DrawText(TextFormat("Mesa %d", mesaDir), btnMesaDir.x + 35, btnMesaDir.y + 65, 25, WHITE);
    }
} // Fim da função DrawMap