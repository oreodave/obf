#ifndef LIB_H
#define LIB_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX(a, b) (a > b ? a : b)

bool usable_character(char c);
char *fread_all(FILE *fp);
void print_error(const char *handle, size_t row, size_t column,
                 const char *reason);

typedef struct Buffer
{
  const char *name;
  size_t size;
  char data[];
} buffer_t;

buffer_t *buffer_init_str(const char *name, const char *str, size_t str_size);
#endif
