/* main.c
 * Created: 2023-09-02
 * Author: Aryadev Chavali
 * Description: Entrypoint of compiler
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./lib.h"

#define MEMORY_DEFAULT 30000

typedef struct
{
  size_t dp;
  uint8_t memory[MEMORY_DEFAULT];
} machine_t;

typedef struct AST
{
  size_t col, row;
  enum
  {
    NEXT = 0,
    PREV,
    INC,
    DEC,
    OUT,
    READ,
    LIN,
    LOUT
  } type;
  int loop_ref;
} node_t;

char *ast_to_str(node_t *ast, size_t size)
{
  char *out = calloc(size + 1, 1);
  for (size_t i = 0; i < size; ++i)
  {
    switch (ast[i].type)
    {
    case NEXT:
      out[i] = '>';
      break;
    case PREV:
      out[i] = '<';
      break;
    case INC:
      out[i] = '+';
      break;
    case DEC:
      out[i] = '-';
      break;
    case OUT:
      out[i] = '.';
      break;
    case READ:
      out[i] = ',';
      break;
    case LIN:
      out[i] = '[';
      break;
    case LOUT:
      out[i] = ']';
      break;
    }
  }
  out[size] = '\0';
  return out;
}

struct PResult
{
  node_t *nodes;
  size_t size;
} parse_buffer(buffer_t *buffer)
{
  node_t *nodes = NULL;
  size_t usable = 0, loops = 0;
  for (size_t i = 0; i < buffer->size; ++i)
    if (usable_character(buffer->data[i]))
    {
      ++usable;
      if (buffer->data[i] == '[')
        ++loops;
    }
  nodes = calloc(usable, sizeof(*nodes));

  // First pass: Get my info
  for (size_t i = 0, col = 0, row = 1, nptr = 0; i < buffer->size; ++i)
  {
    ++col;
    if (buffer->data[i] == '\n')
    {
      ++row;
      col = 0;
    }
    else if (usable_character(buffer->data[i]))
    {
      nodes[nptr].col = col;
      nodes[nptr].row = row;
      switch (buffer->data[i])
      {
      case '>':
        nodes[nptr].type = NEXT;
        break;
      case '<':
        nodes[nptr].type = PREV;
        break;
      case '+':
        nodes[nptr].type = INC;
        break;
      case '-':
        nodes[nptr].type = DEC;
        break;
      case '.':
        nodes[nptr].type = OUT;
        break;
      case ',':
        nodes[nptr].type = READ;
        break;
      case '[':
        nodes[nptr].type     = LIN;
        nodes[nptr].loop_ref = -1;
        break;
      case ']':
        nodes[nptr].type     = LOUT;
        nodes[nptr].loop_ref = -1;
        break;
      }
      ++nptr;
    }
  }

  // Second pass: setup any loop references
  node_t *stack[loops];
  memset(stack, 0, loops);
  size_t stackptr = 0;
  for (size_t i = 0; i < usable; ++i)
  {
    node_t *node = nodes + i;
    if (node->type == LIN)
      stack[stackptr++] = node;
    else if (node->type == LOUT)
    {
      if (stackptr == 0)
      {
        print_error(buffer->name, node->row, node->col,
                    "ERROR: Unbalanced square brackets!");
        goto error;
      }
      // Access last IN loop
      --stackptr;
      node->loop_ref            = stack[stackptr] - nodes;
      stack[stackptr]->loop_ref = i;
    }
  }

  if (stackptr > 0)
  {
    node_t *node = nodes + usable - 1;
    print_error(buffer->name, node->row, node->col,
                "ERROR: Unbalanced square brackets!");
    goto error;
  }

  return (struct PResult){nodes, usable};
error:
  if (nodes)
    free(nodes);
  return (struct PResult){0};
}

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    fprintf(
        stderr,
        "Usage: %s [FILE]...\nReads FILES sequentially on the same machine\n",
        argv[0]);
    return 1;
  }

  char *filepath, *file_data;
  buffer_t *buffer;
  struct PResult res;

  for (int i = 1; i < argc; ++i)
  {
    filepath = argv[i];

    FILE *handle = fopen(filepath, "r");
    if (!handle)
    {
      fprintf(stderr, "ERROR: Could not open \"%s\"\n", filepath);
      goto error;
    }
    file_data = fread_all(handle);
    fclose(handle);

    buffer = buffer_init_str(filepath, file_data, strlen(file_data));
    res    = parse_buffer(buffer);
    if (res.nodes == NULL)
    {
      fputs("Exiting early...\n", stderr);
      goto error;
    }

    char *str = ast_to_str(res.nodes, res.size);

    printf("%s=>%s\n", filepath, str);

    free(str);
    free(res.nodes);
    free(buffer);
    free(file_data);
  }
  return 0;
error:
  if (buffer)
    free(buffer);
  if (res.nodes)
    free(res.nodes);
  return 1;
}
