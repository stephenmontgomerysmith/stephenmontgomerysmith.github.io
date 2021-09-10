/*
 * Copyright (c) 2001 by Stephen Montgomery-Smith <stephen@math.missouri.edu>
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
#include <unistd.h>
#include <string.h>

void do_computations(char *input, char *output) {
  int i,j;

  if (sscanf(input,"%d%d",&i,&j)==2)
    sprintf(output,"%d\n",i+j);
  else
    strcpy(output,"Invalid data\n");
//sleep(1);
}

int main(int argc, char **argv) {
  char *input=NULL;
  char output[1000];

  prepare_enact(argc,argv,"",0);

  while (1) {
    get_dispensed(&input);
    do_computations(input,output);
    send_enacted(output);
  }
}
