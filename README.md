# Contagem paralela de objetos em matriz binária

Trabalho prático de Sistemas Operacionais (PUCRS, 2026/II). O projeto compara uma referência sequencial com uma implementação paralela baseada em Pthreads. As duas versões contam componentes conexos de valor `1` usando conectividade 8.

Autores: Ana Schmitt e Giovanni Schardong.

Vídeo: https://youtu.be/QUMmf3xhu-o

## Compilação

Requisitos: compilador C com APIs POSIX, `make` e shell POSIX. O projeto foi escrito em ANSI C (C89/C90).

```sh
make
```

O comando produz:

- `bin/conta-objetos-sequencial`
- `bin/conta-objetos-paralelo`
- `bin/gera-matriz`

## Formato de entrada

A primeira linha contém `LINHAS COLUNAS`. Em seguida aparecem exatamente `LINHAS * COLUNAS` valores, cada um igual a `0` ou `1`.

```text
3 4
1 1 0 0
0 1 0 1
0 0 0 1
```

## Execução

```sh
bin/conta-objetos-sequencial tests/matrizes/exemplo1.txt
bin/conta-objetos-paralelo tests/matrizes/exemplo1.txt 4
```

A versão paralela exige pelo menos duas threads e no máximo uma thread por linha. A saída usa pares `chave=valor`, o que facilita os testes e a coleta de tempos.

## Arquitetura

A versão sequencial usa flood fill iterativo. Uma pilha alocada no heap evita a profundidade de recursão proporcional ao tamanho do objeto.

A versão paralela divide a matriz em faixas contíguas e balanceadas de linhas. Cada thread executa flood fill somente dentro da sua faixa e atribui um rótulo a cada componente local. Como duas threads nunca escrevem na mesma linha, a fase local não precisa de mutex.

`pthread_join` forma a barreira de sincronização. Depois que todas as threads terminam, a thread principal compara cada fronteira horizontal, incluindo as duas diagonais adjacentes, e une rótulos equivalentes com uma estrutura union-find. Essa consolidação é sequencial e não sofre condições de corrida. Um objeto que atravessa várias faixas ou chega diagonalmente ao encontro de regiões é unificado de forma transitiva.

O trabalho paralelo cobre todas as células. A etapa sequencial de consolidação examina apenas as fronteiras entre faixas. Para uma matriz com `L` linhas, `C` colunas e `T` threads, o trabalho total é `O(L*C)` e a consolidação custa `O((T-1)*C)`.

## Resultados de execução

### Testes de correção

```sh
make test
```

O teste executa as cinco matrizes obrigatórias nas versões sequencial, com 2 threads e com 4 threads. Também cobre uma matriz vazia e uma cadeia diagonal.

| Caso | Dimensões | Esperado | Sequencial | Paralelo (2 threads) | Paralelo (4 threads) |
|---:|---:|---:|---:|---:|---:|
| Exemplo 1 | 5 x 5 | 3 | 3 | 3 | 3 |
| Exemplo 2 | 6 x 8 | 4 | 4 | 4 | 4 |
| Exemplo 3 | 8 x 8 | 5 | 5 | 5 | 5 |
| Exemplo 4 | 9 x 12 | 6 | 6 | 6 | 6 |
| Exemplo 5 | 12 x 12 | 7 | 7 | 7 | 7 |
| Matriz vazia | 4 x 4 | 0 | 0 | 0 | 0 |
| Cadeia diagonal | 5 x 5 | 1 | 1 | 1 | 1 |

As três configurações produziram a mesma contagem em todos os casos. Em particular, os exemplos 2 a 5 confirmam que a consolidação preserva objetos que atravessam faixas por conexões horizontais, verticais ou diagonais. A cadeia diagonal verifica diretamente a conectividade 8, enquanto a matriz vazia verifica o caso sem componentes.

### Experimento de desempenho

```sh
make benchmark
```

O alvo gera de forma determinística uma matriz de 1800 x 1800, com densidade de 38%, semente `20260929` e 3.240.000 células. O experimento executa cinco repetições da versão sequencial e das versões com 2 e 4 threads sobre os mesmos dados. A leitura do arquivo ocorre antes do cronômetro, portanto os valores medem apenas a contagem. A média aritmética representa cada configuração.

Ambiente da medição: Darwin 24.6.0 arm64, usando Apple clang 17.0.0.

| Versão | Threads | Objetos | Tempo médio (s) | Aceleração |
|---|---:|---:|---:|---:|
| Sequencial | 1 | 68.267 | 0,035645628 | 1,000 |
| Paralela | 2 | 68.267 | 0,021617174 | 1,649 |
| Paralela | 4 | 68.267 | 0,013322878 | 2,676 |

A aceleração foi calculada por `S = T_sequencial / T_paralelo`. A configuração com 2 threads reduziu o tempo médio em aproximadamente 39,4%, enquanto a configuração com 4 threads reduziu o tempo em aproximadamente 62,6%. Todas as configurações encontraram os mesmos 68.267 objetos, confirmando que o ganho de desempenho não alterou o resultado.

<details>
<summary>Tempos das cinco repetições</summary>

| Repetição | Sequencial (s) | 2 threads (s) | 4 threads (s) |
|---:|---:|---:|---:|
| 1 | 0,035778046 | 0,021659136 | 0,012764215 |
| 2 | 0,035566092 | 0,021222115 | 0,014270067 |
| 3 | 0,035881996 | 0,021597862 | 0,012959957 |
| 4 | 0,035465956 | 0,021380901 | 0,013384104 |
| 5 | 0,035536051 | 0,022225857 | 0,013236046 |

</details>

O paralelismo não produz aceleração perfeita porque existem custos de criação e finalização das threads, alocação das estruturas de rotulagem, sincronização por `pthread_join` e consolidação das fronteiras. O acesso concorrente à memória também pode limitar o desempenho. Esses custos tendem a dominar em matrizes pequenas. Por isso o experimento utiliza uma entrada maior.

Para mudar as quantidades de threads ou o número de repetições:

```sh
THREAD_COUNTS="2 4 8" sh tools/benchmark.sh 10
```

Os dados brutos também ficam disponíveis em `results/benchmark.csv`. Os arquivos da pasta `results/` são mantidos como registros auxiliares, mas as informações necessárias para avaliar correção e desempenho estão consolidadas no README.

## Organização

```text
include/   interfaces da matriz e dos algoritmos
src/       leitura, medição, contagem sequencial e contagem paralela
tests/     matrizes obrigatórias, casos adicionais e testes automatizados
tools/     gerador determinístico e script de benchmark
results/   resultados de correção e desempenho
slides/    apresentação em PPTX e PDF
```

## Tratamento de erros e recursos

Os programas validam argumentos, dimensões, conteúdo da matriz, alocações, criação e finalização de threads. Arquivos e áreas alocadas são liberados nos caminhos normais e de erro. As principais chamadas POSIX têm seus retornos verificados.
