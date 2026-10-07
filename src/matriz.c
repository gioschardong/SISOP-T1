#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "matriz.h"

static int converter_dimensao(unsigned long valor, size_t *destino)
{
    size_t convertido;

    convertido = (size_t)valor;
    if ((unsigned long)convertido != valor || convertido == 0) {
        return -1;
    }
    *destino = convertido;
    return 0;
}

int matriz_ler(const char *caminho, Matriz *matriz)
{
    FILE *arquivo;
    unsigned long linhas_lidas;
    unsigned long colunas_lidas;
    size_t total;
    size_t i;
    int valor;
    int caractere;

    matriz->linhas = 0;
    matriz->colunas = 0;
    matriz->celulas = NULL;

    arquivo = fopen(caminho, "r");
    if (arquivo == NULL) {
        fprintf(stderr, "erro: nao foi possivel abrir '%s': %s\n",
                caminho, strerror(errno));
        return -1;
    }

    if (fscanf(arquivo, "%lu %lu", &linhas_lidas, &colunas_lidas) != 2 ||
        converter_dimensao(linhas_lidas, &matriz->linhas) != 0 ||
        converter_dimensao(colunas_lidas, &matriz->colunas) != 0) {
        fprintf(stderr, "erro: dimensoes invalidas em '%s'\n", caminho);
        fclose(arquivo);
        return -1;
    }

    if (matriz->linhas > ((size_t)-1) / matriz->colunas) {
        fprintf(stderr, "erro: matriz grande demais em '%s'\n", caminho);
        fclose(arquivo);
        return -1;
    }
    total = matriz->linhas * matriz->colunas;
    matriz->celulas = (unsigned char *)malloc(total * sizeof(unsigned char));
    if (matriz->celulas == NULL) {
        fprintf(stderr, "erro: memoria insuficiente para a matriz\n");
        fclose(arquivo);
        return -1;
    }

    for (i = 0; i < total; ++i) {
        if (fscanf(arquivo, "%d", &valor) != 1 || (valor != 0 && valor != 1)) {
            fprintf(stderr,
                    "erro: esperado 0 ou 1 na celula %lu de '%s'\n",
                    (unsigned long)i, caminho);
            matriz_liberar(matriz);
            fclose(arquivo);
            return -1;
        }
        matriz->celulas[i] = (unsigned char)valor;
    }

    caractere = fgetc(arquivo);
    while (caractere != EOF && isspace((unsigned char)caractere)) {
        caractere = fgetc(arquivo);
    }
    if (caractere != EOF) {
        fprintf(stderr, "erro: dados excedentes em '%s'\n", caminho);
        matriz_liberar(matriz);
        fclose(arquivo);
        return -1;
    }

    if (fclose(arquivo) != 0) {
        fprintf(stderr, "erro: falha ao fechar '%s'\n", caminho);
        matriz_liberar(matriz);
        return -1;
    }
    return 0;
}

void matriz_liberar(Matriz *matriz)
{
    free(matriz->celulas);
    matriz->celulas = NULL;
    matriz->linhas = 0;
    matriz->colunas = 0;
}

double tempo_agora(void)
{
    struct timeval agora;

    if (gettimeofday(&agora, NULL) != 0) {
        return -1.0;
    }
    return (double)agora.tv_sec + (double)agora.tv_usec / 1000000.0;
}
