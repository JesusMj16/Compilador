# ==============================
# Makefile para Compilador (VERSIÓN AUTOMATIZADA)
# Flex + Bison
# ==============================

# Toolchain
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Ibuild

FLEX ?= flex
BISON ?= bison

# Compatibilidad Windows/Unix
ifeq ($(OS),Windows_NT)
	SHELL := cmd.exe
	.SHELLFLAGS := /C
	EXEEXT := .exe
	# Autodetección (MSYS2 suele instalar flex/bison aquí)
	ifneq ("$(wildcard C:/msys64/usr/bin/bison.exe)","")
		BISON := C:/msys64/usr/bin/bison.exe
	endif
	ifneq ("$(wildcard C:/msys64/usr/bin/flex.exe)","")
		FLEX := C:/msys64/usr/bin/flex.exe
	endif
else
	EXEEXT :=
endif

# Carpetas
SRC_DIR = src
AUTO_DIR = $(SRC_DIR)/automatizado
BUILD_DIR = build
BIN_DIR = bin

# Entradas automatizadas
LEXER_L = $(AUTO_DIR)/lexer/lexer.l
PARSER_Y = $(AUTO_DIR)/parser/parser.y

# Salidas generadas
LEXER_GEN_C = $(BUILD_DIR)/lexer.yy.c
PARSER_GEN_C = $(BUILD_DIR)/parser.tab.c
PARSER_GEN_H = $(BUILD_DIR)/parser.tab.h

# Fuentes
MAIN_SRC = $(SRC_DIR)/main.c

# Objetos
MAIN_OBJ = $(BUILD_DIR)/main.o
LEXER_OBJ = $(BUILD_DIR)/lexer.yy.o
PARSER_OBJ = $(BUILD_DIR)/parser.tab.o
ALL_OBJ = $(MAIN_OBJ) $(LEXER_OBJ) $(PARSER_OBJ)

# Ejecutables
TARGET = $(BIN_DIR)/compilador$(EXEEXT)
LEXER_RUNNER = $(BIN_DIR)/flex-runner$(EXEEXT)

# Pruebas
LEX_EXAMPLES_DIR = docs/Analizador-Lexico/examples-flex
PARSER_EXAMPLES_DIR = docs/Analizador-sintactico/examples-bison

# ==============================
# Reglas principales
# ==============================

# Regla por defecto
all: directories generate $(TARGET)

# Crear directorios necesarios
directories:
	@if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
	@if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"

$(TARGET): $(ALL_OBJ) | directories
	@echo "Enlazando ejecutable principal (Flex+Bison)..."
	$(CC) $(CFLAGS) -o $@ $(ALL_OBJ)
	@echo "✓ Compilado: $(TARGET)"

$(LEXER_RUNNER): $(LEXER_OBJ) $(BUILD_DIR)/flex_runner.o | directories
	@echo "Enlazando runner del lexer (Flex standalone)..."
	$(CC) $(CFLAGS) -o $@ $^
	@echo "✓ Compilado: $(LEXER_RUNNER)"

# ==============================
# Reglas de compilación
# ==============================

$(MAIN_OBJ): $(MAIN_SRC) $(PARSER_GEN_H) | directories
	@echo "Compilando main.c (driver Bison)..."
	$(CC) $(CFLAGS) -c $< -o $@

$(PARSER_OBJ): $(PARSER_GEN_C) | directories
	@echo "Compilando parser generado: $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(LEXER_OBJ): $(LEXER_GEN_C) $(PARSER_GEN_H) | directories
	@echo "Compilando lexer generado: $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/flex_runner.o: $(AUTO_DIR)/lexer/flex_runner.c | directories
	@echo "Compilando flex_runner.c..."
	$(CC) $(CFLAGS) -c $< -o $@

# ==============================
# Generación (Flex/Bison)
# ==============================

generate: $(PARSER_GEN_C) $(LEXER_GEN_C)

$(PARSER_GEN_C) $(PARSER_GEN_H): $(PARSER_Y) | directories
	@echo "Generando parser con Bison..."
	$(BISON) -d -o $(PARSER_GEN_C) $(PARSER_Y)

$(LEXER_GEN_C): $(LEXER_L) $(PARSER_GEN_H) | directories
	@echo "Generando lexer con Flex..."
	$(FLEX) -o $(LEXER_GEN_C) $(LEXER_L)

# ==============================
# Reglas de limpieza
# ==============================

# Limpiar todo
clean:
	@echo "Limpiando archivos compilados..."
	@if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)"
	@if exist "$(BIN_DIR)" rmdir /S /Q "$(BIN_DIR)"
	@echo "✓ Limpieza completa"

# Limpiar solo objetos
clean-obj:
	@echo "Limpiando archivos objeto..."
	@if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)"
	@echo "✓ Archivos objeto eliminados"

# ==============================
# Reglas de ejecución
# ==============================

run: $(TARGET)
	@echo "=== Ejecutando parse (automático) ==="
	@if "$(FILE)"=="" (echo Error: Especifica un archivo con FILE=archivo.txt & exit /b 1)
	"$(TARGET)" "$(FILE)"

run-file: $(TARGET)
	@if "$(FILE)"=="" (echo Error: Especifica un archivo con FILE=archivo.txt & exit /b 1)
	"$(TARGET)" "$(FILE)"

lex-runner: $(LEXER_RUNNER)
	@echo "=== Ejecutando runner del lexer (Flex standalone) ==="
	"$(LEXER_RUNNER)" --check-all "$(LEX_EXAMPLES_DIR)"

# ==============================
# Reglas de pruebas
# ==============================

test-lexer: lex-runner

# ==============================
# Reglas de información
# ==============================

# Mostrar información del proyecto
info:
	@echo "=== Información del Compilador (Automatizado) ==="
	@echo "  - Lexer Flex: $(LEXER_L)"
	@echo "  - Parser Bison: $(PARSER_Y)"


# Mostrar ayuda
help:
	@echo "=== Makefile del Compilador - Ayuda ==="
	@echo ""
	@echo "Reglas principales:"
	@echo "  all          - Generar (Flex/Bison) y compilar"
	@echo "  generate     - Solo generar lexer/parser"
	@echo "  clean        - Limpiar archivos compilados"
	@echo ""
	@echo "Ejecución:"
	@echo "  run-file FILE=archivo.txt - Ejecutar con archivo específico"
	@echo "  test-lexer   - Ejecuta el runner del lexer en examples-flex"
	@echo ""
	@echo "Pruebas:"
	@echo "  test-lexer   - Pruebas del lexer Flex (standalone)"
	@echo ""
	@echo "Información:"
	@echo "  info         - Mostrar información del proyecto"
	@echo "  help         - Mostrar esta ayuda"
	@echo ""
	@echo "Ejemplos:"
	@echo "  mingw32-make all"
	@echo "  mingw32-make test-lexer"
	@echo "  mingw32-make run-file FILE=docs/Analizador-sintactico/archivos_parser/exito-01.txt"

# ==============================
# Reglas que no son archivos
# ==============================

.PHONY: all clean clean-obj generate run run-file lex-runner test-lexer info help directories
