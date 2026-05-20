#include "acutest.h"
#include <stdint.h>
#include <string.h>

extern size_t countr_zero_uint8_t(uint8_t);
extern size_t countr_zero_uint8_t_nonzero(uint8_t);

void test_countr_zero(void) {
  TEST_CHECK(countr_zero_uint8_t(0) == sizeof(uint8_t) * 8);
  for (int i = 0; i < sizeof(uint8_t) * 8; ++i) {
    size_t ctz = countr_zero_uint8_t((uint8_t)1 << i);
    TEST_CHECK_(ctz == i,
                "countr_zero_uint8_t(1 << %d): expected %d but got %zu", i, i,
                ctz);
  }
}

void test_countr_zero_nonzero(void) {
  for (int i = 0; i < sizeof(uint8_t) * 8; ++i) {
    size_t ctz = countr_zero_uint8_t_nonzero((uint8_t)1 << i);
    TEST_CHECK_(ctz == i,
                "countr_zero_uint8_t_nonzero(1 << %d): expected %d but got %zu",
                i, i, ctz);
  }
}

TEST_LIST = {
    {"countr_zero_uint8_t", test_countr_zero},
    {"countr_zero_uint8_t_nonzero", test_countr_zero_nonzero},
    {},
};
