# Makefile automatizado para projeto C
PROJ_NAME = ted
LIBS = -lm

# Busca arquivos fonte
SRC_FILES := $(shell find src -name "*.c" 2>/dev/null)
ifeq ($(SRC_FILES),)
	SRC_FILES := $(wildcard src/*.c) $(wildcard src/*/*.c) $(wildcard src/*/*/*.c)
endif
OBJETOS := $(SRC_FILES:.c=.o) # Substitui .c por .o

# Compilador e flags
CC = gcc
CFLAGS = -ggdb -O0 -std=c99 -fstack-protector-all -Werror=implicit-function-declaration
LDFLAGS = -O0

# Regra principal
$(PROJ_NAME): $(OBJETOS)
	$(CC) -o $(PROJ_NAME) $(LDFLAGS) $(OBJETOS) $(LIBS)

# Compila cada .c em .o
%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

# Limpeza dos arquivos gerados
clean:
	rm -f $(OBJETOS) $(PROJ_NAME)

# Mostra variáveis para debug
debug:
	@echo "SRC_FILES: $(SRC_FILES)"
	@echo "OBJETOS: $(OBJETOS)"