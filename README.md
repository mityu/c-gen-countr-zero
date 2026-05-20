# Generator of `countr_zero` function implementation in C

A small C program to generate a small C function to count the consective 0 bits from the least significant bit in a given integer number (a.k.a C++'s `std::countr_zero()` or GCC's `__builtin_ctz()` family).

## Build requirements

GCC or Clang (or any C compiler that supports C11 and provides `__SIZE_OF_SIZE_T__` macro).

## How to build

`$ make`

## How to run

`./gen-countr-zero-c <bit-length>`

This will output an alternative implementation in C for the C++'s `std::countr_zero()` where numbers between `0` to `2^<bits-length> - 1` are supported.

### Example

`$ ./gen-countr-zero-c 16`

```c
size_t countr_zero_uint16_t(uint16_t n) {
    static const size_t positions[16] = {
        16, 0, 12, 1, 13, 7, 9, 2, 14, 11, 6, 8, 10, 5, 4, 3,
    };
    n = n - (n & (n - 1));
    return positions[(uint16_t)(n * UINT16_C(0x1eb2)) >> 12];
}
```

## Example Usage

A simple C program to count and print the consective 0 bits for `2^n` numbers where `n` is `0 <= n < 16`.

```c
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>

// This function is the output of `./gen-countr-zero-c 16`.
size_t countr_zero_uint16_t(uint16_t n) {
    static const size_t positions[16] = {
        16, 0, 12, 1, 13, 7, 9, 2, 14, 11, 6, 8, 10, 5, 4, 3,
    };
    n = n - (n & (n - 1));
    return positions[(uint16_t)(n * UINT16_C(0x1eb2)) >> 12];
}

int main(void) {
  printf("countr_zero_uint16_t(0): %zd\n", countr_zero_uint16_t(0));
  for (int i = 0; i <= 16; ++i) {
    printf("countr_zero_uint16_t(1 << %d): %zd\n", i, countr_zero_uint16_t(1 << i));
  }
}
```

This will output this.

```
countr_zero_uint16_t(0): 16
countr_zero_uint16_t(1 << 0): 0
countr_zero_uint16_t(1 << 1): 1
countr_zero_uint16_t(1 << 2): 2
countr_zero_uint16_t(1 << 3): 3
countr_zero_uint16_t(1 << 4): 4
countr_zero_uint16_t(1 << 5): 5
countr_zero_uint16_t(1 << 6): 6
countr_zero_uint16_t(1 << 7): 7
countr_zero_uint16_t(1 << 8): 8
countr_zero_uint16_t(1 << 9): 9
countr_zero_uint16_t(1 << 10): 10
countr_zero_uint16_t(1 << 11): 11
countr_zero_uint16_t(1 << 12): 12
countr_zero_uint16_t(1 << 13): 13
countr_zero_uint16_t(1 << 14): 14
countr_zero_uint16_t(1 << 15): 16
countr_zero_uint16_t(1 << 16): 16
```

## License

This project is the MIT license except for the `test/acutest.h`.
The `test/acutest.h` is just a copy from https://github.com/mity/acutest repository, which distributed under the MIT license.  See [LICENSE-acutest](./LICENSE-acutest) for the details.
