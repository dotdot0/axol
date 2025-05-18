CC:=clang++
SRC_DIR:=src
C:=axol
TEST_DIR:=test

SRC_FILES:=$(wildcard $(SRC_DIR)/*.cpp)

LDFLAGS := $(shell llvm-config --ldflags --libs)

all:
	$(CC) $(SRC_FILES) $(LDFLAGS) -o $(C)

run:
	./$(C) $(TEST_DIR)/main.ax

clean:
	rm -rf axol
	rm -rf a.out