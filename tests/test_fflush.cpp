#include <stdio.h>
#include <iostream>

int main()
{
    std::cout << "Print in " << "two parts." << '\n';
    std::cerr << "std::cerr in " << "two parts." << '\n';

    printf("hello!");
    fflush(stdout);
    fprintf(stderr, "hello from stderr too!");
    fflush(stderr);
}
