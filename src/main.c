#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "history.h"
#include "token.h"
#include "lexer.h"

int main(void)
{
    // Display a welcome banner when the shell starts
    printf("=====================================\n");
    printf("          Shellforge\n");
    printf("  A Unix Style Shell written in C\n");
    printf("=====================================\n");

    token_list_t tokens;

    /* Initialize token list */
    token_list_init(&tokens);

    using_history();

    char *line;

    while (1)
    {
        line = readline("shellforge$ ");

        /* Ctrl + D */
        if (line == NULL)
        {
            printf("\nGoodbye!\n");
            break;
        }

        /* Exit */
        if (strcmp(line, "exit") == 0)
        {
            free(line);
            printf("Exiting...\n");
            break;
        }

        /* Empty input */
        if (strlen(line) == 0)
        {
            free(line);
            continue;
        }

        /* History command */
        if (strcmp(line, "history") == 0)
        {
            print_history();
            free(line);
            continue;
        }

        /* Add command to history */
        add_history(line);

        /* Lexical analysis */
        lexer(line, &tokens);

        /* Display tokens */
        token_print(&tokens);

        free(line);
    }

    return 0;
}
