CXX := gcc
OPT := -O3 -std=c23 -Isrc
OPS := -march=native -fopt-info-vec-all=vec.txt -S -masm=intel
PRF := -g -pg

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin

TARGET := $(BIN_DIR)/compare_str

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(BIN_DIR) $(OBJ_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OPT) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CXX) $(OPT) -c $< -o $@

$(OBJ_DIR)/compare_str.o: $(SRC_DIR)/compare_str.c
	$(CXX) $(OPT) -c $< -o $@
	rm -f vec.txt
	$(CXX) $(OPT) $(OPS) -S -o $@.s $^

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)


profile: $(BIN_DIR) clean $(SRCS)
	$(CXX) $(OPT) $(PRF) -o $(TARGET) $(SRCS) && $(TARGET) && \
		gprof $(TARGET) gmon.out -I $(SRC_DIR) \
			--flat-profile --table-length 30

.PHONY: clean
clean:
	rm -rf $(OBJ_DIR)/* $(TARGET) vec.* gmon.*