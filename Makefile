# Detecta o sistema operacional
ifdef OS
  OS := $(strip $(OS))
else
  OS := $(strip $(shell uname))
endif

BINNAME = FigurinhaHunters

ifeq ($(OS),Windows_NT)
	INCLUDE = -I./include/
	LIBS = -L./libwin
	EXTRA_FLAGS = -Wall -std=gnu99 -Wno-missing-braces -lraylib -lopengl32 -lgdi32 -lwinmm -Wno-implicit-function-declaration
	BIN = ./output/$(BINNAME).exe
	RM = del /Q /F
else
	INCLUDE = -I./include/
	LIBS = -L./lib
	EXTRA_FLAGS = -Wall -std=gnu99 -Wno-missing-braces -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -Wno-implicit-function-declaration
	BIN = ./output/$(BINNAME)
	RM = rm -f
endif

SRC = src/*.c

all:
	gcc $(SRC) $(INCLUDE) $(LIBS) -o $(BIN) $(EXTRA_FLAGS)

run: all
	$(BIN)

clean:
	$(RM) $(BIN)
