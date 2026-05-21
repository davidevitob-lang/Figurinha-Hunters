#include "map.h"

// Criamos uma loja de teste
PontoInteresse lojaExemplo;

void InitMap(void) {
    lojaExemplo.area = (Rectangle){ 400, 300, 50, 50 }; // Posição no meio da tela
    lojaExemplo.cor = GRAY;
    lojaExemplo.ativa = false;
    lojaExemplo.tempoAtivacao = 0.0f;
}

void UpdateMap(void) {
    Vector2 mousePos = GetMousePosition();

    // Se clicar com o botão esquerdo em cima da loja
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(mousePos, lojaExemplo.area)) {
            lojaExemplo.ativa = true;
            lojaExemplo.tempoAtivacao = GetTime(); // Marca o momento do clique
        }
    }

    // Se passaram 3 segundos, desativa o "cubo verde"
    if (lojaExemplo.ativa && (GetTime() - lojaExemplo.tempoAtivacao >= 3.0f)) {
        lojaExemplo.ativa = false;
    }
}

void DrawMap(void) {
    // Desenha a base da loja
    DrawRectangleRec(lojaExemplo.area, lojaExemplo.cor);
    DrawText("LOJA", lojaExemplo.area.x, lojaExemplo.area.y - 20, 10, BLACK);

    // Se estiver ativa (dentro dos 3s), desenha o feedback verde
    if (lojaExemplo.ativa) {
        DrawRectangle(lojaExemplo.area.x + 10, lojaExemplo.area.y + 10, 30, 30, GREEN);
    }
}