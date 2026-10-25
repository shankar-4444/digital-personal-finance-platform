CC = gcc
CFLAGS = -std=c11 -g -O0 -Wall -Wextra -Iinclude
SRC = src/main.c src/transaction.c src/report.c src/budget.c
OUT = build/finance

all: $(OUT)

$(OUT): $(SRC) include/finance.h
	mkdir -p build
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) -lm

clean:
	rm -f build/finance
