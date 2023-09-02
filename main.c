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
#include "./parser.h"

#define MEMORY_DEFAULT 30000

typedef struct
{
  size_t dp;
  uint8_t memory[MEMORY_DEFAULT];
} machine_t;

void interpret(machine_t *cpu, node_t *ast, size_t num)
{
  for (size_t i = 0; i < num; ++i)
  {
    node_t node = ast[i];
    switch (node.type)
    {
    case NEXT:
      ++cpu->dp;
      break;
    case PREV:
      --cpu->dp;
      break;
    case INC:
      ++cpu->memory[cpu->dp];
      break;
    case DEC:
      --cpu->memory[cpu->dp];
      break;
    case OUT:
      putchar(cpu->memory[cpu->dp]);
      break;
    case READ:
      cpu->memory[cpu->dp] = getchar();
      break;
    case LIN:
      if (cpu->memory[cpu->dp] == 0)
        i = node.loop_ref;
      break;
    case LOUT:
      if (cpu->memory[cpu->dp] != 0)
        i = node.loop_ref;
      break;
    }
  }
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

  machine_t machine = {0};

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

    interpret(&machine, res.nodes, res.size);

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
