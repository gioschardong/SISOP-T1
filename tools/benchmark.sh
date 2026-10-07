#!/bin/sh
set -eu

# O CSV sempre usa ponto decimal. Forcar o locale C evita que o awk de
# ambientes configurados em pt_BR interprete os tempos como zero.
LC_ALL=C
export LC_ALL

RAIZ=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
REPETICOES=${1:-5}
MATRIZ=${2:-"$RAIZ/tests/matrizes/desempenho_1800x1800.txt"}
THREAD_COUNTS=${THREAD_COUNTS:-"2 4"}
CSV="$RAIZ/results/benchmark.csv"
RESUMO="$RAIZ/results/desempenho.md"
TEMPORARIO=$(mktemp "${TMPDIR:-/tmp}/sisop-benchmark.XXXXXX")
trap 'rm -f "$TEMPORARIO"' EXIT HUP INT TERM

if [ ! -f "$MATRIZ" ]; then
    echo "erro: matriz de desempenho inexistente: $MATRIZ" >&2
    echo "execute 'make matriz-desempenho' primeiro" >&2
    exit 1
fi

case "$REPETICOES" in
    ''|*[!0-9]*|0) echo "erro: REPETICOES deve ser inteiro positivo" >&2; exit 1 ;;
esac

mkdir -p "$RAIZ/results"
echo "modo,threads,repeticao,objetos,tempo_segundos" > "$CSV"

executar_configuracao()
{
    modo=$1
    threads=$2
    repeticao=1
    while [ "$repeticao" -le "$REPETICOES" ]; do
        if [ "$modo" = "sequencial" ]; then
            "$RAIZ/bin/conta-objetos-sequencial" "$MATRIZ" > "$TEMPORARIO"
        else
            "$RAIZ/bin/conta-objetos-paralelo" "$MATRIZ" "$threads" > "$TEMPORARIO"
        fi
        objetos=$(awk -F= '$1 == "objetos" { print $2 }' "$TEMPORARIO")
        tempo=$(awk -F= '$1 == "tempo_segundos" { print $2 }' "$TEMPORARIO")
        echo "$modo,$threads,$repeticao,$objetos,$tempo" >> "$CSV"
        repeticao=$((repeticao + 1))
    done
}

executar_configuracao sequencial 1
for threads in $THREAD_COUNTS; do
    executar_configuracao paralelo "$threads"
done

referencia=$(awk -F, 'NR == 2 { print $4 }' "$CSV")
if awk -F, -v ref="$referencia" 'NR > 1 && $4 != ref { erro=1 } END { exit erro }' "$CSV"; then
    :
else
    echo "erro: as configuracoes produziram contagens diferentes" >&2
    exit 1
fi

{
    echo "# Análise de desempenho"
    echo
    echo "Matriz: $(basename "$MATRIZ"). Cada configuração foi executada $REPETICOES vezes; a média aritmética do tempo interno de contagem é o valor representativo. A leitura do arquivo não entra na medição."
    echo
    echo "Ambiente: $(uname -srm). Compilador: $(${CC:-cc} --version 2>/dev/null | sed -n '1p')."
    echo
    echo "| Versão | Threads | Objetos | Tempo médio (s) | Aceleração |"
    echo "|---|---:|---:|---:|---:|"
    awk -F, '
        NR > 1 {
            chave=$1 SUBSEP $2
            soma[chave]+=$5
            n[chave]++
            objetos[chave]=$4
            ordem[++linhas]=chave
            if ($1 == "sequencial") seq+=$5
        }
        END {
            seq=seq/n["sequencial" SUBSEP "1"]
            for (i=1; i<=linhas; i++) {
                chave=ordem[i]
                if (visto[chave]++) continue
                split(chave, partes, SUBSEP)
                media=soma[chave]/n[chave]
                printf "| %s | %s | %s | %.9f | %.3f |\n", partes[1], partes[2], objetos[chave], media, seq/media
            }
        }
    ' "$CSV"
    echo
    echo "A aceleração usa S = T_sequencial / T_paralelo. Em matrizes pequenas, criação de threads, alocação e sincronização podem custar mais do que o cálculo. Nesta matriz maior, o resultado depende do processador, da carga do sistema e da largura de banda de memória; por isso os dados brutos permanecem em \`results/benchmark.csv\`."
} > "$RESUMO"

echo "Resultados gravados em $CSV e $RESUMO"
