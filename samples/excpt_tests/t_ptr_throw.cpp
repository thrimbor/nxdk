#include "nunit.h"

TEST(Exceptions, ptr_int_throw_void_catch)
{
    int t1=0;
    try {
        try {
            throw &t1;
            ASSERT_NOREACH();
        } catch (int i) {
            ASSERT_NOREACH();
        }
        ASSERT_NOREACH();
    } catch (int i) {
        ASSERT_NOREACH();
    } catch (void *e) {
        *(int *)e = 1337;
    } catch (int *e) {
        ASSERT_NOREACH();
    }

    ASSERT_EQ(t1, 1337);
}
