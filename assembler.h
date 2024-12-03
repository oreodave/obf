#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "./lib.h"
#include "./parser.h"

void asm_setup_buffer(buffer_t **asm_buffer, const char *outname);
void asm_translate_nodes(buffer_t **asm_buffer, struct PResult nodes,
                         const char *src_name);

void asm_write(buffer_t **asm_buffer);
int asm_assemble(const char *srcname, const char *objname);
int asm_link(const char *objname, const char *outname);
int asm_compile(buffer_t **asm_buffer, const char *objname,
                const char *outname);

#endif
