#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "contagem.h"
#include "matriz.h"

static int ler_numero_threads(const char *texto, size_t *quantidade)
{
    char *fim;
    unsigned long valor;

    errno = 0;
    fim = NULL;
    valor = strtoul(texto, &fim, 10);
    if (errno != 0 || fim == texto || *fim != '\0' || valor < 2 ||
        (unsigned long)(size_t)valor != valor) {
        return -1;
    }
    *quantidade = (size_t)valor;
    return 0;
}

int main(int argc, char **argv)
{
    Matriz matriz;
    size_t numero_threads;
    size_t objetos;
    double inicio;
    double fim;

    if (argc != 3) {
        fprintf(stderr, "uso: %s ARQUIVO_MATRIZ NUMERO_THREADS\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (ler_numero_threads(argv[2], &numero_threads) != 0) {
        fprintf(stderr, "erro: NUMERO_THREADS deve ser um inteiro >= 2\n");
        return EXIT_FAILURE;
    }
    if (matriz_ler(argv[1], &matriz) != 0) {
        return EXIT_FAILURE;
    }
    if (numero_threads > matriz.linhas) {
        fprintf(stderr,
                "erro: numero de threads (%lu) excede as linhas (%lu)\n",
                (unsigned long)numero_threads, (unsigned long)matriz.linhas);
        matriz_liberar(&matriz);
        return EXIT_FAILURE;
    }

    inicio = tempo_agora();
    if (inicio < 0.0 ||
        contar_objetos_paralelo(&matriz, numero_threads, &objetos) != 0) {
        fprintf(stderr, "erro: falha na contagem paralela\n");
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
    printf("threads=%lu\n", (unsigned long)numero_threads);
    printf("tempo_segundos=%.9f\n", fim - inicio);
    matriz_liberar(&matriz);
    return EXIT_SUCCESS;
}
