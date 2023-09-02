#ifndef PARSER_H
#define PARSER_H

#include "./lib.h"

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

struct PResult
{
  node_t *nodes;
  size_t size;
};

char *ast_to_str(node_t *ast, size_t size);
struct PResult parse_buffer(buffer_t *buffer);

#endif
