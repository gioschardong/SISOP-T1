#ifndef MATRIZ_H
#define MATRIZ_H

#include <stddef.h>

typedef struct {
    size_t linhas;
    size_t colunas;
    unsigned char *celulas;
} Matriz;

int matriz_ler(const char *caminho, Matriz *matriz);
void matriz_liberar(Matriz *matriz);
double tempo_agora(void);

#endif
