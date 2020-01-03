#include <string.h>
#include <stdlib.h>

char *strdup (const char *s)
{
    if (s == NULL) {
        return NULL;
    }

    char *new_s = malloc(strlen(s) + 1);
    if (new_s != NULL) {
        strcpy(new_s, s);
    }

    return new_s;
}

size_t strnlen (const char *s, size_t maxlen)
{
    size_t len;

    for (len = 0; len < maxlen; len++, s++) {
        if (!*s) {
            break;
        }
    }

    return len;
}
