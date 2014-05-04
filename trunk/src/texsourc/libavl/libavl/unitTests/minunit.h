// ======================================================================================
// File         : minunit.h
// Author       : Adrien Oliva 
// Last Change  : 01/02/2013 | 10:46:45 AM | Wednesday,January
// Description  : Minimal unit test framework
// ======================================================================================
#ifndef __MINUNIT_H__
#define __MINUNIT_H__

// ========================================================
//  Test framework configuration
// ========================================================

#ifdef WITH_RANDOM
#ifdef __CPLUSPLUS
#include <ctime>
#include <random>
#endif
#endif

// ========================================================
//  Test framework:
//      Each unit test function must be on form
//          char *unit_test();
//      On success, function must return NULL and on error,
//      function must return a string describing the error.
//
//      To help writing test, you can use the "mu_assert"
//      macro to perform a vital test. If test fails, macro
//      return the given error message. If test is ok,
//      program continue without doing anything.
//
//      Once a test function is written, you must call it
//      with macro "mu_run_test" which takes in argument
//      the test function. You can compile all tests in a
//      single function "char *all_test()" which calls all
//      your "mu_run_test". With such a function, you can
//      simply call in your main, the macro "mu_run_all":
//      it runs all your tests, print the number of passed
//      tests, and the error message if a test fails.
//
//      Finally, the last macro provide by minunit is
//      "mu_run_set". This macro is similar to
//      "mu_run_test" but does not increase the number of
//      test run.
// ========================================================

#ifdef __CPLUSPLUS
#define mu_assert(message, test) do {                       \
    if (!(test))                                            \
        return const_cast<char *>(message);                 \
} while (0)
#else
#define mu_assert(message, test) do {                       \
    if (!(test))                                            \
        return (char *) (message);                          \
} while (0)
#endif

#ifdef __CPLUSPLUS
#define mu_try try
#define mu_catch(message) catch (...) {                     \
    std::string error_msg = std::string("Exception in ");   \
    error_msg += std::string(__func__);                     \
    error_msg += std::string(": ");                         \
    error_msg += message;                                   \
    return const_cast<char *>(error_msg.c_str());           \
}
#endif

#define mu_run_test(test) do {                              \
    char *message = test();                                 \
    tests_run++;                                            \
    if (message)                                            \
        return message;                                     \
} while (0)

#define mu_run_set(test_set) do {                           \
    char *message = test_set();                             \
    if (message)                                            \
        return message;                                     \
} while (0)

#define mu_run_all(all_tests) do {                          \
    char *result = all_tests();                             \
    printf("%d test(s) run.\n", tests_run);                 \
    if (result != 0) {                                      \
        fprintf(stdout, "%s\n", result);                    \
        fprintf(stderr, "\e[01;31m");                       \
        fflush(stderr);                                     \
        fprintf(stdout, "TEST FAILED");                     \
        fflush(stdout);                                     \
        fprintf(stderr, "\e[0m");                           \
        fflush(stderr);                                     \
        fprintf(stdout, "\n");                              \
        return -1;                                          \
    } else {                                                \
        fprintf(stderr, "\e[01;32m");                       \
        fflush(stderr);                                     \
        fprintf(stdout, "ALL TESTS PASSED");                \
        fflush(stdout);                                     \
        fprintf(stderr, "\e[0m");                           \
        fflush(stderr);                                     \
        fprintf(stdout, "\n");                              \
        return 0;                                           \
    }                                                       \
} while (0)

extern int tests_run;

// ========================================================
// Usefull function to generate random stuff
// ========================================================

#ifdef __CPLUSPLUS
static void inline get_random_string(std::string &str, std::string &pool, int size) {
    unsigned index = 0;
#ifdef WITH_RANDOM
    std::random_device rd;
    unsigned bytes = 0;
    for (int i = 0; i < size; i++) {
        index = rd() % pool.size();
        if (pool[index] & 0x80) {
            while (((pool[index] & 0xc0) >> 6) != 0x03)
                index--;
            if (((pool[index] & 0xfe) >> 1) == 0x7e)
                bytes = 6;
            else if (((pool[index] & 0xfc) >> 2) == 0x3e)
                bytes = 5;
            else if (((pool[index] & 0xf8) >> 3) == 0x1e)
                bytes = 4;
            else if (((pool[index] & 0xf0) >> 4) == 0x0e)
                bytes = 3;
            else if (((pool[index] & 0xe0) >> 5) == 0x06)
                bytes = 2;
            for (unsigned int j = 0; j < bytes; j++)
                str += pool[index + j];
        } else {
            str += pool[index];
        }
    }
#else
    for (int i = 0; i < size; i++) {
        do {
            str += pool[index % pool.size()];
            index++;
        } while (pool[index % pool.size()] & 0x80);
    }

#endif
}
#endif

#endif
