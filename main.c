#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define safeFree(ptr)                                                          \
  do {                                                                         \
    free(ptr);                                                                 \
    ptr = NULL;                                                                \
  } while (0)

typedef struct {
  bool support_zero;
} Config;

typedef struct {
  size_t bitlen;
  size_t seqwidth;
  size_t depth;
  bool *visited;
  uint8_t *magic;
} State;

static bool try_next(State *state, uintmax_t curmagic);
static bool search_debruijn_seq(State *state, uintmax_t curmagic);
static void print_countr_zero_implementation(FILE *fp, const State *state,
                                             const Config *config);
static size_t into_size_t(uint8_t *seq, size_t len);

size_t log2_for_pow2(size_t n) {
  size_t log2 = 0;
  while ((n & 0x1) == 0x0) {
    log2++;
    n >>= 1;
  }
  return log2;
}

size_t ceil_pow2(size_t n) {
  --n;
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
#if __SIZE_OF_SIZE_T__ >= 4
  n |= n >> 16;
#if __SIZE_OF_SIZE_T__ >= 8
  n |= n >> 32;
#endif
#endif
  return n + 1;
}

bool try_next(State *state, uintmax_t curmagic) {
  if (curmagic >= state->bitlen || state->visited[curmagic]) {
    return false;
  }
  if (state->depth >= state->bitlen) {
    return true;
  }

  state->visited[curmagic] = true;
  if (search_debruijn_seq(state, curmagic)) {
    return true;
  }
  state->visited[curmagic] = false;
  return false;
}

bool search_debruijn_seq(State *state, uintmax_t curmagic) {
  size_t idx = state->bitlen - state->depth - 1;

  state->depth++;
  state->magic[idx] = 0;
  if (try_next(state, curmagic >> 1)) {
    return true;
  }

  state->magic[idx] = 1;
  if (try_next(state, (1ull << (state->seqwidth - 1)) | (curmagic >> 1))) {
    return true;
  }
  state->depth--;

  return false;
}

int gen_contr_zero(size_t bitlen) {
  size_t pow2_bitlen = ceil_pow2(bitlen);
  size_t seqwidth = log2_for_pow2(pow2_bitlen);
  State state = {
      bitlen,
      seqwidth,
      1, // Start from depth 1.
      (bool *)calloc(bitlen, sizeof(bool)),
      (uint8_t *)calloc(bitlen, sizeof(uint8_t)),
  };
  Config config = {
      true,
  };
  state.visited[0] = true;
  if (!search_debruijn_seq(&state, 0)) {
    fputs("Failed to search De Bruijn sequence.\n", stderr);
    return -1;
  }

  if (pow2_bitlen < 8) {
    pow2_bitlen = 8;
  }

  print_countr_zero_implementation(stdout, &state, &config);
  config.support_zero = false;
  print_countr_zero_implementation(stdout, &state, &config);
  safeFree(state.magic);
  safeFree(state.visited);

  return 0;
}

void print_countr_zero_implementation(FILE *fp, const State *state,
                                      const Config *config) {
  size_t bitlen = state->bitlen <= 8 ? 8 : ceil_pow2(state->bitlen);
  size_t table_size = bitlen + (config->support_zero ? 1 : 0);
  char *hexmagic = (char *)calloc(bitlen / 4, sizeof(char));
  size_t *hash2zeros = (size_t *)calloc(table_size, sizeof(size_t));
  char *fn_suffix = NULL;

  // Get string notation of the magic number.
  {
    char *buf = hexmagic;
    char fmt[10] = {};
    for (uint8_t *start = state->magic, *end = state->magic + state->bitlen;
         start != end;) {
      size_t len = (size_t)(end - start);
      int printlen = 0;
      if (len > sizeof(size_t) * 8) {
        len = sizeof(size_t) * 8;
      }
      printlen = len / 4;
      sprintf(buf, "%0*zx", printlen, into_size_t(start, len));
      buf += printlen;
      start += len;
    }
  }

  // Build hash2zeros array.
  if (config->support_zero) {
    hash2zeros[0] = state->bitlen;
  }
  for (int i = 0; i <= state->bitlen; ++i) {
    uint8_t *start = state->magic + i - 1;
    size_t offset_support_zero = config->support_zero ? 1 : 0;
    size_t len, hash;
    len = state->seqwidth;
    if (state->bitlen <= i + len) {
      len = state->bitlen - i;
    }
    hash = into_size_t(start, len);
    hash <<= state->seqwidth - len;
    hash2zeros[hash + offset_support_zero] = i - 1;
  }

  fn_suffix = config->support_zero ? "" : "_nonzero";
  fprintf(fp, "size_t countr_zero_uint%zd_t%s(uint%zd_t n) {\n", bitlen,
          fn_suffix, bitlen);
  fprintf(fp, "    static const size_t hash2zeros[%zd] = {\n", table_size);
  fprintf(fp, "        ");
  for (int i = 0; i < table_size; ++i) {
    fprintf(fp, "%zd,%c", hash2zeros[i], (i + 1) < table_size ? ' ' : '\n');
  }
  fprintf(fp, "    };\n");
  fprintf(fp, "    size_t hash;\n");
  fprintf(fp, "    n = n - (n & (n - 1));\n");
  fprintf(fp, "    hash = ((uint%zd_t)(n * UINT%zd_C(0x%s)) >> %zd)", bitlen,
          bitlen, hexmagic, bitlen - state->seqwidth);
  if (config->support_zero) {
    fprintf(fp, " + ((uint%zd_t)(~(n - 1) | n) >> %zd);\n", bitlen, bitlen - 1);
  } else {
    fprintf(fp, ";\n");
  }
  fprintf(fp, "    return hash2zeros[hash];\n");
  fprintf(fp, "}\n");
  safeFree(hexmagic);
  safeFree(hash2zeros);
}

size_t into_size_t(uint8_t *start, size_t len) {
  size_t result = 0;

  assert(len <= sizeof(size_t) * 8);

  for (int i = 0; i < len; ++i) {
    result = result << 1 | start[i];
  }

  return result;
}

int main(int argc, char **argv) {
  size_t bitlen = 0;
  char *endptr = NULL;
  if (argc != 2) {
    fprintf(stderr, "Usage: ./gen-contr-zero-c <bit-length>\n");
    return -1;
  }
  bitlen = strtoul(argv[1], &endptr, 10);
  if (argv[1] == endptr) {
    fprintf(stderr, "strtoul: %s\n", argv[1]);
    return -1;
  }

  return gen_contr_zero(bitlen);
}
