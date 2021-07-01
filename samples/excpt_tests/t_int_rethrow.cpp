#include "nunit.h"

TEST(Exceptions, int_rethrow_simple)
{
    int v_result = 0;
    try {
        try {
            throw 1337;
            ASSERT_NOREACH();
        } catch (int i) {
            throw;
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int v) {
        v_result = v;
    } catch (...) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, int_rethrow_from_generic_catch)
{
    int v_result = 0;
    try {
        try {
            throw 1337;
            ASSERT_NOREACH();
        } catch (...) {
            throw;
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int v) {
        v_result = v;
    } catch (...) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, int_rethrow_throw_new_copy)
{
    int v_result = 0;
    try {
        try {
            throw 1337;
            ASSERT_NOREACH();
        } catch (int i) {
            throw i;
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int v) {
        v_result = v;
    } catch (...) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(v_result, 1337);
}

