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

#include <string.h>
#include <db3/db.h>

char *fgetsalloc(char **s, FILE *f);

int main (int argc, char **argv) {
  DB *db;
  FILE *against;
  DBT key = {NULL,0,0,0,0,DB_DBT_REALLOC};
  DBT data = {NULL,0,0,0,0,DB_DBT_REALLOC};
  char *s=NULL;

  setlinebuf(stdout);

  if (argc != 2) {
    fprintf(stderr, "usage: filter-input against-file\n");
    exit(1);
  }

  against = fopen(argv[1],"r");
  if (against==NULL) {
    fprintf(stderr,"Cannot open file\n");
    exit(1);
  }

  if (db_create(&db,NULL,0)!=0 ||
      db->open(db,NULL,NULL,DB_BTREE,DB_CREATE,0) !=0) {
    fprintf(stderr,"Unable to create database\n");
    exit(1);
  }

  while(fgetsalloc(&s,against)!=NULL) {
    key.size = strlen(s)+1;
    key.data = s;
    if (db->get(db,NULL,&key,&data,0)==0)
      fprintf(stderr,"Duplicate input line:\n%s",(char *)key.data);
    if (db->put(db,NULL,&key,&data,0)!=0) {
      fprintf(stderr,"Cannot write to db\n");
      exit(1);
    }
  }

  fclose(against);

  while(fgetsalloc(&s,stdin)!=NULL) {
    key.size = strlen(s)+1;
    key.data = s;
    if (db->get(db,NULL,&key,&data,0)!=0)
      fputs(s,stdout);
  }

  exit(0);
}
