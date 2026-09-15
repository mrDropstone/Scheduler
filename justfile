default:
    @just run

run:
    @clang++ main.cpp -std=c++23 && ./a.out
