/* lib.c
 * Created: 2023-09-02
 * Author: Aryadev Chavali
 * Description: General functions used throughout
 */

#include <string.h>

#include "./lib.h"

bool usable_character(char c)
{
  return c == '>' || c == '<' || c == '+' || c == '-' || c == '-' || c == '.' ||
         c == ',' || c == '[' || c == ']';
}

buffer_t *buffer_init_str(const char *name, const char *str, size_t str_size)
{
  buffer_t *buf = malloc(sizeof(*buf) + str_size + 1);
  buf->name     = name;
  buf->size     = str_size;
  if (str)
  {
    memcpy(buf->data, str, str_size);
    buf->data[str_size] = '\0';
  }
  return buf;
}

void print_error(const char *handle, size_t row, size_t column,
                 const char *reason)
{
  fprintf(stderr, "%s:%lu:%lu:%s\n", handle, row, column, reason);
}

char *fread_all(FILE *fp)
{
  const size_t CHUNK_SIZE = 1024, MULT = 2;
  struct
  {
    char *data;
    size_t used, available;
  } buffer = {calloc(CHUNK_SIZE, sizeof(*buffer.data)), 0, CHUNK_SIZE};

  if (!buffer.data)
  {
    print_error(
        "[internal]", 0, 0,
        "ERROR: Out of Memory (could not allocate buffer in fread_all)");
    return NULL;
  }

  size_t acc = 0, bytes_read = 0;
  while ((bytes_read = fread(buffer.data + acc, sizeof(*buffer.data),
                             CHUNK_SIZE, fp)) != 0)
  {
    buffer.used += bytes_read;
    acc += bytes_read;
    if (buffer.used + CHUNK_SIZE >= buffer.available)
    {
      buffer.available = MAX(buffer.available * MULT, buffer.used + CHUNK_SIZE);
      buffer.data      = realloc(buffer.data, buffer.available);
    }
  }
  buffer.data              = realloc(buffer.data, buffer.used + 1);
  buffer.data[buffer.used] = '\0';
  return buffer.data;
}

void vec_append(vec_t *vec, void *ptr, u64 size)
{
  vec_ensure_free(vec, size);
  memcpy(vec->data + vec->size, ptr, size);
  vec->size += size;
}

void vec_ensure(vec_t *vec, u64 abs_size)
{
  if (vec->capacity < abs_size)
  {
    vec->capacity = MAX(vec->capacity * 2, abs_size);
    vec->data     = realloc(vec->data, vec->capacity);
  }
  else if (!vec->data)
  {
    vec->data = calloc(1, vec->capacity);
  }
}

void vec_ensure_free(vec_t *vec, u64 rel_size)
{
  vec_ensure(vec, vec->size + rel_size);
}

void vec_free(vec_t *vec)
{
  if (vec->data)
    free(vec->data);
  memset(vec, 0, sizeof(*vec));
}
