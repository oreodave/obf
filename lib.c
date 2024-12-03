/* lib.c
 * Created: 2023-09-02
 * Author: Aryadev Chavali
 * Description: General functions used throughout
 */

#include <string.h>

#include "./lib.h"

buffer_t *buffer_init_str(const char *name, const char *str, size_t str_size)
{
  buffer_t *buf = malloc(sizeof(*buf) + str_size + 1);
  buf->name     = name;
  buf->size     = str_size;
  buf->capacity = str_size;
  if (str)
  {
    memcpy(buf->data, str, str_size);
    buf->data[str_size] = '\0';
  }
  return buf;
}

bool buffer_ensure(buffer_t **buffer, u64 expected)
{
  if (!buffer)
    return false;
  else if (!*buffer)
  {
    *buffer = buffer_init_str(NULL, NULL, expected);
  }
  else if (buffer[0]->capacity <= expected)
  {
    buffer[0]->capacity = MAX(buffer[0]->capacity * 2, expected);
    *buffer = realloc(*buffer, sizeof(**buffer) + buffer[0]->capacity);
  }
  return true;
}

bool buffer_ensure_relative(buffer_t **buffer, u64 expected)
{
  return buffer_ensure(buffer,
                       buffer && *buffer ? buffer[0]->size + expected : 0);
}

bool buffer_append(buffer_t **buffer, u8 datum)
{
  bool ret = buffer_ensure_relative(buffer, 1);
  if (ret)
    buffer[0]->data[buffer[0]->size++] = datum;
  return ret;
}

bool buffer_append_bytes(buffer_t **buffer, u8 *data, u64 size)
{
  bool ret = buffer_ensure_relative(buffer, size);
  if (ret)
  {
    memcpy(buffer[0]->data + buffer[0]->size, data, size);
    buffer[0]->size += size;
  }
  return ret;
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

bool usable_character(char c)
{
  return c == '>' || c == '<' || c == '+' || c == '-' || c == '-' || c == '.' ||
         c == ',' || c == '[' || c == ']';
}
