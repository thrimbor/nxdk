#ifdef __cplusplus
extern "C" {
#endif

char *strdup (const char *s);

static char *_strdup (const char *s)
{
    return strdup(s);
}

size_t strnlen (const char *s, size_t maxlen);

#ifdef __cplusplus
}
#endif
