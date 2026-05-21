#include "raylib.h"
#include "cadastro.h"
#include <string.h>

static char inputTitulo[100] = "\0";
static char inputCodigo[20] = "\0";
static int campoFocado = 0;
static bool carregouDados = false;

void UpdateDrawCadastro(Figurinha **album, int *total, int *estadoAtual, int indiceEdicao) {
    // Se estiver editando e ainda não carregou os dados para o buffer
    if (indiceEdicao != -1 && !carregouDados) {
        strcpy(inputTitulo, (*album)[indiceEdicao].titulo);
        strcpy(inputCodigo, (*album)[indiceEdicao].codigo);
        carregouDados = true;
    }

    // Lógica de Foco e Digitação
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (GetMouseY() > 180 && GetMouseY() < 220) campoFocado = 0;
        else if (GetMouseY() > 280 && GetMouseY() < 320) campoFocado = 1;
    }
    if (IsKeyPressed(KEY_TAB)) campoFocado = !campoFocado;

    int key = GetCharPressed();
    char *ref = (campoFocado == 0) ? inputTitulo : inputCodigo;
    int maxLen = (campoFocado == 0) ? 99 : 19;

    while (key > 0) {
        if (key >= 32 && key <= 125 && strlen(ref) < maxLen) {
            int len = strlen(ref);
            ref[len] = (char)key; ref[len+1] = '\0';
        }
        key = GetCharPressed();
    }
    if (IsKeyPressed(KEY_BACKSPACE) && strlen(ref) > 0) ref[strlen(ref)-1] = '\0';

    // Botão Finalizar
    Rectangle btnFinalizar = { 300, 400, 200, 50 };
    bool mouseSobre = CheckCollisionPointRec(GetMousePosition(), btnFinalizar);

    if ((mouseSobre && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) || IsKeyPressed(KEY_ENTER)) {
        if (indiceEdicao == -1) {
            // MODO NOVO: Cria espaço e salva
            *album = alocarEspacoVazio(*album, total);
            strcpy((*album)[*total - 1].titulo, inputTitulo);
            strcpy((*album)[*total - 1].codigo, inputCodigo);
        } else {
            // MODO EDIÇÃO: Apenas sobrescreve os dados no índice atual
            strcpy((*album)[indiceEdicao].titulo, inputTitulo);
            strcpy((*album)[indiceEdicao].codigo, inputCodigo);
        }

        // Reseta buffers para a próxima vez
        inputTitulo[0] = '\0'; inputCodigo[0] = '\0';
        carregouDados = false;
        *estadoAtual = 2; // Volta para FIGURINHAS
    }

    // Desenho
    DrawText(indiceEdicao == -1 ? "NOVO CADASTRO" : "EDITAR FIGURINHA", 280, 50, 25, DARKBLUE);
    
    DrawText("Título:", 200, 150, 20, DARKGRAY);
    DrawRectangle(200, 180, 400, 40, campoFocado == 0 ? LIGHTGRAY : WHITE);
    DrawText(inputTitulo, 210, 190, 20, BLACK);

    DrawText("Código:", 200, 250, 20, DARKGRAY);
    DrawRectangle(200, 280, 400, 40, campoFocado == 1 ? LIGHTGRAY : WHITE);
    DrawText(inputCodigo, 210, 290, 20, BLACK);

    DrawRectangleRec(btnFinalizar, mouseSobre ? SKYBLUE : BLUE);
    DrawText("SALVAR", 365, 415, 20, WHITE);
}