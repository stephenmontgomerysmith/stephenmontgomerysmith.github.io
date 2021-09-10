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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Copy src to *dest, allocating space to *dest as needed. */

char *strcpyalloc(char **dest, char *src) {
  if ((*dest=realloc(*dest,strlen(src)+1))==NULL) {
    fprintf(stderr,"Allocation error\n");
    exit(1);
  }
  strcpy(*dest,src);
  return *dest;
}

/* Concatonate src to *dest, allocating space to *dest as needed. */

char *strcatalloc(char **dest, char *src) {
  if ((*dest=realloc(*dest,strlen(*dest)+strlen(src)+1))==NULL) {
    fprintf(stderr,"Allocation error\n");
    exit(1);
  }
  strcat(*dest,src);
  return *dest;
}

/* If the string *s ends in "x\n", where x is not '\r', replace end by
   "x\r\n".  Return 0 if string ends with '\n'.
*/

int n2rn(char **s) {
  int len = strlen(*s);

  if (len>=1 && (*s)[len-1] == '\n') {
    if (len==1 || (*s)[len-2]!='\r') {
      if ((*s = realloc(*s,len+2))==NULL) {
        fprintf(stderr,"Allocation error\n");
        exit(1);
      }
      (*s)[len-1] = '\r';
      (*s)[len] = '\n';
      (*s)[len+1] = '\0';
    }
    return 0;
  } else
    return -1;
}

/* If the string *s ends in "\r\n", replace end by "\n".  Returns 0 if this
   replacement took place.
*/

int rn2n(char **s) {
  int len = strlen(*s);

  if (len>=2 && (*s)[len-2] == '\r' && (*s)[len-1] == '\n') {
    (*s)[len-2] = '\n';
    (*s)[len-1] = '\0';
    return 0;
  } else
    return -1;
}
