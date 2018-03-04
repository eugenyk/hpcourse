#ifndef FIRST_ASSIGNMENT_UTILS_H
#define FIRST_ASSIGNMENT_UTILS_H

#include <assert.h>
#include <stdio.h>


#ifdef NDEBUG
  #define ASSERT_ZERO(cmd) do { \
    if ((cmd) != 0) { \
      fprintf(stderr, "%s", "Error: `"#cmd"` is not equal to zero\n"); \
      exit(17); \
    } \
  } while(0)
#else
  #define ASSERT_ZERO(cmd) assert((cmd) == 0)
#endif

#ifdef LOG_ENABLE
  #define LOG(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#else
  #define LOG(...)
#endif

#endif  // FIRST_ASSIGNMENT_UTILS_H
