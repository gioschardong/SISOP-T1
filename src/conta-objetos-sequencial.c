#include <stdio.h>
#include <stdlib.h>

#include "contagem.h"
#include "matriz.h"

int main(int argc, char **argv)
{
    Matriz matriz;
    size_t objetos;
    double inicio;
    double fim;

    if (argc != 2) {
        fprintf(stderr, "uso: %s ARQUIVO_MATRIZ\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (matriz_ler(argv[1], &matriz) != 0) {
        return EXIT_FAILURE;
    }

    inicio = tempo_agora();
    if (inicio < 0.0 || contar_objetos_sequencial(&matriz, &objetos) != 0) {
        fprintf(stderr, "erro: falha na contagem sequencial\n");
        matriz_liberar(&matriz);
        return EXIT_FAILURE;
    }
    fim = tempo_agora();
    if (fim < 0.0) {
        fprintf(stderr, "erro: falha ao medir o tempo\n");
        matriz_liberar(&matriz);
        return EXIT_FAILURE;
    }

    printf("objetos=%lu\n", (unsigned long)objetos);
    printf("tempo_segundos=%.9f\n", fim - inicio);
    matriz_liberar(&matriz);
    return EXIT_SUCCESS;
}
