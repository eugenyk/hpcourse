#ifndef FIRST_ASSIGNMENT_UTILS_H
#define FIRST_ASSIGNMENT_UTILS_H

#include <assert.h>
#include <stdio.h>

#define ASSERT_ZERO(cmd) do { \
    if ((cmd) != 0) { \
      fprintf(stderr, "%s", "Assertion error: ("#cmd") is not equal to zero"); \
      assert(false && "ASSERT_ZERO failed"); \
    } \
  } while (0)

#define MAY_BE_NONZERO(cmd) (cmd)

#ifdef LOG_ENABLE
  #define LOG(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#else
  #define LOG(...)
#endif

void check_err(int error_code, const char* error_message);

#endif  // FIRST_ASSIGNMENT_UTILS_H
