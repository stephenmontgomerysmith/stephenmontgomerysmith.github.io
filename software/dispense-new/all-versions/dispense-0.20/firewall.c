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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>

char *fgetsalloc(char **s, FILE *f);

typedef struct {
  char accept;
  u_long addr;
  u_long mask;
} rule_t;

rule_t *rule = NULL;
int nr_rules = 0;

pthread_mutex_t rules_mutex = PTHREAD_MUTEX_INITIALIZER;

int get_rules(char *rules_file_name) {
  FILE *rule_file;
  char *s = NULL;
  char a[1000];
  int d1,d2,d3,d4,m;
  rule_t *new_rule = NULL;
  int nr_new_rules = 0;
  int ret_val = -1;
  int i;

  pthread_mutex_lock(&rules_mutex);
  rule_file = fopen(rules_file_name,"r");
  if (rule_file == NULL) {
    ret_val = 1;
    goto end;
  }
  while (fgetsalloc(&s,rule_file)!=NULL) {
    m = 32;
    for (i=0;i<strlen(s)&&s[i]!='#';i++);
    for (;i>0&&isspace(s[i-1]);i--);
    s[i] = '\0';
    if (strlen(s)>0) {
      if ((sscanf(s,"%s %d.%d.%d.%d/%d\n",a,&d1,&d2,&d3,&d4,&m) != 6 &&
           sscanf(s,"%s %d.%d.%d.%d\n",a,&d1,&d2,&d3,&d4) != 5) ||
          (strcmp(a,"allow")!=0 && strcmp(a,"deny")!=0) ||
          m>32 || m<0 || 
          d1>255 || d1<0 || d2>255 || d2<0 || 
          d3>255 || d3<0 || d4>255 || d4<0)
        goto bad_end;;
      nr_new_rules++;
      if ((new_rule = realloc(new_rule,sizeof(rule_t)*nr_new_rules))==NULL) {
        fprintf(stderr,"Allocation error\n");
        exit(1);
      }
      new_rule[nr_new_rules-1].accept = strcmp(a,"allow")==0;
      new_rule[nr_new_rules-1].addr = (d1<<24)|(d2<<16)|(d3<<8)|d4;
      if (m==0)
        new_rule[nr_new_rules-1].mask = 0;
      else
        new_rule[nr_new_rules-1].mask = 0xffffffffu<<(32-m);
      new_rule[nr_new_rules-1].addr &= new_rule[nr_new_rules-1].mask;
    }
  }
  ret_val = 0;
end:
  nr_rules = nr_new_rules;
  if (nr_rules==0) {
    if (rule!=NULL) free(rule);
    rule = NULL;
  }
  else {
    if ((rule = realloc(rule,sizeof(rule_t)*nr_rules))==NULL) {
      fprintf(stderr,"Allocation error\n");
      exit(1);
    }
    memcpy(rule,new_rule,sizeof(rule_t)*nr_rules);
  }
bad_end:
  if (new_rule!=NULL) free(new_rule);
  if (rule_file!=NULL) fclose(rule_file);
  pthread_mutex_unlock(&rules_mutex);
  return ret_val;
}

void print_rules(void) {
  int i;

  pthread_mutex_lock(&rules_mutex);
  if (nr_rules==1)
    fprintf(stderr,"There is 1 rule.\n");
  else
    fprintf(stderr,"There are %d rules.\n",nr_rules);
  for (i=0;i<nr_rules;i++)
    fprintf(stderr,"%s ip %lu.%lu.%lu.%lu with mask %lu.%lu.%lu.%lu\n",
            rule[i].accept?"accept":"deny",
            rule[i].addr>>24,(rule[i].addr>>16)&0xff,
            (rule[i].addr>>8)&0xff,rule[i].addr&0xff,
            rule[i].mask>>24,(rule[i].mask>>16)&0xff,
            (rule[i].mask>>8)&0xff,rule[i].mask&0xff);
  pthread_mutex_unlock(&rules_mutex);
}

int check_rule(u_long ip) {
  int i;
  int ret_val=1;

  pthread_mutex_lock(&rules_mutex);
  if (ip==0x7f000001) ret_val=1;
  else for (i=0;i<nr_rules;i++)
    if ((ip&rule[i].mask) == rule[i].addr) {
      ret_val = rule[i].accept;
      break;
    }
  pthread_mutex_unlock(&rules_mutex);
  return ret_val;
}
