#include "acutest.h"
#include <stdint.h>
#include <string.h>

extern size_t countr_zero_uint16_t(uint16_t);
extern size_t countr_zero_uint16_t_nonzero(uint16_t);

void test_contr_zero(void) {
  TEST_CHECK(countr_zero_uint16_t(0) == sizeof(uint16_t) * 8);
  for (int i = 0; i < sizeof(uint16_t) * 8; ++i) {
    size_t ctz = countr_zero_uint16_t((uint16_t)1 << i);
    TEST_CHECK_(ctz == i,
                "countr_zero_uint16_t(1 << %d): expected %d but got %zu", i, i,
                ctz);
  }
}

void test_contr_zero_nonzero(void) {
  for (int i = 0; i < sizeof(uint16_t) * 8; ++i) {
    size_t ctz = countr_zero_uint16_t_nonzero((uint16_t)1 << i);
    TEST_CHECK_(
        ctz == i,
        "countr_zero_uint16_t_nonzero(1 << %d): expected %d but got %zu", i, i,
        ctz);
  }
}

TEST_LIST = {
    {"contr_zero_uint16_t", test_contr_zero},
    {"contr_zero_uint16_t_nonzero", test_contr_zero_nonzero},
    {},
};
