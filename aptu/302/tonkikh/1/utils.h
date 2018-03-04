#ifndef FIRST_ASSIGNMENT_UTILS_H
#define FIRST_ASSIGNMENT_UTILS_H

#include <assert.h>
#include <stdio.h>

inline static void assert_zero(int value) {
  (void) value;  // suppress unused variable warning in release
  assert(value == 0);
}

#ifdef LOG_ENABLE
  #define LOG(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#else
  #define LOG(...)
#endif

void check_err(int error_code, const char* error_message);

#endif  // FIRST_ASSIGNMENT_UTILS_H
