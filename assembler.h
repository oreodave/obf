#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "./lib.h"
#include "./parser.h"

void asm_translate_nodes(vec_t *asm_buffer, struct PResult nodes,
                         const char *src_name);

void asm_write(const char *asm_name, vec_t *asm_buffer);
int asm_assemble(const char *asm_name, const char *objname);
int asm_link(const char *objname, const char *outname);
int asm_compile(vec_t *asm_buffer, const char *asm_name, const char *objname,
                const char *outname);

#endif
