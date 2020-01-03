#ifdef __cplusplus
extern "C" {
#endif

char *strdup (const char *s);

static char *_strdup (const char *s)
{
    return strdup(s);
}

size_t strnlen (const char *s, size_t maxlen);

#define _stricmp strcmp
int stricmp (const char *s1, const char *s2);
#define _strnicmp strnicmp
int strnicmp (const char *s1, const char *s2, size_t n);

#ifdef __cplusplus
}
#endif
