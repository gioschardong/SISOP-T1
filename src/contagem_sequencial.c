#include <stdlib.h>

#include "contagem.h"

int contar_objetos_sequencial(const Matriz *matriz, size_t *quantidade)
{
    unsigned char *visitadas;
    size_t *pilha;
    size_t total;
    size_t indice;
    size_t topo;
    size_t objetos;

    if (matriz == NULL || matriz->celulas == NULL || quantidade == NULL) {
        return -1;
    }

    total = matriz->linhas * matriz->colunas;
    visitadas = (unsigned char *)calloc(total, sizeof(unsigned char));
    pilha = (size_t *)malloc(total * sizeof(size_t));
    if (visitadas == NULL || pilha == NULL) {
        free(visitadas);
        free(pilha);
        return -1;
    }

    objetos = 0;
    for (indice = 0; indice < total; ++indice) {
        size_t linha;
        size_t coluna;

        if (matriz->celulas[indice] == 0 || visitadas[indice] != 0) {
            continue;
        }

        ++objetos;
        topo = 0;
        pilha[topo++] = indice;
        visitadas[indice] = 1;

        while (topo > 0) {
            size_t atual;
            size_t linha_inicio;
            size_t linha_fim;
            size_t coluna_inicio;
            size_t coluna_fim;
            size_t vizinha_linha;
            size_t vizinha_coluna;

            atual = pilha[--topo];
            linha = atual / matriz->colunas;
            coluna = atual % matriz->colunas;
            linha_inicio = linha == 0 ? 0 : linha - 1;
            linha_fim = linha + 1 < matriz->linhas ? linha + 1 : linha;
            coluna_inicio = coluna == 0 ? 0 : coluna - 1;
            coluna_fim = coluna + 1 < matriz->colunas ? coluna + 1 : coluna;

            for (vizinha_linha = linha_inicio;
                 vizinha_linha <= linha_fim; ++vizinha_linha) {
                for (vizinha_coluna = coluna_inicio;
                     vizinha_coluna <= coluna_fim; ++vizinha_coluna) {
                    size_t vizinha;

                    vizinha = vizinha_linha * matriz->colunas + vizinha_coluna;
                    if (matriz->celulas[vizinha] != 0 &&
                        visitadas[vizinha] == 0) {
                        visitadas[vizinha] = 1;
                        pilha[topo++] = vizinha;
                    }
                }
            }
        }
    }

    free(pilha);
    free(visitadas);
    *quantidade = objetos;
    return 0;
}
