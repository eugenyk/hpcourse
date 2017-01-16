CC=g++
C_FLAGS=-Wall -Werror -pedantic -g -std=c++11
LD_FLAGS=-ltbb

BIN_DIR=./bin
OBJ_DIR=./obj
SRC_DIR=./src


SOURCES=$(wildcard $(SRC_DIR)/*.cpp)
OBJECTS=$(addprefix $(OBJ_DIR)/,$(notdir $(SOURCES:.cpp=.o)))

BIN_NAME=imgproc
BIN=$(BIN_DIR)/$(BIN_NAME)
MAIN_TARGET=$(BIN_NAME)

all: filestructure $(MAIN_TARGET)

filestructure:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)

$(MAIN_TARGET): $(OBJECTS)
	$(CC) -o $(BIN) $(OBJECTS) $(LD_FLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) -c $< $(C_FLAGS) -o $@

clean:
	rm -rf $(BIN_DIR)/* $(OBJ_DIR)/*

.PHONY: clean all
