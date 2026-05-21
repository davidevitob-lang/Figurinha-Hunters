#ifndef MAP_H
#define MAP_H

#include "raylib.h"

// Estrutura para representar uma loja ou ponto de interesse
typedef struct {
    Rectangle area;
    Color cor;
    bool ativa;
    float tempoAtivacao; 
} PontoInteresse;

void InitMap(void);
void UpdateMap(void);
void DrawMap(void);

#endif