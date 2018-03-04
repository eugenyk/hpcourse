#include <stdlib.h>
#include <errno.h>
#include "utils.h"

void check_err(int error_code, const char* error_message) {
  if (error_code != 0) {
    errno = error_code;
    perror(error_message);
    exit(error_code);
  }
}
