#include <stdio.h>
#include <stdlib.h>

#include <readline/doc/readline.h>
#include <readline/doc/history.h>

int main()
{
    printf( "%s\n", readline( "test> " ) );
    return 0;
}