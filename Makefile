CC:=clang++
SRC_DIR:=src
C:=axol
TEST_DIR:=test

SRC_FILES:=$(wildcard $(SRC_DIR)/*.cpp)

all:
	$(CC) $(SRC_FILES) -o $(C)


clean:
	rm -rf axol