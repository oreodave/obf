/* obc.c
 * Created: 2024-12-03
 * Author: Aryadev Chavali
 * Description: Entrypoint of compiler
 */

#include <stdio.h>
#include <string.h>

#include "./assembler.h"
#include "./lib.h"
#include "./parser.h"

void usage(const char *name, FILE *fp)
{
  fprintf(
      fp,
      "Usage: %s [-s ASM-NAME | -c OBJ-NAME | -o EXEC-NAME] FILE\n"
      "\tGiven BrainFuck source code FILE, compile it.\n"
      "\tOptions:\n"
      "\t\t-s ASM-NAME: Transpile to assembly, store in ASM-NAME\n"
      "\t\t-c OBJ-NAME: Transpile and assemble, store in OBJ-NAME\n"
      "\t\t-o EXEC-NAME: Transpile, assemble and link, store in EXEC-NAME\n",
      name);
}

struct Config
{
  char *prog_name, *asm_name, *obj_name, *exec_name;
};

int parse_config(struct Config *config, int argc, char *argv[])
{
  for (int i = 1; i < argc; ++i)
  {
    size_t size = strlen(argv[i]);
    if (argv[i][0] == '-')
    {
      if (size > 2 || !strspn(argv[i] + 1, "cso") || i == argc - 1)
        return 0;
      switch (argv[i][1])
      {
      case 's':
        config->asm_name = argv[i + 1];
        ++i;
        break;
      case 'c':
        config->obj_name = argv[i + 1];
        ++i;
        break;
      case 'o':
        config->exec_name = argv[i + 1];
        ++i;
        break;
      }
    }
    else
      config->prog_name = argv[i];
  }
  if (!config->prog_name)
    return 0;
  return 1;
}

int main(int argc, char *argv[])
{
  if (argc == 1)
  {
    usage(argv[0], stderr);
    return 1;
  }

  struct Config config = {0};
  int ret              = parse_config(&config, argc, argv);
  if (!ret)
  {
    usage(argv[0], stderr);
    goto end;
  }
  ret = 0;

  char *file_data  = NULL;
  buffer_t *buffer = NULL, *asm_buffer = NULL;
  struct PResult res = {0};

  FILE *handle = fopen(config.prog_name, "r");
  if (!handle)
  {
    fprintf(stderr, "ERROR: Could not open \"%s\"\n", config.prog_name);
    ret = 1;
    goto end;
  }
  file_data = fread_all(handle);
  fclose(handle);
  buffer = buffer_init_str(config.prog_name, file_data, strlen(file_data));
  res    = parse_buffer(buffer);

  if (res.nodes == NULL)
  {
    fputs("[WARNING]: Empty source file.", stderr);
    ret = 1;
    goto end;
  }

  asm_setup_buffer(&asm_buffer, config.asm_name ? config.asm_name : "a.asm");
  asm_translate_nodes(&asm_buffer, res, config.prog_name);
  asm_write(&asm_buffer);
  asm_assemble(asm_buffer->name, config.obj_name ? config.obj_name : "a.o");
  asm_link(config.obj_name ? config.obj_name : "a.o",
           config.exec_name ? config.exec_name : "a.out");
end:
  if (asm_buffer)
    free(asm_buffer);
  if (res.nodes)
    free(res.nodes);
  if (buffer)
    free(buffer);
  if (file_data)
    free(file_data);
  return ret;
}
