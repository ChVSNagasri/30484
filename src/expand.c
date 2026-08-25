#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expand.h"

#define EXPANDED_ARG_LEN 1024

/*
 * Expand environment variables in a single argument.
 *
 * Example:
 *     "$HOME"
 *     "$USER"
 *     "hello_$USER"
 */
static void expand_string(const char *input,
                          char *output,
                          size_t output_size)
{
    size_t i = 0;
    size_t j = 0;

    if (input == NULL || output == NULL || output_size == 0)
        return;

    while (input[i] != '\0' && j < output_size - 1)
    {
        /*
         * Environment variable starts with $
         */
        if (input[i] == '$')
        {
            i++;

            char variable[128];
            size_t k = 0;

            /*
             * Read variable name:
             * $HOME
             * $USER
             * $PATH
             */
            while (input[i] != '\0' &&
                   (input[i] == '_' ||
                    (input[i] >= 'A' && input[i] <= 'Z') ||
                    (input[i] >= 'a' && input[i] <= 'z') ||
                    (input[i] >= '0' && input[i] <= '9')) &&
                   k < sizeof(variable) - 1)
            {
                variable[k++] = input[i++];
            }

            variable[k] = '\0';

            /*
             * If there was no variable name,
             * keep the '$'.
             */
            if (k == 0)
            {
                output[j++] = '$';
                continue;
            }

            const char *value = getenv(variable);

            /*
             * Undefined environment variable -> empty string
             */
            if (value == NULL)
                continue;

            size_t value_len = strlen(value);

            if (j + value_len >= output_size)
                value_len = output_size - j - 1;

            memcpy(output + j, value, value_len);
            j += value_len;
        }
        else
        {
            output[j++] = input[i++];
        }
    }

    output[j] = '\0';
}


/*
 * Expand environment variables in every argv argument
 * of every command in the pipeline.
 */
void expand_variables(pipeline_t *pipeline)
{
    if (pipeline == NULL)
        return;

    for (int i = 0; i < pipeline->command_count; i++)
    {
        command_t *cmd = &pipeline->commands[i];

        for (int j = 0; j < cmd->argc; j++)
        {
            if (cmd->argv[j] == NULL)
                continue;

            char expanded[EXPANDED_ARG_LEN];

            expand_string(cmd->argv[j],
                          expanded,
                          sizeof(expanded));

            /*
             * Copy expanded value back into the token buffer.
             *
             * argv[j] currently points to token text,
             * which has MAX_TOKEN_LEN bytes.
             */
            strncpy(cmd->argv[j],
                    expanded,
                    MAX_TOKEN_LEN - 1);

            cmd->argv[j][MAX_TOKEN_LEN - 1] = '\0';
        }
    }
}
