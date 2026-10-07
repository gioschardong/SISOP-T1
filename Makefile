CC ?= cc
CPPFLAGS = -Iinclude
CFLAGS = -std=c89 -Wall -Wextra -Wpedantic -O2
PTHREAD_FLAGS = -pthread

BIN_DIR = bin
BUILD_DIR = build
SEQ = $(BIN_DIR)/conta-objetos-sequencial
PAR = $(BIN_DIR)/conta-objetos-paralelo
GERADOR = $(BIN_DIR)/gera-matriz
COMUNS = $(BUILD_DIR)/matriz.o

.PHONY: all test benchmark matriz-desempenho clean

all: $(SEQ) $(PAR) $(GERADOR)

$(BIN_DIR) $(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/matriz.o: src/matriz.c include/matriz.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/contagem_sequencial.o: src/contagem_sequencial.c include/contagem.h include/matriz.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/contagem_paralela.o: src/contagem_paralela.c include/contagem.h include/matriz.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PTHREAD_FLAGS) -c $< -o $@

$(BUILD_DIR)/main_sequencial.o: src/conta-objetos-sequencial.c include/contagem.h include/matriz.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/main_paralelo.o: src/conta-objetos-paralelo.c include/contagem.h include/matriz.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PTHREAD_FLAGS) -c $< -o $@

$(SEQ): $(COMUNS) $(BUILD_DIR)/contagem_sequencial.o $(BUILD_DIR)/main_sequencial.o | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(PAR): $(COMUNS) $(BUILD_DIR)/contagem_paralela.o $(BUILD_DIR)/main_paralelo.o | $(BIN_DIR)
	$(CC) $(CFLAGS) $(PTHREAD_FLAGS) $^ -o $@

$(GERADOR): tools/gera-matriz.c | $(BIN_DIR)
	$(CC) $(CFLAGS) $< -o $@

matriz-desempenho: $(GERADOR)
	$(GERADOR) 1800 1800 20260929 38 tests/matrizes/desempenho_1800x1800.txt

test: all
	sh tests/run_tests.sh

benchmark: all matriz-desempenho
	sh tools/benchmark.sh 5

clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)
	rm -f tests/matrizes/desempenho_1800x1800.txt
