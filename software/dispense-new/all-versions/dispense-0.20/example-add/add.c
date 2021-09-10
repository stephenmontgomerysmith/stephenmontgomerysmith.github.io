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

#include <stdio.h>
#include <unistd.h>

int main() {
  int i,j;
  char s[1000];

  setlinebuf(stdout);
  while ((fgets(s,1000,stdin))!=NULL) {
    if (sscanf(s,"%d%d",&i,&j)!=2)
      printf("Invalid input\n");
    else
      printf("%d\n",i+j);
//  sleep(1);
  }
  exit(0);
}
