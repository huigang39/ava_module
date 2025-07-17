TARGET      := logger_print
DIR_BUILD   := build
BIN         := $(DIR_BUILD)/$(TARGET)

DIR_INCLUDE := -I../
SRC_C 	    := $(TARGET).c

CC          := gcc
FLAGS_C     += -Wall -Wextra
FLAGS_C     += -g
FLAGS_LD    += -lpthread

ifeq ($(OS), Windows_NT)
	SHELL := cmd.exe
	MKDIR := md
	RMDIR := rd /s /q
else
	MKDIR := mkdir -p
	RMDIR := rm -rf
endif

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRC_C)
	$(MKDIR) $(DIR_BUILD)
	$(CC) $(FLAGS_C) $(DIR_INCLUDE) $< -o $@ $(FLAGS_LD)
	@echo "[COMPILE OBJ] $(OBJ)"

clean:
	$(RMDIR) $(DIR_BUILD)
	@echo "[REMOVE OBJ] $(OBJ)"
