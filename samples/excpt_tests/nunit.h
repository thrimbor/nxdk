#ifndef NUNIT_NUNIT_H
#define NUNIT_NUNIT_H

#define _CRT_SECURE_NO_WARNINGS

// TODO:
// - possibility to only run specific test

// TODO:
// - Utilize SEH/signals
// 2. Float
// https://randomascii.wordpress.com/2012/02/25/comparing-floating-point-numbers-2012-edition/

#include <stdio.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifndef NUNIT_DISABLE_STRINGS
#include <string.h>
#include <ctype.h>
#endif

#if defined(__cplusplus)
extern "C"
{
#endif

extern int nunit_asserts_failed;
extern int nunit_asserts_passed;
extern int nunit_asserts_total;
extern bool nunit_testfail_flag;

// Provided by the test runner
void nunit_print(const char *str, ...);

typedef struct _nunit_test_t
{
    void (*testfunction)(void);
    const char *name;
} nunit_test_t;

#ifdef _MSC_VER
#define NUNIT_TEST_SECTION_PREFIX \
    __pragma(data_seg(push)) \
    __pragma(section("nunit_tests1$test", read)) \
    __declspec(allocate("nunit_tests1$test"))
#define NUNIT_TEST_SECTION_POSTFIX __pragma(data_seg(pop))
#else
#define NUNIT_TEST_SECTION_PREFIX __attribute__((section("nunit_tests1")))
#define NUNIT_TEST_SECTION_POSTFIX
#endif
#define NUNIT_TEST_STRUCT_NAME(unitname, testname) test_ ## unitname ## _ ## testname ## _ ## t
#define NUNIT_TEST_NAME(unitname, testname) test_ ## unitname ## _ ## testname ## _ ## f

#define TEST(unitname, testname) \
void NUNIT_TEST_NAME(unitname, testname) (void); \
nunit_test_t NUNIT_TEST_STRUCT_NAME(unitname, testname) = { \
    NUNIT_TEST_NAME(unitname, testname), \
    #unitname "/" #testname \
}; \
NUNIT_TEST_SECTION_PREFIX \
nunit_test_t * NUNIT_TEST_STRUCT_NAME(unitname, testname ## _) = &NUNIT_TEST_STRUCT_NAME(unitname, testname); \
NUNIT_TEST_SECTION_POSTFIX \
void NUNIT_TEST_NAME(unitname, testname) (void)

#define ASSERT_OP_GENERIC(fail, term, formatter, termstr, actualstr, expectedstr) \
    do { \
        bool nunit_result_ = !!(term); \
        nunit_asserts_total++; \
        if (nunit_result_) { \
            nunit_asserts_passed++; \
        } else { \
            nunit_testfail_flag = true; \
            nunit_asserts_failed++; \
            nunit_print("[----] %s:%d: Assertion failed: %s\n" \
                        "       Actual: " formatter " Expected: " formatter "\n" \
                        , __FILE__, __LINE__, termstr, \
                        actualstr, expectedstr); \
            if (fail) return; \
        } \
    } while (0)

#ifndef NUNIT_DISABLE_STRINGS
static int nunit_strcasecmp(const char *a, const char *b)
{
    int r;

    if (a == b) return 0;

    while ((r = tolower(*a) - tolower(*b)) == 0) {
        if (*a == '\0')
            break;
        a++;
        b++;
    }

    return r;
}
#endif

#define ASSERT_OP_BINARY(fail, formatter, op, actual, expected) ASSERT_OP_GENERIC(fail, actual op expected, formatter, (#actual " " #op " " #expected), actual, expected)
#define ASSERT_OP_BOOL(fail, actual, expected) ASSERT_OP_GENERIC(fail, expected ? actual : !actual, "%s", #actual, actual ? "TRUE" : "FALSE", expected ? "TRUE" : "FALSE")

#define EXPECT_TRUE(actual) ASSERT_OP_BOOL(false, actual, true)
#define EXPECT_FALSE(actual) ASSERT_OP_BOOL(false, actual, false)
#define EXPECT_EQ(actual, expected) ASSERT_OP_BINARY(false, "%d", ==, actual, expected)
#define EXPECT_NE(actual, expected) ASSERT_OP_BINARY(false, "%d", !=, actual, expected)
#define EXPECT_LT(actual, expected) ASSERT_OP_BINARY(false, "%d", <, actual, expected)
#define EXPECT_LE(actual, expected) ASSERT_OP_BINARY(false, "%d", <=, actual, expected)
#define EXPECT_GT(actual, expected) ASSERT_OP_BINARY(false, "%d", >, actual, expected)
#define EXPECT_GE(actual, expected) ASSERT_OP_BINARY(false, "%d", >=, actual, expected)

#define ASSERT_TRUE(actual) ASSERT_OP_BOOL(true, actual, true)
#define ASSERT_FALSE(actual) ASSERT_OP_BOOL(true, actual, false)
#define ASSERT_EQ(actual, expected) ASSERT_OP_BINARY(true, "%d", ==, actual, expected)
#define ASSERT_NE(actual, expected) ASSERT_OP_BINARY(true, "%d", !=, actual, expected)
#define ASSERT_LT(actual, expected) ASSERT_OP_BINARY(true, "%d", <, actual, expected)
#define ASSERT_LE(actual, expected) ASSERT_OP_BINARY(true, "%d", <=, actual, expected)
#define ASSERT_GT(actual, expected) ASSERT_OP_BINARY(true, "%d", >, actual, expected)
#define ASSERT_GE(actual, expected) ASSERT_OP_BINARY(true, "%d", >=, actual, expected)

#define ASSERT_NOREACH() ASSERT_TRUE(0)

#ifndef NUNIT_DISABLE_STRINGS
#define ASSERT_OP_STR(fail, op, actual, expected) ASSERT_OP_GENERIC(fail, strcmp(actual, expected) op 0, "\"%s\"", (#actual " " #op " " #expected), actual, expected)
#define ASSERT_OP_CISTR(fail, op, actual, expected) ASSERT_OP_GENERIC(fail, nunit_strcasecmp(actual, expected) op 0, "\"%s\"", (#actual " " #op " " #expected), actual, expected)
#define EXPECT_STREQ(actual, expected) ASSERT_OP_STR(false, ==, actual, expected)
#define EXPECT_STRNE(actual, expected) ASSERT_OP_STR(false, !=, actual, expected)
#define EXPECT_STRCASEEQ(actual, expected) ASSERT_OP_CISTR(false, ==, actual, expected)
#define EXPECT_STRCASENE(actual, expected) ASSERT_OP_CISTR(false, !=, actual, expected)
#define ASSERT_STREQ(actual, expected) ASSERT_OP_STR(true, ==, actual, expected)
#define ASSERT_STRNE(actual, expected) ASSERT_OP_STR(true, !=, actual, expected)
#define ASSERT_STRCASEEQ(actual, expected) ASSERT_OP_CISTR(true, ==, actual, expected)
#define ASSERT_STRCASENE(actual, expected) ASSERT_OP_CISTR(true, !=, actual, expected)
#endif

#define EXPECT_FLOAT_EQ(actual, expected)
#define EXPECT_DOUBLE_EQ(actual, expected)
#define EXPECT_NEAR(actual, expected, error)
#define ASSERT_FLOAT_EQ(actual, expected)
#define ASSERT_DOUBLE_EQ(actual, expected)
#define ASSERT_NEAR(actual, expected, error)

extern nunit_test_t *__start_nunit_tests1;
extern nunit_test_t *__stop_nunit_tests1;

static size_t nunit_count_tests() {
    size_t count = 0;
    for (nunit_test_t **iter = &__start_nunit_tests1; iter < &__stop_nunit_tests1; iter++) {
        if (*iter == NULL) continue;
        count++;
    }

    return count;
}

#ifdef _MSC_VER
#define NUNIT_DEFINE_GLOBALS_SECTION                        \
    __pragma(comment(linker, "/merge:nunit_tests1=.rdata")) \
    __pragma(data_seg(push))                                \
    __pragma(section("nunit_tests1$aaa", read))             \
    __declspec(allocate("nunit_tests1$aaa"))                \
    nunit_test_t *__start_nunit_tests1 = NULL;              \
    __pragma(section("nunit_tests1$zzz", read))             \
    __declspec(allocate("nunit_tests1$zzz"))                \
    nunit_test_t *__stop_nunit_tests1 = NULL;               \
    __pragma(data_seg(pop))
#else
#define NUNIT_DEFINE_GLOBALS_SECTION
#endif

#define NUNIT_DEFINE_GLOBALS      \
    bool nunit_testfail_flag;     \
    int nunit_asserts_passed = 0; \
    int nunit_asserts_failed = 0; \
    int nunit_asserts_total = 0;  \
    NUNIT_DEFINE_GLOBALS_SECTION

static int nunit_run_tests()
{
    int nunit_tests_run = 0;
    int nunit_tests_failed = 0;
    size_t tests_total = nunit_count_tests();
    size_t counter = 0;
    for (nunit_test_t **iter = &__start_nunit_tests1; iter < &__stop_nunit_tests1; iter++) {
        if (*iter == NULL) continue;
        counter++;
        nunit_print("[TEST] [%zu/%zu] %s\n", counter, tests_total, (*iter)->name);
        nunit_testfail_flag = false;
        #ifdef _WIN32
        __try {
        #endif
            (*iter)->testfunction();
        #ifdef _WIN32
        } __except (1) {
            nunit_print("Unexpected exception: %08x\n", _exception_code());
            nunit_testfail_flag = true;
        }
        #endif
        nunit_tests_run++;
        if (nunit_testfail_flag) {
            nunit_tests_failed++;
            nunit_print("[FAIL] %s\n", (*iter)->name);
        }
    }

    nunit_print("[====] Tested: %d Passing: %d Failing: %d\n", nunit_tests_run, nunit_tests_run - nunit_tests_failed, nunit_tests_failed);
    nunit_print("[====] Asserts: %d Passing: %d Failing: %d\n", nunit_asserts_total, nunit_asserts_passed, nunit_asserts_failed);
    return 0;
}

#if defined(__cplusplus)
}
#endif

#endif
