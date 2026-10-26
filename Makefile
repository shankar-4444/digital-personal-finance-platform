CC = gcc
CXX = g++
CFLAGS = -std=c11 -g -O0 -Wall -Wextra -Iinclude
CXXFLAGS = -std=c++17 -g -O0 -Wall -Wextra -Iinclude
SRC = src/main.c src/transaction.c src/report.c src/budget.c
TEST_SRC = test/test_finance.cpp
OUT = build/finance
TEST_OUT = build/finance_tests
OBJ = build/transaction.o build/report.o build/budget.o

.PHONY: all clean test valgrind cppcheck misra check

all: $(OUT)

$(OUT): $(SRC) include/finance.h
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) -lm

build/transaction.o: src/transaction.c include/finance.h
	mkdir -p build
	$(CC) $(CFLAGS) -c src/transaction.c -o $@

build/report.o: src/report.c include/finance.h
	mkdir -p build
	$(CC) $(CFLAGS) -c src/report.c -o $@

build/budget.o: src/budget.c include/finance.h
	mkdir -p build
	$(CC) $(CFLAGS) -c src/budget.c -o $@

$(TEST_OUT): $(TEST_SRC) $(OBJ) include/finance.h
	$(CXX) $(CXXFLAGS) $(TEST_SRC) $(OBJ) -lgtest -lgtest_main -pthread -lm -o $(TEST_OUT)

test: $(TEST_OUT)
	mkdir -p build/gtest-data
	cd build/gtest-data && ../finance_tests

valgrind: $(OUT)
	command -v valgrind >/dev/null 2>&1 || { echo "Error: valgrind is required. Install it with: sudo apt install valgrind"; exit 1; }
	mkdir -p build/valgrind-data
	cd build/valgrind-data && printf '11\n' | valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --errors-for-leak-kinds=all --error-exitcode=1 ../finance

cppcheck:
	command -v cppcheck >/dev/null 2>&1 || { echo "Error: cppcheck is required. Install it with: sudo apt install cppcheck"; exit 1; }
	cppcheck --enable=warning,performance,portability --error-exitcode=1 --std=c11 -I include src

misra:
	command -v cppcheck >/dev/null 2>&1 || { echo "Error: cppcheck is required. Install it with: sudo apt install cppcheck"; exit 1; }
	cppcheck --addon=/usr/lib/x86_64-linux-gnu/cppcheck/addons/misra.py --error-exitcode=1 --suppress=misra-config:src/transaction.c:528 --std=c11 -I include src

check: test cppcheck misra valgrind

clean:
	rm -f $(OUT) $(TEST_OUT) $(OBJ)
