#pragma once

// The one check macro of the host tests. A failed check prints its place
// and counts as a failure. main() returns the count as exit status.

#include <cstdio>

inline int failures = 0;

#define CHECK(cond)                                                  \
   do {                                                              \
      if(!(cond)) {                                                  \
         std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
         ++failures;                                                 \
      }                                                              \
   } while(0)
