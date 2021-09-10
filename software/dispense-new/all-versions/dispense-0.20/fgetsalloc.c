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

#include <stdio.h>
#include <stdlib.h>

/* Get a string from the stream f, allocating space to s as required.
*/

char *fgetsalloc(char **s, FILE *f) {
  int c;
  int len;
  int allocated;

  allocated = 4096;
  if ((*s = realloc(*s,4096))==NULL) {
    fprintf(stderr,"Allocation error\n");
    exit(1);
  }
  len = 0;

  while (1) {
    c=getc(f);
    if (c==EOF) break;
    if (len>=allocated-1) {
      allocated += 4096;
      if ((*s = realloc(*s,allocated))==NULL) {
        fprintf(stderr,"Allocation error\n");
        exit(1);
      }
    }
    (*s)[len] = c;
    len++;
    if (c=='\n') break;
  }
  if (len!=0) {
    (*s)[len] = '\0';
    if ((*s = realloc(*s,len+1))==NULL) {
      fprintf(stderr,"Allocation error\n");
      exit(1);
    }
  }
  else {
    if (*s!=NULL) free(*s);
    *s = NULL;
  }
  return *s;
}
