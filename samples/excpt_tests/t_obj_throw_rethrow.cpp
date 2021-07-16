#include "nunit.h"

class ThrowTestClassR {
    public:
    int v1;
    int v2;
    int v3;
    ThrowTestClassR(int v1, int v2, int v3) : v1(v1), v2(v2), v3(v3) {};
};

TEST(Exceptions, obj_simple_rethrow)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw ThrowTestClassR(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (ThrowTestClassR e) {
            throw;
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (ThrowTestClassR e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_simple_rethrow_copy)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw ThrowTestClassR(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (ThrowTestClassR e) {
            throw e;
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (ThrowTestClassR e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_simple_nested_throw)
{
    int ti=0;
    try {
        try {
            throw ThrowTestClassR(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (ThrowTestClassR e) {
            throw 5;
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ti = i;
    } catch (ThrowTestClassR e) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(ti, 5);
}

TEST(Exceptions, obj_simple_rethrow_sametryblock)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw ThrowTestClassR(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (ThrowTestClassR e) {
            throw;
        } catch (ThrowTestClassR e) {
            ASSERT_NOREACH();
        }
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (ThrowTestClassR e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_simple_rethrow_copy_sametryblock)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw ThrowTestClassR(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (ThrowTestClassR e) {
            throw e;
        } catch (ThrowTestClassR e) {
            ASSERT_NOREACH();
        }
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (ThrowTestClassR e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}
