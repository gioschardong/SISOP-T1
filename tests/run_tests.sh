#!/bin/sh
set -eu

RAIZ=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
CASOS="$RAIZ/tests/casos.txt"
TOTAL=0

extrair_objetos()
{
    awk -F= '$1 == "objetos" { print $2 }'
}

while read -r arquivo esperado; do
    case "$arquivo" in
        ''|'#'*) continue ;;
    esac

    caminho="$RAIZ/tests/$arquivo"
    obtido=$("$RAIZ/bin/conta-objetos-sequencial" "$caminho" | extrair_objetos)
    if [ "$obtido" != "$esperado" ]; then
        echo "FALHA sequencial: $arquivo, esperado=$esperado, obtido=$obtido" >&2
        exit 1
    fi

    for threads in 2 4; do
        obtido=$("$RAIZ/bin/conta-objetos-paralelo" "$caminho" "$threads" | extrair_objetos)
        if [ "$obtido" != "$esperado" ]; then
            echo "FALHA paralelo/$threads: $arquivo, esperado=$esperado, obtido=$obtido" >&2
            exit 1
        fi
    done
    TOTAL=$((TOTAL + 1))
    echo "OK: $arquivo -> $esperado objeto(s)"
done < "$CASOS"

echo "$TOTAL casos aprovados nas versoes sequencial, 2 threads e 4 threads."
