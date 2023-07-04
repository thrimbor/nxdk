#include "nunit.h"

class CatchallTestClass {
    public:
    int i;
};

TEST(Exceptions, catchall_simple)
{
    int v_result = 0;
    try {
        throw 1337;
        ASSERT_NOREACH();
    } catch (...) {
        v_result = 1337;
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, catchall_rethrow)
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
    } catch (int i) {
        v_result = i;
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, catchall_obj)
{
    int v_result = 0;
    try {
        CatchallTestClass ctc;
        throw ctc;
        ASSERT_NOREACH();
    } catch (...) {
        v_result = 1337;
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, catchall_obj_rethrow)
{
    int v_result = 0;
    try {
        try {
            CatchallTestClass ctc;
            ctc.i = 1337;
            throw ctc;
            ASSERT_NOREACH();
        } catch (...) {
            throw;
            ASSERT_NOREACH();
        }
    } catch (CatchallTestClass c) {
        v_result = c.i;
    }

    ASSERT_EQ(v_result, 1337);
}

TEST(Exceptions, catchall_obj_nestedthrow)
{
    int v_result = 0;
    try {
        try {
            CatchallTestClass ctc;
            ctc.i = 1337;
            throw ctc;
            ASSERT_NOREACH();
        } catch (...) {
            throw 1338;
            ASSERT_NOREACH();
        }
    } catch (int c) {
        v_result = c;
    }

    ASSERT_EQ(v_result, 1338);
}
