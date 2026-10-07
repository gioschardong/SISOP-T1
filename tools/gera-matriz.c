#include <stdio.h>
#include <stdlib.h>

static unsigned long proximo_aleatorio(unsigned long *estado)
{
    *estado = *estado * 1103515245UL + 12345UL;
    return (*estado / 65536UL) % 32768UL;
}

int main(int argc, char **argv)
{
    FILE *arquivo;
    unsigned long linhas;
    unsigned long colunas;
    unsigned long semente;
    unsigned long densidade;
    unsigned long linha;
    unsigned long coluna;

    if (argc != 6) {
        fprintf(stderr,
                "uso: %s LINHAS COLUNAS SEMENTE DENSIDADE_PERCENTUAL SAIDA\n",
                argv[0]);
        return EXIT_FAILURE;
    }
    linhas = strtoul(argv[1], NULL, 10);
    colunas = strtoul(argv[2], NULL, 10);
    semente = strtoul(argv[3], NULL, 10);
    densidade = strtoul(argv[4], NULL, 10);
    if (linhas == 0 || colunas == 0 || densidade > 100) {
        fprintf(stderr, "erro: parametros invalidos\n");
        return EXIT_FAILURE;
    }

    arquivo = fopen(argv[5], "w");
    if (arquivo == NULL) {
        perror("erro ao criar matriz");
        return EXIT_FAILURE;
    }
    if (fprintf(arquivo, "%lu %lu\n", linhas, colunas) < 0) {
        fclose(arquivo);
        return EXIT_FAILURE;
    }

    for (linha = 0; linha < linhas; ++linha) {
        for (coluna = 0; coluna < colunas; ++coluna) {
            int valor;

            valor = (proximo_aleatorio(&semente) % 100UL) < densidade ? 1 : 0;
            if (fprintf(arquivo, "%d%s", valor,
                        coluna + 1 == colunas ? "\n" : " ") < 0) {
                fclose(arquivo);
                return EXIT_FAILURE;
            }
        }
    }

    if (fclose(arquivo) != 0) {
        perror("erro ao fechar matriz");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
