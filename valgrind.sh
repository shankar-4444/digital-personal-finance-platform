#!/usr/bin/env bash
set -eu

cd "$(dirname "$0")"

command -v gcc >/dev/null 2>&1 || { echo "Error: gcc is required."; exit 1; }
command -v valgrind >/dev/null 2>&1 || { echo "Error: valgrind is not installed. Run this script in Linux or WSL."; exit 1; }

mkdir -p build/valgrind-data

gcc -std=c11 -g -O0 -Wall -Wextra -Iinclude src/main.c src/transaction.c src/report.c src/budget.c -o build/finance_valgrind -lm

cd build/valgrind-data
printf '11\n' | valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --errors-for-leak-kinds=all --error-exitcode=1 ../finance_valgrind
