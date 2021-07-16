#include <cstdarg>
#include <cstdio>

#include "nunit.h"

void nunit_print (const char *str, ...)
{
    char outstr[4096];
    va_list args;
    va_start(args, str);
    vsprintf(outstr, str, args);
    printf("%s", outstr);
    va_end(args);
}

NUNIT_DEFINE_GLOBALS

// TODO: No volatile tests yet!
// TODO: Non-rethrowing nested throw test!
// TODO: Test whether re-throwing can be caught in the same try block
// TODO: mut -> const test
// TODO: volatile -> non-volatile test
// TODO: SEH with catch(...) ?
// TODO: Non-virtual inheritance with different offsets

int main ()
{
    nunit_run_tests();

    return 0;
}
