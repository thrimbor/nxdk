#include "nunit.h"
#include <string.h>

TEST(Exceptions, int_throw_simple)
{
    int v_result = 0;
    try {
        throw 1337;
        ASSERT_NOREACH();
    } catch (int v) {
        v_result = v;
    } catch (...) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, int_throw_simple2)
{
    int v_result = 0;
    try {
        throw 1337;
        ASSERT_NOREACH();
    } catch (int v) {
        v_result = v;
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, int_throw_simple3)
{
    int v_result = 0;
    try {
        throw 1337;
        ASSERT_NOREACH();
    } catch (void *v) {
        ASSERT_NOREACH();
    } catch (int v) {
        v_result = v;
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, int_throw_nested)
{
    int v_result = 0;
    try {
        try {
            throw 1337;
            ASSERT_NOREACH();
        } catch (char[3]) {
            ASSERT_NOREACH();
        }
    } catch (int v) {
        v_result = v;
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, int_throw_constcatch)
{
    int v_result = 0;
    try {
        try {
            throw 1337;
            ASSERT_NOREACH();
        } catch (char[3]) {
            ASSERT_NOREACH();
        }
    } catch (const int v) {
        v_result = v;
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, int_throw_constthrow_simplecatch)
{
    int v_result = 0;
    try {
        try {
            throw (const int)1337;
            ASSERT_NOREACH();
        } catch (char[3]) {
            ASSERT_NOREACH();
        }
    } catch (int v) {
        v_result = v;
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, int_throw_refcatch)
{
    int v_result = 0;
    try {
        try {
            throw 1337;
            ASSERT_NOREACH();
        } catch (char[3]) {
            ASSERT_NOREACH();
        }
    } catch (int &v) {
        v_result = v;
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, int_throw_refcatch_overwrite_check)
{
    auto tf  = []() {
        volatile char tbuf[4096];
        memset((void *)tbuf, 0xBF, 4096);
    };
    int v_result = 0;
    try {
        try {
            throw 1337;
            ASSERT_NOREACH();
        } catch (char[3]) {
            ASSERT_NOREACH();
        }
    } catch (int &v) {
        tf();
        v_result = v;
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, int_throw_volatile)
{
    volatile int v_result = 0;
    try {
        throw 1337;
        ASSERT_NOREACH();
    } catch (int v) {
        v_result = v;
    } catch (...) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, int_throw_simple_catch_volatile)
{
    int v_result = 0;
    try {
        throw 1337;
        ASSERT_NOREACH();
    } catch (volatile int v) {
        v_result = v;
    } catch (...) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(v_result, 1337);
}
