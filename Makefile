# ==============================
# Makefile para Compilador
# ==============================

# Compilador y banderas
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

# Carpetas
SRC_DIR = src
LEXER_DIR = $(SRC_DIR)/lexer
PARSER_DIR = $(SRC_DIR)/parser
INC_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Archivos fuente
MAIN_SRC = $(SRC_DIR)/main.c
LEXER_SRC = $(wildcard $(LEXER_DIR)/*.c)
PARSER_SRC = $(wildcard $(PARSER_DIR)/*.c)
ALL_SRC = $(MAIN_SRC) $(LEXER_SRC) $(PARSER_SRC)

# Archivos objeto
MAIN_OBJ = $(BUILD_DIR)/main.o
LEXER_OBJ = $(patsubst $(LEXER_DIR)/%.c, $(BUILD_DIR)/lexer/%.o, $(LEXER_SRC))
PARSER_OBJ = $(patsubst $(PARSER_DIR)/%.c, $(BUILD_DIR)/parser/%.o, $(PARSER_SRC))
ALL_OBJ = $(MAIN_OBJ) $(LEXER_OBJ) $(PARSER_OBJ)

# Ejecutables
TARGET = $(BIN_DIR)/compilador
LEXER_TEST = $(BIN_DIR)/lexer-test

# Archivos de prueba
TEST_FILE = src/lexer/test.txt
EXAMPLES_DIR = docs/Analizador-Lexico/examples

# ==============================
# Reglas principales
# ==============================

# Regla por defecto
all: directories $(TARGET)

# Crear directorios necesarios
directories:
	@mkdir -p $(BUILD_DIR)/lexer $(BUILD_DIR)/parser $(BIN_DIR)

# Compilar ejecutable principal
$(TARGET): $(ALL_OBJ) | directories
	@echo "Enlazando ejecutable principal..."
	$(CC) $(CFLAGS) -o $@ $(ALL_OBJ)
	@echo "✓ Compilado: $(TARGET)"

# Compilar solo el lexer para pruebas
$(LEXER_TEST): $(BUILD_DIR)/lexer/lexer.o $(BUILD_DIR)/lexer/keywords.o | directories
	@echo "Enlazando test del lexer..."
	$(CC) $(CFLAGS) -DLEXER_STANDALONE -o $@ $^
	@echo "✓ Compilado: $(LEXER_TEST)"

# ==============================
# Reglas de compilación
# ==============================

# Compilar main.c
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c | directories
	@echo "Compilando main.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar archivos del lexer
$(BUILD_DIR)/lexer/%.o: $(LEXER_DIR)/%.c | directories
	@echo "Compilando lexer: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Compilar archivos del parser
$(BUILD_DIR)/parser/%.o: $(PARSER_DIR)/%.c | directories
	@echo "Compilando parser: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# ==============================
# Reglas de limpieza
# ==============================

# Limpiar todo
clean:
	@echo "Limpiando archivos compilados..."
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "✓ Limpieza completa"

# Limpiar solo objetos
clean-obj:
	@echo "Limpiando archivos objeto..."
	rm -rf $(BUILD_DIR)
	@echo "✓ Archivos objeto eliminados"

# ==============================
# Reglas de ejecución
# ==============================

# Ejecutar con análisis léxico
run-lex: $(TARGET)
	@echo "=== Ejecutando análisis léxico ==="
	./$(TARGET) -l $(TEST_FILE)

# Ejecutar con análisis léxico y sintáctico
run-parse: $(TARGET)
	@echo "=== Ejecutando análisis léxico y sintáctico ==="
	./$(TARGET) -p $(TEST_FILE)

# Ejecutar compilación completa
run: $(TARGET)
	@echo "=== Ejecutando compilación completa ==="
	./$(TARGET) $(TEST_FILE)

# Ejecutar con archivo personalizado
run-file: $(TARGET)
	@if [ -z "$(FILE)" ]; then \
		echo "Error: Especifica un archivo con FILE=archivo.txt"; \
		exit 1; \
	fi
	./$(TARGET) $(FILE)

# Generar archivo de tokens
tokens: $(TARGET)
	@echo "=== Generando archivo de tokens ==="
	./$(TARGET) -t $(TEST_FILE)

# Generar tokens con archivo personalizado
tokens-file: $(TARGET)
	@if [ -z "$(FILE)" ]; then \
		echo "Error: Especifica un archivo con FILE=archivo.txt"; \
		exit 1; \
	fi
	./$(TARGET) -t $(FILE)

# ==============================
# Reglas de pruebas
# ==============================

# Probar con todos los ejemplos de éxito
test-examples: $(TARGET)
	@echo "=== Probando ejemplos de éxito ==="
	@for file in $(EXAMPLES_DIR)/exito-*.txt; do \
		if [ -f "$$file" ]; then \
			echo "Probando: $$file"; \
			./$(TARGET) -l "$$file" || true; \
			echo ""; \
		fi \
	done

# Probar con ejemplos de error
test-errors: $(TARGET)
	@echo "=== Probando ejemplos de error ==="
	@for file in $(EXAMPLES_DIR)/error-*.txt; do \
		if [ -f "$$file" ]; then \
			echo "Probando: $$file"; \
			./$(TARGET) -l "$$file" || true; \
			echo ""; \
		fi \
	done

# Ejecutar todas las pruebas
test: test-examples test-errors

# ==============================
# Reglas de información
# ==============================

# Mostrar información del proyecto
info:
	@echo "=== Información del Compilador ==="
	@echo "Archivos fuente: $(words $(ALL_SRC))"
	@echo "  - Main: $(MAIN_SRC)"
	@echo "  - Lexer: $(words $(LEXER_SRC)) archivos"
	@echo "  - Parser: $(words $(PARSER_SRC)) archivos"


# Mostrar ayuda
help:
	@echo "=== Makefile del Compilador - Ayuda ==="
	@echo ""
	@echo "Reglas principales:"
	@echo "  all          - Compilar el proyecto completo"
	@echo "  clean        - Limpiar archivos compilados"
	@echo ""
	@echo "Ejecución:"
	@echo "  run          - Ejecutar compilación completa"
	@echo "  run-lex      - Ejecutar solo análisis léxico"
	@echo "  run-parse    - Ejecutar análisis léxico y sintáctico"
	@echo "  run-file FILE=archivo.txt - Ejecutar con archivo específico"
	@echo "  tokens       - Generar archivo de tokens del archivo de prueba"
	@echo "  tokens-file FILE=archivo.txt - Generar tokens de archivo específico"
	@echo ""
	@echo "Pruebas:"
	@echo "  test         - Ejecutar todas las pruebas"
	@echo "  test-examples - Probar ejemplos de éxito"
	@echo "  test-errors  - Probar ejemplos de error"
	@echo ""
	@echo "Información:"
	@echo "  info         - Mostrar información del proyecto"
	@echo "  help         - Mostrar esta ayuda"
	@echo ""
	@echo "Ejemplos:"
	@echo "  make all              # Compilar todo"
	@echo "  make run-lex          # Solo análisis léxico"
	@echo "  make tokens           # Generar archivo de tokens"
	@echo "  make run-file FILE=mi_programa.lang"
	@echo "  make tokens-file FILE=mi_programa.lang"

# ==============================
# Reglas que no son archivos
# ==============================

.PHONY: all clean clean-obj run run-lex run-parse run-file tokens tokens-file \
        test test-examples test-errors info help directories
