#ifndef CONTAGEM_H
#define CONTAGEM_H

#include <stddef.h>
#include "matriz.h"

int contar_objetos_sequencial(const Matriz *matriz, size_t *quantidade);
int contar_objetos_paralelo(const Matriz *matriz, size_t numero_threads,
                            size_t *quantidade);

#endif
