#include <errno.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>

#include "contagem.h"

typedef struct {
    const Matriz *matriz;
    size_t *rotulos;
    size_t *pais;
    size_t linha_inicio;
    size_t linha_fim;
    size_t componentes;
    int erro;
} Trabalho;

static void *rotular_faixa(void *argumento)
{
    Trabalho *trabalho;
    const Matriz *matriz;
    size_t capacidade;
    size_t *pilha;
    size_t linha;
    size_t coluna;

    trabalho = (Trabalho *)argumento;
    matriz = trabalho->matriz;
    capacidade = (trabalho->linha_fim - trabalho->linha_inicio) *
                 matriz->colunas;
    pilha = (size_t *)malloc(capacidade * sizeof(size_t));
    if (pilha == NULL) {
        trabalho->erro = ENOMEM;
        return NULL;
    }

    for (linha = trabalho->linha_inicio;
         linha < trabalho->linha_fim; ++linha) {
        for (coluna = 0; coluna < matriz->colunas; ++coluna) {
            size_t indice;
            size_t rotulo;
            size_t topo;

            indice = linha * matriz->colunas + coluna;
            if (matriz->celulas[indice] == 0 ||
                trabalho->rotulos[indice] != 0) {
                continue;
            }

            rotulo = indice + 1;
            trabalho->pais[rotulo] = rotulo;
            trabalho->rotulos[indice] = rotulo;
            ++trabalho->componentes;
            topo = 0;
            pilha[topo++] = indice;

            while (topo > 0) {
                size_t atual;
                size_t atual_linha;
                size_t atual_coluna;
                size_t linha_minima;
                size_t linha_maxima;
                size_t coluna_minima;
                size_t coluna_maxima;
                size_t vizinha_linha;
                size_t vizinha_coluna;

                atual = pilha[--topo];
                atual_linha = atual / matriz->colunas;
                atual_coluna = atual % matriz->colunas;
                linha_minima = atual_linha == trabalho->linha_inicio ?
                               atual_linha : atual_linha - 1;
                linha_maxima = atual_linha + 1 < trabalho->linha_fim ?
                               atual_linha + 1 : atual_linha;
                coluna_minima = atual_coluna == 0 ? 0 : atual_coluna - 1;
                coluna_maxima = atual_coluna + 1 < matriz->colunas ?
                                atual_coluna + 1 : atual_coluna;

                for (vizinha_linha = linha_minima;
                     vizinha_linha <= linha_maxima; ++vizinha_linha) {
                    for (vizinha_coluna = coluna_minima;
                         vizinha_coluna <= coluna_maxima; ++vizinha_coluna) {
                        size_t vizinha;

                        vizinha = vizinha_linha * matriz->colunas +
                                  vizinha_coluna;
                        if (matriz->celulas[vizinha] != 0 &&
                            trabalho->rotulos[vizinha] == 0) {
                            trabalho->rotulos[vizinha] = rotulo;
                            pilha[topo++] = vizinha;
                        }
                    }
                }
            }
        }
    }

    free(pilha);
    return NULL;
}

static size_t encontrar_raiz(size_t *pais, size_t valor)
{
    size_t raiz;

    raiz = valor;
    while (pais[raiz] != raiz) {
        raiz = pais[raiz];
    }
    while (pais[valor] != valor) {
        size_t anterior;

        anterior = pais[valor];
        pais[valor] = raiz;
        valor = anterior;
    }
    return raiz;
}

static int unir(size_t *pais, size_t primeiro, size_t segundo)
{
    size_t raiz_primeiro;
    size_t raiz_segundo;

    raiz_primeiro = encontrar_raiz(pais, primeiro);
    raiz_segundo = encontrar_raiz(pais, segundo);
    if (raiz_primeiro == raiz_segundo) {
        return 0;
    }
    if (raiz_primeiro < raiz_segundo) {
        pais[raiz_segundo] = raiz_primeiro;
    } else {
        pais[raiz_primeiro] = raiz_segundo;
    }
    return 1;
}

int contar_objetos_paralelo(const Matriz *matriz, size_t numero_threads,
                            size_t *quantidade)
{
    size_t total;
    size_t *rotulos;
    size_t *pais;
    pthread_t *threads;
    Trabalho *trabalhos;
    size_t base;
    size_t resto;
    size_t inicio;
    size_t i;
    size_t criadas;
    size_t componentes;
    int falhou;

    if (matriz == NULL || matriz->celulas == NULL || quantidade == NULL ||
        numero_threads < 2 || numero_threads > matriz->linhas) {
        return -1;
    }

    total = matriz->linhas * matriz->colunas;
    rotulos = (size_t *)calloc(total, sizeof(size_t));
    pais = (size_t *)calloc(total + 1, sizeof(size_t));
    threads = (pthread_t *)malloc(numero_threads * sizeof(pthread_t));
    trabalhos = (Trabalho *)calloc(numero_threads, sizeof(Trabalho));
    if (rotulos == NULL || pais == NULL || threads == NULL ||
        trabalhos == NULL) {
        free(rotulos);
        free(pais);
        free(threads);
        free(trabalhos);
        return -1;
    }

    base = matriz->linhas / numero_threads;
    resto = matriz->linhas % numero_threads;
    inicio = 0;
    criadas = 0;
    falhou = 0;

    for (i = 0; i < numero_threads; ++i) {
        size_t quantidade_linhas;
        int retorno;

        quantidade_linhas = base + (i < resto ? 1 : 0);
        trabalhos[i].matriz = matriz;
        trabalhos[i].rotulos = rotulos;
        trabalhos[i].pais = pais;
        trabalhos[i].linha_inicio = inicio;
        trabalhos[i].linha_fim = inicio + quantidade_linhas;
        inicio += quantidade_linhas;

        retorno = pthread_create(&threads[i], NULL, rotular_faixa,
                                 &trabalhos[i]);
        if (retorno != 0) {
            falhou = 1;
            break;
        }
        ++criadas;
    }

    for (i = 0; i < criadas; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            falhou = 1;
        }
        if (trabalhos[i].erro != 0) {
            falhou = 1;
        }
    }
    if (falhou) {
        free(trabalhos);
        free(threads);
        free(pais);
        free(rotulos);
        return -1;
    }

    componentes = 0;
    for (i = 0; i < numero_threads; ++i) {
        componentes += trabalhos[i].componentes;
    }

    for (i = 0; i + 1 < numero_threads; ++i) {
        size_t linha_superior;
        size_t linha_inferior;
        size_t coluna;

        linha_superior = trabalhos[i].linha_fim - 1;
        linha_inferior = trabalhos[i + 1].linha_inicio;
        for (coluna = 0; coluna < matriz->colunas; ++coluna) {
            size_t indice_superior;
            size_t coluna_inicio;
            size_t coluna_fim;
            size_t coluna_inferior;

            indice_superior = linha_superior * matriz->colunas + coluna;
            if (matriz->celulas[indice_superior] == 0) {
                continue;
            }
            coluna_inicio = coluna == 0 ? 0 : coluna - 1;
            coluna_fim = coluna + 1 < matriz->colunas ? coluna + 1 : coluna;
            for (coluna_inferior = coluna_inicio;
                 coluna_inferior <= coluna_fim; ++coluna_inferior) {
                size_t indice_inferior;

                indice_inferior = linha_inferior * matriz->colunas +
                                  coluna_inferior;
                if (matriz->celulas[indice_inferior] != 0 &&
                    unir(pais, rotulos[indice_superior],
                         rotulos[indice_inferior])) {
                    --componentes;
                }
            }
        }
    }

    *quantidade = componentes;
    free(trabalhos);
    free(threads);
    free(pais);
    free(rotulos);
    return 0;
}
