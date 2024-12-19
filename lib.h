#ifndef LIB_H
#define LIB_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define MAX(A, B)      ((A) < (B) ? (B) : (A))
#define MIN(A, B)      ((A) > (B) ? (A) : (B))
#define MEMORY_DEFAULT 30000

bool usable_character(char c);
char *fread_all(FILE *fp);
void print_error(const char *handle, size_t row, size_t column,
                 const char *reason);

typedef struct Buffer
{
  const char *name;
  u64 size;
  u8 data[];
} buffer_t;

buffer_t *buffer_init_str(const char *name, const char *str, size_t str_size);
#endif
