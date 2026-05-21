#ifndef ALBUM_H
#define ALBUM_H

#include <stdbool.h>

typedef struct {
    char codigo[20];
    char titulo[100];
    char secao[50];
    char grupo[20];
    char tipo[30];
    bool colada;
    int paraTroca;
} Figurinha;

// Gerenciamento de Dados
Figurinha* carregarDadosIniciais(int *total);
void salvarDadosBinario(Figurinha *album, int total);

// CRUD e Busca
Figurinha* adicionarFigurinha(Figurinha *album, int *total, const char* titulo);
Figurinha* alocarEspacoVazio(Figurinha *album, int *total);
Figurinha* removerFigurinha(Figurinha *album, int *total, int indice); 
Figurinha* resetarAlbum(Figurinha *album, int *total);                

int pesquisarFigurinha(Figurinha *album, int total, const char *termo);

#endif