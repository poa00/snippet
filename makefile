CC 		:= clang++
CFLAGS	:= -std=c++17 -Wall

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

EXE		:= $(BIN_DIR)/snippet

SRC 	:= $(wildcard $(SRC_DIR)/*.cc)
HEADER	:= $(wildcard $(SRC_DIR)/*.h)

OBJ 	:= $(SRC:$(SRC_DIR)/%.cc=$(OBJ_DIR)/%.o)

all: $(EXE)

.PHONY: all

install: $(EXE)
	mv $(EXE) ~/.local/bin/

debug: $(EXE)
	$(info running ./bin/main through valgrind)
	valgrind --tool=memcheck -s $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc $(HEADER)| $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)