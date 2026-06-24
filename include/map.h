#ifndef MAP_H
#define MAP_H
#include "raylib.h"
#include "album.h"

// Estrutura para representar uma loja ou ponto de interesse
typedef struct {
    Rectangle area;
    Color cor;
    bool ativa;
    float tempoAtivacao; 
} PontoInteresse;

void InitMap(void);
void UpdateMap(Figurinha *album, int total);
void DrawMap(Figurinha *meuAlbum, int total);

#endif