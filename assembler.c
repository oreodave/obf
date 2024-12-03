/* assembler.c
 * Created: 2024-12-03
 * Author: Aryadev Chavali
 * Description: Compile brainfuck into native
 */

#include "./assembler.h"

#include <string.h>

// Structure representing a loop operative as indices into an ASM label array.
struct Label
{
  u64 cur, next;
};

/* First, the functions to call external utilities (badly written, I know). They
 * presume a filled asm_buffer. */

void asm_write(buffer_t **asm_buffer)
{
  FILE *fp = fopen(asm_buffer[0]->name, "w");
  fwrite(asm_buffer[0]->data, asm_buffer[0]->size, 1, fp);
  fclose(fp);
}

int asm_assemble(const char *srcname, const char *objname)
{
#ifdef DEBUG
  char *format_str = "yasm -f elf64 -g dwarf2 -o %s %s";
#else
  char *format_str = "yasm -f elf64 -o %s %s";
#endif
  char command[snprintf(NULL, 0, format_str, objname, srcname) + 1];
  sprintf(command, format_str, objname, srcname);
  return system(command);
}

int asm_link(const char *objname, const char *outname)
{
  char *format_str = "ld -o %s %s";
  char command[snprintf(NULL, 0, format_str, outname, objname) + 1];
  sprintf(command, format_str, outname, objname);
  return system(command);
}

int asm_compile(buffer_t **asm_buffer, const char *objname, const char *outname)
{
  asm_write(asm_buffer);
  asm_assemble(asm_buffer[0]->name, objname);
  return asm_link(objname, outname);
}

void asm_setup_buffer(buffer_t **asm_buffer, const char *outname)
{
  *asm_buffer         = buffer_init_str(outname, NULL, 1024);
  asm_buffer[0]->size = 0;
}

// Write the initial boilerplate of the assembly file
void asm_write_init(buffer_t **asm_buffer)
{
  const char format_string[] = "section .bss\n"
                               "  memory resb %d\n"
                               "section .text\n"
                               "global _start\n"
                               "_start:\n"
                               "  mov r9, memory\n";
  char initial_assembly[snprintf(NULL, 0, format_string, MEMORY_DEFAULT) + 1];
  snprintf(initial_assembly, sizeof(initial_assembly), format_string,
           MEMORY_DEFAULT);
  buffer_append_bytes(asm_buffer, (u8 *)initial_assembly,
                      sizeof(initial_assembly) - 1);
}

// Write the exit code for the assembly file
void asm_write_exit(buffer_t **asm_buffer)
{
  buffer_append_bytes(asm_buffer,
                      (u8 *)"  mov rax, 60\n"
                            "  mov rdi, 0\n"
                            "  syscall\n",
                      37);
}

// A table which translates brainfuck operations into generic assembly code (I
// love C's array indexing)
static const struct
{
  char *str;
  u64 len;
} table[] = {
    [NEXT] = {.str = "  add r9, 1\n", .len = 12},
    [PREV] = {.str = "  sub r9, 1\n", .len = 12},
    [INC]  = {.str = "  add byte [r9], 1\n", .len = 19},
    [DEC]  = {.str = "  sub byte [r9], 1\n", .len = 19},
    [OUT]  = {.str = "  mov rax, 1\n"
                      "  mov rdi, 1\n"
                      "  mov rsi, r9\n"
                      "  mov rdx, 1\n"
                      "  syscall\n",
              .len = 63},
    [READ] = {.str = "  mov rax, 0\n"
                     "  mov rdi, 0\n"
                     "  mov rsi, r9\n"
                     "  mov rdx, 1\n"
                     "  syscall\n",
              .len = 63},
};

// Linear search in the ASM label array for a label with the current AST
// relative index
i64 get_abs_label(u64, struct Label *, u64);

// Translate an AST relative jump to a specific ASM label, storing the current
// and next items in given pointers.
void ast_ref_to_asm_label(u64, struct Label *, u64, i64 *, i64 *);

void asm_translate_nodes(buffer_t **asm_buffer, struct PResult nodes,
                         const char *src_name)
{
  asm_write_init(asm_buffer);
  // First pass: Setup the ASM label array
  struct Label labels[nodes.labels ? nodes.labels * 2 : 1];
  if (nodes.labels)
  {
    u64 label_ptr = 0;
    for (size_t i = 0; i < nodes.size; ++i)
      if (nodes.nodes[i].type == LIN || nodes.nodes[i].type == LOUT)
        labels[label_ptr++] =
            (struct Label){.cur = i, .next = nodes.nodes[i].loop_ref};
  }

  // Second pass: Translating to assembly
  for (size_t i = 0; i < nodes.size; ++i)
  {
    node_t node = nodes.nodes[i];
    if (node.type == LIN || node.type == LOUT)
    {
      // Translate the node-relative addresses to assembly-label-relative
      // addresses
      i64 cur_asm_label = 0, next_asm_label = 0;
      ast_ref_to_asm_label(i, labels, nodes.labels * 2, &cur_asm_label,
                           &next_asm_label);
      if (cur_asm_label == -1 || next_asm_label == -1)
      {
        print_error(
            src_name, node.row, node.col,
            "[ASSEMBLY ERROR]: Could not find label for current jump!\n");
        exit(1);
      }

      // Format labels
      char current_label[128], next_label[128];

      sprintf(current_label, ".L%lu", cur_asm_label);
      sprintf(next_label, ".L%lu", next_asm_label);

      // Setup format string for assembly jump code
      char *format_string = NULL;
      if (node.type == LIN)
      {
        format_string = "%s:\n"
                        "  cmp byte [r9], 0\n"
                        "  je %s\n";
      }
      else
      {
        format_string = "%s:\n"
                        "  cmp byte [r9], 0\n"
                        "  jne %s\n";
      }
      char formatted_string[snprintf(NULL, 0, format_string, current_label,
                                     next_label) +
                            1];
      snprintf(formatted_string, sizeof(formatted_string), format_string,
               current_label, next_label);
      buffer_append_bytes(asm_buffer, (u8 *)formatted_string,
                          sizeof(formatted_string) - 1);
    }
    else
    {
      // I love tables so goddamn much
      buffer_append_bytes(asm_buffer, (u8 *)table[node.type].str,
                          table[node.type].len);
    }
  }
  asm_write_exit(asm_buffer);
}

/* Implementations for throwaway functions */

i64 get_abs_label(u64 ref, struct Label *labels, u64 size)
{
  for (u64 i = 0; i < size; ++i)
    if (labels[i].cur == ref)
      return (i64)i;
  return -1;
}

void ast_ref_to_asm_label(u64 ref, struct Label *labels, u64 size, i64 *cur,
                          i64 *next)
{
  *cur = get_abs_label(ref, labels, size);
  if (*cur == -1)
    return;
  *next = get_abs_label(labels[*cur].next, labels, size);
}
