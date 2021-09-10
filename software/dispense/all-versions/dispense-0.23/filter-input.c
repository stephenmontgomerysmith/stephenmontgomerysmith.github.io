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
#include <ctype.h>
#include <stdlib.h>

char *strcpyalloc(char **,char*);
char *fgetsalloc(char **, FILE *);

int main (int argc, char **argv) {
  DB *db;
  FILE *against;
  DBT key = {NULL,0,0,0,0,DB_DBT_REALLOC};
  DBT data = {NULL,0,0,0,0,DB_DBT_REALLOC};
  char *s=NULL;
  char **str;
  char *against_file_name=NULL;
  int i, j, error=0;
  int by=1, offset=1;

  if (db_create(&db,NULL,0)!=0 ||
      db->open(db,NULL,NULL,DB_BTREE,DB_CREATE,0) !=0) {
    fprintf(stderr,"Unable to create database\n");
    exit(1);
  }

  i = 1;
  while(i<argc) {
    if (strncmp(argv[i],"-o",2)==0) {
      if (strlen(argv[i])==2) {
        if (i+1<argc) {
          strcpyalloc(&against_file_name,argv[i+1]);
          i+=2;
        }
        else error = 1;
      } else {
        strcpyalloc(&against_file_name,argv[i]+2);
        i++;
      }
      against = fopen(against_file_name,"r");
      if (against==NULL) {
        fprintf(stderr,"Cannot open file %s: ",against_file_name);
        perror("");
        exit(1);
      }
      while(fgetsalloc(&s,against)!=NULL) {
        if (strncmp(s,"Data from",9)==0) {
          if (fgetsalloc(&s,against)==NULL) {
            fprintf(stderr,"bad output-against file\n");
            exit(1);
          }
          key.size = strlen(s)+1;
          key.data = s;
          if (db->get(db,NULL,&key,&data,0)==0)
            fprintf(stderr,"Duplicate input line:\n%s",(char *)key.data);
          if (db->put(db,NULL,&key,&data,0)!=0) {
            fprintf(stderr,"Cannot write to db\n");
            exit(1);
          }
          if (fgetsalloc(&s,against)==NULL) {
            fprintf(stderr,"bad output-against file\n");
            exit(1);
          }
        }
      }
      fclose(against);
    }

    else if (strncmp(argv[i],"-q",2)==0) {
      if (strlen(argv[i])==2) {
        if (i+1<argc) {
          strcpyalloc(&against_file_name,argv[i+1]);
          i+=2;
        }
        else error = 1;
      } else {
        strcpyalloc(&against_file_name,argv[i]+2);
        i++;
      }
      against = fopen(against_file_name,"r");
      if (against==NULL) {
        fprintf(stderr,"Cannot open file %s: ",against_file_name);
        perror("");
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
        if (fgetsalloc(&s,against)==NULL) {
          fprintf(stderr,"bad output-against file\n");
          exit(1);
        }
      }
      fclose(against);
    }

    else if (strncmp(argv[i],"-n",2)==0) {
      if (strlen(argv[i])==2) {
        if (i+1<argc) {
          strcpyalloc(&against_file_name,argv[i+1]);
          i+=2;
        }
        else error = 1;
      } else {
        strcpyalloc(&against_file_name,argv[i]+2);
        i++;
      }
      against = fopen(against_file_name,"r");
      if (against==NULL) {
        fprintf(stderr,"Cannot open file %s: ",against_file_name);
        perror("");
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
    }

    else if (argv[i][0]=='-' && isdigit(argv[i][1])) {
      by = atoi(argv[i]+1);
      for (j=1;isdigit(argv[i][j]);j++);
      if (argv[i][j]!=',' || !isdigit(argv[i][j+1])) error = 1;
      else {
        offset = atoi(argv[i]+j+1);
        for (j=j+1;isdigit(argv[i][j]);j++);
        if (argv[i][j]!='\0') error = 1;
      }
      if (by<=0 || offset<=0 || offset>by) error=1;
      i++;
    }

    else error = 1;

    if (error==1) {
      fprintf(stderr, "usage: filter-input [-o against-output] [-q against-queue] "
              "[-n against-normal] [-number,number]\n");
      exit(1);
    }
  }

  str = malloc(by*sizeof(char*));
  if (str==NULL) {
    fprintf(stderr,"Allocation error\n");
    exit(1);
  }
  bzero(str,by*sizeof(char*));

  setlinebuf(stdout);

  while(fgetsalloc(str,stdin)!=NULL) {
    for (i=1;i<by;i++)
      if (fgetsalloc(str+i,stdin)==NULL) {
        fprintf(stderr,"Improper end to stdin\n");
        exit(1);
      }
    key.size = strlen(str[offset-1])+1;
    key.data = str[offset-1];
    if (db->get(db,NULL,&key,&data,0)!=0)
      for (i=0;i<by;i++)
        fputs(str[i],stdout);
  }

  exit(0);
}
