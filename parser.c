/* parser.c
 * Created: 2023-09-02
 * Author: Aryadev Chavali
 * Description: Functions for (de)serialising
 */

#include <string.h>

#include "./parser.h"

char *ast_to_str(node_t *ast, size_t size)
{
  char *out = calloc(size + 1, 1);
  if (!out)
  {
    print_error(
        "[internal]", 0, 0,
        "ERROR: Out of Memory (could not allocate buffer in ast_to_str)");
    return NULL;
  }
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

struct PResult parse_buffer(buffer_t *buffer)
{
  node_t *nodes = NULL;
  size_t usable = 0, loops = 0;

  // First pass: Compute |nodes|
  for (size_t i = 0; i < buffer->size; ++i)
    if (usable_character(buffer->data[i]))
    {
      ++usable;
      if (buffer->data[i] == '[')
        ++loops;
    }
  nodes = calloc(usable, sizeof(*nodes));
  if (!nodes)
  {
    print_error(
        "[internal]", 0, 0,
        "ERROR: Out of Memory (could not allocate buffer in parse_buffer)");
    return (struct PResult){0};
  }

  // Second pass: parse nodes
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

  // Third pass: setup loop references
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

  return (struct PResult){nodes, usable, loops};
error:
  if (nodes)
    free(nodes);
  return (struct PResult){0};
}
