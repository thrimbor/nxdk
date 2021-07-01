#include "nunit.h"

class BaseTestClass {
    public:
    int t1;
    //BaseTestClass(int v1, int v2, int v3) : v1(v1), v2(v2), v3(v3) {};

    virtual int get_v1() const {return 11;};
    virtual int get_v2() const {return 22;};
    virtual int get_v3() const {return 33;};
};

class SubTestClass : public BaseTestClass {
    public:
    int v1;
    int v2;
    int v3;
    SubTestClass(int v1, int v2, int v3) : v1(v1), v2(v2), v3(v3) {};
    int get_v1() const {return v1;};
    int get_v2() const {return v2;};
    int get_v3() const {return v3;};
};

class SubClassVirt : public BaseTestClass {
    int dummy;
};

class SubClass2 : public BaseTestClass {
    public:
    virtual int get_v1() const {return 4;};
    virtual int get_v2() const {return 5;};
    virtual int get_v3() const {return 6;};
};

TEST(Exceptions, obj_virt_throw_valuecatch)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw SubTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (SubClassVirt i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (SubClass2 i) {
        ASSERT_NOREACH();
    } catch (SubTestClass e) {
        t1 = e.get_v1();
        t2 = e.get_v2();
        t3 = e.get_v3();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_virt_throw_refcatch)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw SubTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (SubClassVirt &i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (SubClass2 &i) {
        ASSERT_NOREACH();
    } catch (SubTestClass &e) {
        t1 = e.get_v1();
        t2 = e.get_v2();
        t3 = e.get_v3();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_virt_throw_valuecatch_const)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw SubTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (const SubClassVirt i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (const SubClass2 i) {
        ASSERT_NOREACH();
    } catch (const SubTestClass e) {
        t1 = e.get_v1();
        t2 = e.get_v2();
        t3 = e.get_v3();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_virt_throw_refcatch_const)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw SubTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (const SubClassVirt &i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (const SubClass2 &i) {
        ASSERT_NOREACH();
    } catch (const SubTestClass &e) {
        t1 = e.get_v1();
        t2 = e.get_v2();
        t3 = e.get_v3();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_virt_throw_valuecatch_base)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw SubTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (SubClassVirt i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (SubClass2 i) {
        ASSERT_NOREACH();
    } catch (BaseTestClass e) {
        t1 = e.get_v1();
        t2 = e.get_v2();
        t3 = e.get_v3();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_virt_throw_refcatch_base)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw SubTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (SubClassVirt &i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (SubClass2 &i) {
        ASSERT_NOREACH();
    } catch (BaseTestClass &e) {
        t1 = e.get_v1();
        t2 = e.get_v2();
        t3 = e.get_v3();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_virt_throw_valuecatch_base_const)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw SubTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (const SubClassVirt i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (const SubClass2 i) {
        ASSERT_NOREACH();
    } catch (const BaseTestClass e) {
        t1 = e.get_v1();
        t2 = e.get_v2();
        t3 = e.get_v3();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_virt_throw_refcatch_base_const)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw SubTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (const SubClassVirt &i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (const SubClass2 &i) {
        ASSERT_NOREACH();
    } catch (const BaseTestClass &e) {
        t1 = e.get_v1();
        t2 = e.get_v2();
        t3 = e.get_v3();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_virt_throw_valuecatch_rethrow)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw SubTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (BaseTestClass i) {
            throw;
        }
        ASSERT_NOREACH();
    } catch (SubClass2 i) {
        ASSERT_NOREACH();
    } catch (SubTestClass e) {
        t1 = e.get_v1();
        t2 = e.get_v2();
        t3 = e.get_v3();
    } catch (BaseTestClass e) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}

TEST(Exceptions, obj_virt_throw_valuecatch_rethrow_copy)
{
    int t1=0, t2=0, t3=0;
    try {
        try {
            throw SubTestClass(23, 1337, 0xdeadbeef);
            ASSERT_NOREACH();
        } catch (BaseTestClass i) {
            throw i;
        }
        ASSERT_NOREACH();
    } catch (SubClass2 i) {
        ASSERT_NOREACH();
    } catch (SubTestClass e) {
        ASSERT_NOREACH();
    } catch (BaseTestClass e) {
        t1 = e.get_v1();
        t2 = e.get_v2();
        t3 = e.get_v3();
    }

    ASSERT_EQ(t1, 23);
    ASSERT_EQ(t2, 1337);
    ASSERT_EQ(t3, 0xdeadbeef);
}
