#include "nunit.h"

class ThrowTestBaseClass {
    public:
    int v1;
    int v2;
    int v3;
    ThrowTestBaseClass(int v1, int v2, int v3) : v1(v1), v2(v2), v3(v3) {};
};

class ThrowTestSubClass : public ThrowTestBaseClass {
    public:
        ThrowTestSubClass(int v1, int v2, int v3) : ThrowTestBaseClass(v1,v2,v3) {}
};

TEST(Exceptions, obj_subclass_throw_valuecatch)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw ThrowTestSubClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (ThrowTestBaseClass e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    } catch (ThrowTestSubClass e) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_subclass_throw_refcatch)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw ThrowTestSubClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (ThrowTestBaseClass &e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    } catch (ThrowTestSubClass &e) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_subclass_throw_refcatch_const)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw ThrowTestSubClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (const ThrowTestBaseClass &e) {
        t1 = e.v1;
        t2 = e.v2;
        t3 = e.v3;
    } catch (const ThrowTestSubClass &e) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_subclass_ptr_throw_valuecatch)
{
    ThrowTestSubClass t(23, 1337, 0xdeadbeef);
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
    } catch (ThrowTestBaseClass *e) {
        t1 = e->v1;
        t2 = e->v2;
        t3 = e->v3;
    } catch (ThrowTestSubClass *e) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}
