/* obi.c
 * Created: 2023-09-02
 * Author: Aryadev Chavali
 * Description: Entrypoint of interpreter
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./lib.h"
#include "./parser.h"

#define MIN_MEM_DUMP 64

typedef struct
{
  size_t dp;
  uint8_t memory[MEMORY_DEFAULT];

#ifdef DEBUG
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
#ifdef DEBUG
    if (cpu->dp > cpu->dp_max)
      cpu->dp_max = cpu->dp + 1;
#endif
  }

#ifdef DEBUG
  if (cpu->dp_max < MIN_MEM_DUMP)
    cpu->dp_max = MIN_MEM_DUMP;
#endif
}

void usage(const char *name, FILE *fp)
{
  fprintf(fp,
          "Usage: %s FILE...\n\tInterprets FILE as a BrainFuck program"
          "\n",
          name);
}

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    usage(argv[0], stderr);
    return 1;
  }

  int ret        = 0;
  char *filepath = argv[1], *file_data = NULL;
  buffer_t *buffer   = NULL;
  struct PResult res = {0};
  machine_t machine  = {0};

  FILE *handle = fopen(filepath, "r");
#ifdef DEBUG
  printf("[DEBUG]: Attempting to open file handle to %s\n", filepath);
#endif
  if (!handle)
  {
    fprintf(stderr, "ERROR: Could not open \"%s\"\n", filepath);
    ret = 1;
    goto end;
  }
  file_data = fread_all(handle);
  fclose(handle);
#ifdef DEBUG
  printf("[DEBUG]: Read data from file %s\n", filepath);
#endif

  buffer = buffer_init_str(filepath, file_data, strlen(file_data));
#ifdef DEBUG
  puts("[DEBUG]: Initialised buffer");
#endif
  res = parse_buffer(buffer);
  if (res.nodes == NULL)
  {
    fputs("[WARNING]: Empty source file.", stderr);
    ret = 1;
    goto end;
  }

#ifdef DEBUG
  char *str = ast_to_str(res.nodes, res.size);
  printf("[DEBUG]: Parsed buffer (%lu nodes parsed)\n\t[DEBUG]: Out=%s\n",
         res.size, str);
  free(str);
#endif

  interpret(&machine, res.nodes, res.size);

#ifdef DEBUG
  printf("[DEBUG]: Finished interpreting, memory:");
  for (size_t i = 0; i < machine.dp_max; ++i)
  {
    if (i % 8 == 0)
      printf("\n\t");
    printf("%d    ", machine.memory[i]);
  }
  printf("\n");
#endif
end:
  if (res.nodes)
    free(res.nodes);
  if (buffer)
    free(buffer);
  if (file_data)
    free(file_data);
  return ret;
}
