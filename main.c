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

#define DEBUG        0
#define MIN_MEM_DUMP 64

typedef struct
{
  size_t dp;
  uint8_t memory[MEMORY_DEFAULT];

#if DEBUG
  size_t dp_max;
#endif
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
#if DEBUG
    if (cpu->dp > cpu->dp_max)
      cpu->dp_max = cpu->dp + 1;
#endif
  }

#if DEBUG
  if (cpu->dp_max < MIN_MEM_DUMP)
    cpu->dp_max = MIN_MEM_DUMP;
#endif
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
#if DEBUG
    printf("[DEBUG]: Attempting to open file handle to %s\n", filepath);
#endif
    if (!handle)
    {
      fprintf(stderr, "ERROR: Could not open \"%s\"\n", filepath);
      goto error;
    }
    file_data = fread_all(handle);
    fclose(handle);
#if DEBUG
    printf("[DEBUG]: Read data from file %s\n", filepath);
#endif

    buffer = buffer_init_str(filepath, file_data, strlen(file_data));
#if DEBUG
    puts("[DEBUG]: Initialised buffer");
#endif
    res = parse_buffer(buffer);
    if (res.nodes == NULL)
    {
      fputs("Exiting early...\n", stderr);
      goto error;
    }

#if DEBUG
    char *str = ast_to_str(res.nodes, res.size);
    printf("[DEBUG]: Parsed buffer (%lu nodes parsed)\n\t[DEBUG]: Out=%s\n",
           res.size, str);
    free(str);
#endif

    interpret(&machine, res.nodes, res.size);

#if DEBUG
    printf("[DEBUG]: Finished interpreting, memory:");
    for (size_t i = 0; i < machine.dp_max; ++i)
    {
      if (i % 8 == 0)
        printf("\n\t");
      printf("%d    ", machine.memory[i]);
    }
#endif

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
