CC := gcc

SRC_DIR = src
OBJ_DIR = obj

CFLAGS = -std=c11 -Wall
CFLAGS += -Wextra -Wpedantic \
          -Wformat=2 -Wno-unused-parameter -Wshadow \
          -Wwrite-strings \
          -Wredundant-decls -Wnested-externs -Wmissing-include-dirs

LIBD = lib
SDLLIBD = $(LIBD)/SDL
INCFLAGS = -lSDL2 -lSDL2_mixer
OUT = $(BIN)/out
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
BIN = bin

all: $(OUT)

$(OUT): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o bin/out $(INCFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@ $(INCFLAGS)







