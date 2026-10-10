#include <string.h>
#include "accent.h"

const char *get_accent_code(const char *accent)
{
    if (strcmp(accent, "red") == 0)
        return "\033[1;31m";

    if (strcmp(accent, "green") == 0)
        return "\033[1;32m";

    if (strcmp(accent, "yellow") == 0)
        return "\033[1;33m";

    if (strcmp(accent, "blue") == 0)
        return "\033[1;34m";

    if (strcmp(accent, "magenta") == 0)
        return "\033[1;35m";

    if (strcmp(accent, "cyan") == 0)
        return "\033[1;36m";

    if (strcmp(accent, "white") == 0)
        return "\033[1;37m";

    return NULL;
}
