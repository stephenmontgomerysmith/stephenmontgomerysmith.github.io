/*
 * Copyright (c) 2001, 2002 by Stephen Montgomery-Smith
 * <stephen@math.missouri.edu>
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation.
 *
 * This file is provided AS IS with no warranties of any kind.  The author
 * shall have no liability with respect to the infringement of copyrights,
 * trade secrets or any patents by this file or any part thereof.  In no
 * event will the author be liable for any lost revenue or profits or
 * other special, indirect and consequential damages.
 *
 */

#include <sys/types.h>

typedef struct {
  int in_fd, out_fd;
  char *in_buffer;
  size_t in_begin;
  size_t in_end;
  size_t in_allocated;
  char *out_buffer;
  size_t out_begin;
  size_t out_end;
  size_t out_allocated;
  int error, in_eof;
} BPFILE;

BPFILE *bpfdopen(int in_fd, int out_fd);
int bpflush(BPFILE *f);
char *bpgetsalloc_blockq(char **s, BPFILE *f, int block);
#define bpgetsalloc(s,f) bpgetsalloc_blockq(s, f, 1)
#define bpgetsalloc_no_block(s,f) bpgetsalloc_blockq(s, f, 0)
int bpeof_in(BPFILE *f);
int bperror(BPFILE *f);
int bpputs(char *s, BPFILE *f);
int bpclose_out(BPFILE *f);
int bpclose(BPFILE *f);

BPFILE *open_program(char *program_name);
BPFILE *open_program_pty(char *program_name);
