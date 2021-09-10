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

#include <dispense.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  char *input=NULL, *output=NULL;
  BPFILE *stream_to_prog;
  char *program_name;
  int use_pty;

  use_pty = strchr(prepare_enact(argc,argv,"p",
                                 1,"program-name",&program_name),'p')!=NULL;

  if (use_pty) {
    if ((stream_to_prog = open_program_pty(program_name))==NULL) {
      fprintf(stderr,"Cannot open program\n");
      exit(1);
    }
  } else {
    if ((stream_to_prog = open_program(program_name))==NULL) {
      fprintf(stderr,"Cannot open program\n");
      exit(1);
    }
  }

  while (1) {
    get_dispensed(&input);
    if (bpputs(input,stream_to_prog)<0) exit(1);
    bpflush(stream_to_prog);
    if (use_pty)
      if (bpgetsalloc(&input,stream_to_prog)==NULL) exit(1);
    if (bpgetsalloc(&output,stream_to_prog)==NULL) exit(1);
    send_enacted(output);
  }
}
