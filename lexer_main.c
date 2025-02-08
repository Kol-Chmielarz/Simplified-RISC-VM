#include <stdio.h>
#include "lexer.h"

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Error: Invalid number of arguments\n");
        return 1;
    }

    // Initialize the lexer with the input file
    lexer_init(argv[1]);

    // Run the lexer and print the tokens
    lexer_output();

    return 0;
}