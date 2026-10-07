# Análise de desempenho

Matriz: desempenho_1800x1800.txt. Cada configuração foi executada 5 vezes; a média aritmética do tempo interno de contagem é o valor representativo. A leitura do arquivo não entra na medição.

Ambiente: Darwin 24.6.0 arm64. Compilador: Apple clang version 17.0.0 (clang-1700.0.13.5).

| Versão | Threads | Objetos | Tempo médio (s) | Aceleração |
|---|---:|---:|---:|---:|
| sequencial | 1 | 68267 | 0.035223007 | 1.000 |
| paralelo | 2 | 68267 | 0.020965338 | 1.680 |
| paralelo | 4 | 68267 | 0.014475965 | 2.433 |

A aceleração usa S = T_sequencial / T_paralelo. Em matrizes pequenas, criação de threads, alocação e sincronização podem custar mais do que o cálculo. Nesta matriz maior, o resultado depende do processador, da carga do sistema e da largura de banda de memória; por isso os dados brutos permanecem em `results/benchmark.csv`.
