#include "nunit.h"

class ThrowTestClass {
    public:
    int v1;
    int v2;
    int v3;
    ThrowTestClass(int v1, int v2, int v3) : v1(v1), v2(v2), v3(v3) {};
};

TEST(Exceptions, obj_simple_throw_valuecatch)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw ThrowTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (ThrowTestClass e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_simple_throw_refcatch)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw ThrowTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (ThrowTestClass &e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_simple_throw_valuecatch_const)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw ThrowTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (const ThrowTestClass e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_simple_throw_refcatch_const)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw ThrowTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (const ThrowTestClass &e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_ptr_throw_valuecatch)
{
    ThrowTestClass t(23, 1337, 0xdeadbeef);
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw &t;
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (ThrowTestClass *e) {
        t1 = e->v1;
        t2 = e->v2;
        t3 = e->v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_ptr_throw_valuecatch_const)
{
    ThrowTestClass t(23, 1337, 0xdeadbeef);
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw &t;
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (const ThrowTestClass *e) {
        t1 = e->v1;
        t2 = e->v2;
        t3 = e->v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_ptr_throw_valuecatch_constconst)
{
    ThrowTestClass t(23, 1337, 0xdeadbeef);
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw &t;
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (const ThrowTestClass * const e) {
        t1 = e->v1;
        t2 = e->v2;
        t3 = e->v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_const_throw_valuecatch)
{
    const ThrowTestClass t(23, 1337, 0xdeadbeef);
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw t;
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (ThrowTestClass e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

// TODO: Check if this is correct on Windows
TEST(Exceptions, obj_const_throw_refcatch)
{
    const ThrowTestClass t(23, 1337, 0xdeadbeef);
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw t;
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (ThrowTestClass &e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}


// TODO: Can we throw refs?
// TODO: inheritance tests
// TODO: Check number of copies/deletes?
