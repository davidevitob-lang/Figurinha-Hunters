#ifndef CADASTRO_H
#define CADASTRO_H

#include "album.h"

// Agora recebe o indiceEdicao: -1 para novo, >= 0 para editar
void UpdateDrawCadastro(Figurinha **album, int *total, int *estadoAtual, int indiceEdicao);

#endif