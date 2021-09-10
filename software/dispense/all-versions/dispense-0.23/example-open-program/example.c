/* An example to show how to use the open-program function. */

#include <bipipeio.h>
#include <stdio.h>
#include <string.h>

#define nr_lines 10

int main() {
  BPFILE *f;
  int i;
  char s[100];
  char *output=NULL;

  setlinebuf(stdout);
  f = open_program("grep ab");
  for (i=0;i<nr_lines;i++) {
    sprintf(s,"%cb %d\n",i%2?'a':'b',i);
    bpputs(s,f);
/* You might do the following to relieve the buffering implicit in the BPFILE
   structure, but it is not necessary.  (Without it, the program has the
   potential to use a lot of memory, as it stores what it reads from the 
   program "grep ab".)
    if (bpgetsalloc_no_block(&output,f)!=NULL)
      fputs(output,stdout);
*/
  }
/* This next line is necessary to be sure that the program "grep ab" completely
   flushes its output. */
  bpclose_out(f);
  while (bpgetsalloc(&output,f)!=NULL)
    fputs(output,stdout);
  exit(0);
}
