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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <db3/db.h>
#include "dispense-stdingets.h"

#ifdef LIBWRAP
#include <tcpd.h>
#include <syslog.h>
int allow_severity = LOG_INFO;
int deny_severity = LOG_WARNING;
int use_wrappers = 0;
#endif

#undef VERBOSE_QUEUE
#undef VERBOSE
#undef MILDLY_VERBOSE

char *password = NULL;
int timeout;
int max_timeout=1<<30;
int max_simultaneous=1<<30;
char *queue_file_name = NULL;
int port_nr;
int continue_clients = 0;
int socket_reuseadrr = 0;
int finished = 0;
int signal_from_usr1 = 0;
int signal_from_term = 0;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t stdin_mutex = PTHREAD_MUTEX_INITIALIZER;

STDINFILE *stdinfile;

typedef struct {
  int connfd;
  struct in_addr addr;
  u_short port;
}
arg_pass_type;

typedef struct {
  int timeout;
  int simultaneous;
}
queue_data_type;

DB *queue_db, *use_db;

int min_timeout = 0; /* guaranteed to be <= minimum timeout in queue_db */

void set_finished(char *message, int do_exit) {
  char s_time[1000];

  pthread_mutex_lock(&print_mutex);
  if (!finished)
    printf("%s [%s]\n",message,str_time(s_time));
  if (do_exit) exit(0);
  finished = 1;
  pthread_mutex_unlock(&print_mutex);
}

int check_finished(void) {
  int r;

  pthread_mutex_lock(&print_mutex);
  r = finished;
  pthread_mutex_unlock(&print_mutex);
  return r;
}

/* Return 0 if database is empty, 1 if there is data, 2 if there is data but
   it is not usuable yet */
int de_queue(int regular) {
  DBT key = {NULL,0,0,0,0,DB_DBT_REALLOC};
  queue_data_type queue_data;
  DBT data = {&queue_data,sizeof(queue_data),sizeof(queue_data),0,0,DB_DBT_USERMEM};
  DBC *cursor;
  int ret_val = 0;
  int temp_min_timeout = max_timeout;

#ifdef VERBOSE_QUEUE
  pthread_mutex_lock(&print_mutex);
  fprintf(stderr,"De-queue - %s\n",regular?"regular":"not regular");
  pthread_mutex_unlock(&print_mutex);
#endif

  pthread_mutex_lock(&db_mutex);
  if (queue_db->cursor(queue_db,NULL,&cursor,0) != 0) {
    fprintf(stderr,"Cannot create cursor for queue database\n");
    exit(1);
  }
  while(cursor->c_get(cursor,&key,&data,DB_NEXT)==0) {
    if (ret_val==0) ret_val = 2;
    if (regular || queue_data.simultaneous<max_simultaneous) {
      ret_val = 1;
      if (regular)
        queue_data.timeout--;
      else
        queue_data.timeout -= min_timeout+1;
    }

#ifdef VERBOSE_QUEUE
    pthread_mutex_lock(&print_mutex);
    fprintf(stderr,"De-queue: %d %d %s",queue_data.timeout,
            queue_data.simultaneous, (char*)key.data);
    pthread_mutex_unlock(&print_mutex);
#endif

    if (queue_data.timeout<0) {
      if (regular) queue_data.simultaneous=1;
      if (queue_db->del(queue_db,NULL,&key,0) != 0 ||
          use_db->put(use_db,NULL,&key,&data,0) != 0) {
        fprintf(stderr,"Unable to delete from or put in db\n");
        exit(1);
      }
    }
    else {
      if (queue_data.timeout < temp_min_timeout)
        temp_min_timeout = queue_data.timeout;
      if (queue_db->del(queue_db,NULL,&key,0) != 0 ||
          queue_db->put(queue_db,NULL,&key,&data,0) != 0) {
        fprintf(stderr,"Unable to delete from or put in db\n");
        exit(1);
      }
    }
  }
  min_timeout = temp_min_timeout;
  if (cursor->c_close(cursor)!=0) {
    fprintf(stderr,"Cannot close cursor\n");
    exit(1);
  }

  if (ret_val != 1) {
    if (use_db->cursor(use_db,NULL,&cursor,0) != 0) {
      fprintf(stderr,"Cannot create cursor for use database\n");
      exit(1);
    }
    if (cursor->c_get(cursor,&key,&data,DB_NEXT)==0)
      ret_val = 1;
    if (cursor->c_close(cursor)!=0) {
      fprintf(stderr,"Cannot close cursor\n");
      exit(1);
    }
  }

  pthread_mutex_unlock(&db_mutex);

  if (key.data!=NULL) free(key.data);

  return ret_val;
}

void *repeat_de_queue(void *arg) {
  pthread_detach(pthread_self());
  while(1) {
    sleep(60);
    de_queue(1);
  }
}

int print_queue_and_die() {
  DBT key = {NULL,0,0,0,0,DB_DBT_REALLOC};
  queue_data_type queue_data;
  DBT data = {&queue_data,sizeof(queue_data),sizeof(queue_data),0,0,DB_DBT_USERMEM};
  DBC *cursor;
  char s_time[1000];

  pthread_mutex_lock(&print_mutex);
  pthread_mutex_lock(&db_mutex);

  fprintf(stderr,"Dispense terminating at [%s]\nQueue is:\n",str_time(s_time));

  if (queue_db->cursor(queue_db,NULL,&cursor,0) != 0) {
    fprintf(stderr,"Cannot create cursor for queue database\n");
    exit(1);
  }
  while(cursor->c_get(cursor,&key,&data,DB_NEXT)==0) {
    fprintf(stderr,"%s",(char*)key.data);
    fprintf(stderr,"%d %d\n",queue_data.timeout,queue_data.simultaneous);
  }
  if (cursor->c_close(cursor)!=0) {
    fprintf(stderr,"Cannot close cursor\n");
    exit(1);
  }
  if (use_db->cursor(use_db,NULL,&cursor,0) != 0) {
    fprintf(stderr,"Cannot create cursor for use database\n");
    exit(1);
  }
  while(cursor->c_get(cursor,&key,&data,DB_NEXT)==0) {
    fprintf(stderr,"%s",(char*)key.data);
    fprintf(stderr,"%d %d\n",-1,0);
  }

  fprintf(stderr,"End of queue\n");
  exit(0);
}

void *repeat_print_queue_and_die(void *arg) {
  pthread_detach(pthread_self());
  while(1) {
    sleep(1);
    if (signal_from_term) print_queue_and_die();
  }
}

void add_to_queue_db(char *input,int timeout,int simultaneous) {
  queue_data_type queue_data = {timeout,simultaneous};
  DBT key = {input,strlen(input)+1,strlen(input)+1,0,0,DB_DBT_USERMEM};
  DBT data = {&queue_data,sizeof(queue_data),sizeof(queue_data),0,0,DB_DBT_USERMEM};

  if (timeout < min_timeout)
    min_timeout = timeout;
  if (queue_db->put(queue_db,NULL,&key,&data,0) != 0) {
    fprintf(stderr,"Unable to put to db\n");
    exit(1);
  }
}

void add_to_use_db(char *input) {
  queue_data_type queue_data = {-1,0};
  DBT key = {input,strlen(input)+1,strlen(input)+1,0,0,DB_DBT_USERMEM};
  DBT data = {&queue_data,sizeof(queue_data),sizeof(queue_data),0,0,DB_DBT_USERMEM};

  if (queue_db->put(use_db,NULL,&key,&data,0) != 0) {
    fprintf(stderr,"Unable to put to db\n");
    exit(1);
  }
}

void fill_queue() {
  FILE *queue_file;
  char *queue_line=NULL,*data_line=NULL;
  int timeout, simultaneous;

  if (queue_file_name != NULL) {
    queue_file = fopen(queue_file_name,"r");
    if (queue_file==NULL) {
      fprintf(stderr,"Queue file %s not found\n",queue_file_name);
      exit(1);
    }
    while (fgetsalloc(&queue_line,queue_file)!=NULL) {
      if (fgetsalloc(&data_line,queue_file)==NULL ||
          sscanf(data_line,"%d%d",&timeout,&simultaneous)!=2) {
        fprintf(stderr,"Bad queue file\n");
        exit(1);
      }
      if (timeout>=0)
        add_to_queue_db(queue_line,timeout,simultaneous);
      else
        add_to_use_db(queue_line);
    }
  }
  if (queue_line!=NULL) free(queue_line);
  if (data_line!=NULL) free(data_line);
}

int get_from_use_db(char **input,int timeout) {
  DBC *cursor=NULL;
  DBT key = {NULL,0,0,0,0,DB_DBT_REALLOC};
  queue_data_type queue_data;
  DBT data = {&queue_data,sizeof(queue_data),sizeof(queue_data),0,0,DB_DBT_USERMEM};
  int found = 0;

  pthread_mutex_lock(&db_mutex);
  if (use_db->cursor(use_db,NULL,&cursor,0) != 0) {
    fprintf(stderr,"Cannot create cursor for database\n");
    exit(1);
  }
  if (cursor->c_get(cursor,&key,&data,DB_FIRST)==0) {
    if (use_db->del(use_db,NULL,&key,0) != 0) {
      fprintf(stderr,"Cannot delete from database\n");
      exit(1);
    }
    found = 1;
    strcpyalloc(input,key.data);
    add_to_queue_db(*input,timeout,queue_data.simultaneous+1);
  }
  pthread_mutex_unlock(&db_mutex);
  if (key.data!=NULL) free(key.data);
  if (cursor!=NULL) cursor->c_close(cursor);
  return found;
}

/* Return 0 if eof, 1 if input that is not a duplicate, 2 if waiting on input */
int get_from_stdin(char **input,int timeout) {
  DBT key;
  queue_data_type queue_data;
  DBT data = {&queue_data,sizeof(queue_data),sizeof(queue_data),0,0,DB_DBT_USERMEM};
  int found = 0;
  int duplicate;
  int stdin_eof=0;

  do {
    duplicate = 0;
    pthread_mutex_lock(&stdin_mutex);
    stdin_eof = stdingetsalloc(input,stdinfile)==NULL;
    pthread_mutex_unlock(&stdin_mutex);
    if (!stdin_eof && **input!='\0') {
      pthread_mutex_lock(&db_mutex);
      if (strlen(*input)>=1 && (*input)[strlen(*input)-1]=='\n') {
        key.size = strlen(*input)+1;
        key.data = *input;
        duplicate = queue_db->get(queue_db,NULL,&key,&data,0) == 0 ||
                    use_db->get(use_db,NULL,&key,&data,0) == 0;
        if (duplicate) {
          pthread_mutex_lock(&print_mutex);
          fprintf(stderr, "Duplicate input\n%s",*input);
          pthread_mutex_unlock(&print_mutex);
        }
        else {
          found = 1;
          strcpyalloc(input,key.data);
          add_to_queue_db(*input,timeout,1);
        }
      }
      pthread_mutex_unlock(&db_mutex);
    }
  } while (duplicate);
  if (found) return 1;
  if (stdin_eof) return 0;
  return 2;
}

void *process_client(void *arg) {
  int connfd = ((arg_pass_type*)arg)->connfd;
  struct in_addr addr = ((arg_pass_type*)arg)->addr;
  u_short port = ((arg_pass_type*)arg)->port;

  TCPFILE *client;
  char *read_s=NULL, *input=NULL, *output=NULL;
  char *identifier=NULL;
  char info[1000];
  char s_time[1000];
  DBT key;
  int found;
  int local_timeout;
  int stdin_status;
  int de_queue_status;
  int no_lines,i;
  char query_answer[259];
  int no_data;

  queue_data_type queue_data;
  DBT data = {&queue_data,sizeof(queue_data),sizeof(queue_data),0,0,DB_DBT_USERMEM};


  free(arg);

  pthread_detach(pthread_self());

#ifdef VERBOSE
  sprintf(info, "Starting thread %s:%d [%s]\n",
          inet_ntoa(addr),ntohs(port),
          str_time(s_time));
  pthread_mutex_lock(&print_mutex);
  fprintf(stderr,"%s",info);
  pthread_mutex_unlock(&print_mutex);
#endif

  if ((client=tcpfdopen(connfd))==NULL) goto very_end;
  if (tcpgetsalloc(&read_s,TCP_MAX_LINE,client)==NULL) goto very_end;

  if (strncmp(read_s,"connection from enact",21)!=0) goto end;
  if (strcmp(read_s+21,"\r\n")!=0) {
    if (strncmp(read_s+21,": ",2)!=0) goto end;
    if (strlen(read_s)>500) read_s[500] = '\0';
    strcpyalloc(&identifier,read_s+23);
    if (strlen(identifier)>0 &&
        identifier[strlen(identifier)-1]=='\n')
      identifier[strlen(identifier)-1] = '\0';
    if (strlen(identifier)>0 &&
        identifier[strlen(identifier)-1]=='\r')
      identifier[strlen(identifier)-1] = '\0';
  }
  if (tcpgetsalloc(&read_s,TCP_MAX_LINE,client)==NULL) goto end;
  if (strcmp(read_s,password)!=0) goto end;
  if (tcpgetsalloc(&read_s,TCP_MAX_LINE,client)==NULL || 
      rn2n(&read_s)<0) goto end;
  if (sscanf(read_s,"%d",&local_timeout)!=1) goto end;
  if (local_timeout<timeout) local_timeout=timeout;
  if (local_timeout>max_timeout) local_timeout=max_timeout;

  while (1) {
#ifdef VERBOSE
    sprintf(info, "Thread in loop %s:%d [%s]\n",
            inet_ntoa(addr),ntohs(port),
            str_time(s_time));
    pthread_mutex_lock(&print_mutex);
    fprintf(stderr,"%s",info);
    pthread_mutex_unlock(&print_mutex);
#endif
    if (signal_from_usr1) {
      signal_from_usr1 = 0;
      set_finished("Project terminated with SIGUSR1",0 /*don't exit*/);
    }

    if (tcpgetsalloc(&read_s,TCP_MAX_LINE,client)==NULL) goto very_end;

#ifdef VERBOSE
    sprintf(info, "Thread received command %s:%d [%s]\n",
            inet_ntoa(addr),ntohs(port),
            str_time(s_time));
    pthread_mutex_lock(&print_mutex);
    fprintf(stderr,"%s%s",info,read_s);
    pthread_mutex_unlock(&print_mutex);
#endif

    if (strncmp(read_s,"request data",12)==0
        && strcmp(read_s+strlen(read_s)-2,"\r\n")==0) {
      if (strlen(read_s)==14)
        no_lines = 1;
      else
        no_lines = atoi(read_s+13);
      if (no_lines<1 || no_lines>256) goto end;
      no_data = 0;
      for (i=0;i<no_lines && !no_data;i++) {
        while (1) {
          if (!check_finished()) {
            if (get_from_use_db(&input,local_timeout)) break;
            stdin_status = get_from_stdin(&input,local_timeout);
            if (stdin_status==1) break;
            if (stdin_status==2) {
              strcpyalloc(&input,"No data\n");
              no_data = 1;
              break;
            }
            de_queue_status = de_queue(/*not regular*/0);
            if (de_queue_status==0)
              set_finished("Finished",continue_clients);
            else if (de_queue_status==2) {
              strcpyalloc(&input,"No data\n");
              no_data = 1;
              break;
            }
          }
          else {
            strcpyalloc(&input,"Finished\n");
            no_data = 1;
            break;
          }
        }
        n2rn(&input);
        tcpputs(input,client);
#ifdef MILDLY_VERBOSE
        sprintf(info, "Data to %s:%d [%s]\n",inet_ntoa(addr),ntohs(port),
                str_time(s_time));
        pthread_mutex_lock(&print_mutex);
        fprintf(stderr,"%s%s",info,input);
        pthread_mutex_unlock(&print_mutex);
#endif
      }
    }

    else if (strncmp(read_s,"sending data",12)==0
             && strcmp(read_s+strlen(read_s)-2,"\r\n")==0) {
      if (strlen(read_s)==14)
        no_lines = 1;
      else
        no_lines = atoi(read_s+13);
      if (no_lines<1 || no_lines>256) goto end;
      for (i=0;i<no_lines;i++) {
        if (tcpgetsalloc(&input,TCP_MAX_LINE,client)==NULL ||
            rn2n(&input)<0) goto end;
        if (tcpgetsalloc(&output,TCP_MAX_LINE,client)==NULL ||
            rn2n(&output)<0) goto end;
        if (identifier==NULL)
          sprintf(info, "Data from %s:%d [%s]\n",inet_ntoa(addr),ntohs(port),
                  str_time(s_time));
        else
          sprintf(info, "Data from %s:%d (%s) [%s]\n",inet_ntoa(addr),
                  ntohs(port),identifier,str_time(s_time));
        pthread_mutex_lock(&db_mutex);
        key.data = input;
        key.size = strlen(input)+1;
        found = queue_db->del(queue_db,NULL,&key,0)==0 ||
                use_db->del(use_db,NULL,&key,0)==0;
        pthread_mutex_lock(&print_mutex);
        if (found)
          printf("%s%s%s",info,input,output);
        else
          fprintf(stderr,"Unrecognised %s%s%s",info,input,output);
        pthread_mutex_unlock(&print_mutex);
#ifdef MILDLY_VERBOSE
        sprintf(info, "Data from %s:%d [%s]\n",inet_ntoa(addr),ntohs(port),
                str_time(s_time));
        pthread_mutex_lock(&print_mutex);
        fprintf(stderr,"%s%s%s",info,input,output);
        pthread_mutex_unlock(&print_mutex);
#endif
/* Important to put the following mutex unlock here rather than slightly
   earlier, to avoid a possible race condition in which program might
   quit before writing data to stdout. */
        pthread_mutex_unlock(&db_mutex); 
      }
    }

    else if (strncmp(read_s,"sending intermediate",20)==0
             && strcmp(read_s+strlen(read_s)-2,"\r\n")==0) {
      if (strlen(read_s)==22)
        no_lines = 1;
      else
        no_lines = atoi(read_s+21);
      if (no_lines<1 || no_lines>256) goto end;
      for (i=0;i<no_lines;i++) {
        if (tcpgetsalloc(&input,TCP_MAX_LINE,client)==NULL ||
            rn2n(&input)<0) goto end;
        if (tcpgetsalloc(&output,TCP_MAX_LINE,client)==NULL ||
            rn2n(&output)<0) goto end;
        if (identifier==NULL)
          sprintf(info, "Intermediate from %s:%d [%s]\n",inet_ntoa(addr),ntohs(port),
                  str_time(s_time));
        else
          sprintf(info, "Intermediate from %s:%d (%s) [%s]\n",inet_ntoa(addr),
                  ntohs(port),identifier,str_time(s_time));
        pthread_mutex_lock(&db_mutex);
        key.data = input;
        key.size = strlen(input)+1;
        found = queue_db->get(queue_db,NULL,&key,&data,0) == 0 ||
                use_db->get(use_db,NULL,&key,&data,0) == 0;
        pthread_mutex_lock(&print_mutex);
        if (found)
          printf("%s%s%s",info,input,output);
        else
          fprintf(stderr,"Unrecognised %s%s%s",info,input,output);
        pthread_mutex_unlock(&print_mutex);
#ifdef MILDLY_VERBOSE
        sprintf(info, "Intermediate from %s:%d [%s]\n",inet_ntoa(addr),ntohs(port),
                str_time(s_time));
        pthread_mutex_lock(&print_mutex);
        fprintf(stderr,"%s%s%s",info,input,output);
        pthread_mutex_unlock(&print_mutex);
#endif
/* Important to put the following mutex unlock here rather than slightly
   earlier, to avoid a possible race condition in which program might
   quit before writing data to stdout. */
        pthread_mutex_unlock(&db_mutex); 
      }
    }

    else if (strncmp(read_s,"query",5)==0 
             && strcmp(read_s+strlen(read_s)-2,"\r\n")==0) {
      if (strlen(read_s)==7)
        no_lines = 1;
      else
        no_lines = atoi(read_s+6);
      if (no_lines<1 || no_lines>256) goto end;
      for (i=0;i<no_lines;i++) {
        if (tcpgetsalloc(&input,TCP_MAX_LINE,client)==NULL ||
            rn2n(&input)<0) goto end;
        pthread_mutex_lock(&db_mutex);
        key.data = input;
        key.size = strlen(input)+1;
        found = 0;
        if (queue_db->get(queue_db,NULL,&key,&data,0) == 0) {
          found = 1;
          if (queue_db->del(queue_db,NULL,&key,0)!=0) {
            fprintf(stderr,"Unable to delete from db\n");
            exit(1);
          }
        }
        else if (use_db->get(use_db,NULL,&key,&data,0) == 0) {
          found = 1;
          if (use_db->del(use_db,NULL,&key,0)!=0) {
            fprintf(stderr,"Unable to delete from db\n");
            exit(1);
          }
        }
        query_answer[i] = found?'y':'n';
        if (found)
          add_to_queue_db(input,local_timeout,queue_data.simultaneous);
        pthread_mutex_unlock(&db_mutex);
      }
      query_answer[no_lines] = '\r';
      query_answer[no_lines+1] = '\n';
      query_answer[no_lines+2] = '\0';
      tcpputs(query_answer,client);
#ifdef MILDLY_VERBOSE
      sprintf(info, "Query %d from %s:%d [%s]\n",no_lines,
              inet_ntoa(addr),ntohs(port),
              str_time(s_time));
      pthread_mutex_lock(&print_mutex);
      fprintf(stderr,"%s",info);
      pthread_mutex_unlock(&print_mutex);
#endif
    }

    else if (strncmp(read_s,"relinquish data",15)==0 
             && strcmp(read_s+strlen(read_s)-2,"\r\n")==0) {
      if (strlen(read_s)==17)
        no_lines = 1;
      else
        no_lines = atoi(read_s+16);
      if (no_lines<1 || no_lines>256) goto end;
      for (i=0;i<no_lines;i++) {
        if (tcpgetsalloc(&input,TCP_MAX_LINE,client)==NULL ||
            rn2n(&input)<0) goto end;
        pthread_mutex_lock(&db_mutex);
        key.data = input;
        key.size = strlen(input)+1;
        found = 0;
        if (queue_db->get(queue_db,NULL,&key,&data,0) == 0) {
          found = 1;
          if (queue_db->del(queue_db,NULL,&key,0)!=0) {
            fprintf(stderr,"Unable to delete from db\n");
            exit(1);
          }
        }
        if (found) {
          queue_data.simultaneous--;
          if (queue_data.simultaneous >= 1) {
            if (queue_db->put(queue_db,NULL,&key,&data,0) != 0) {
              fprintf(stderr,"Unable to put to db\n");
              exit(1);
            }
          }
          else {
            if (use_db->put(use_db,NULL,&key,&data,0) != 0) {
              fprintf(stderr,"Unable to put to db\n");
              exit(1);
            }
          }
        }
        pthread_mutex_unlock(&db_mutex);
      }
#ifdef MILDLY_VERBOSE
      sprintf(info, "Relinquish %d from %s:%d [%s]\n",no_lines,
              inet_ntoa(addr),ntohs(port),
              str_time(s_time));
      pthread_mutex_lock(&print_mutex);
      fprintf(stderr,"%s",info);
      pthread_mutex_unlock(&print_mutex);
#endif
    }

    else goto end;

    if (tcpflush(client)<0) goto end;
  }
end:
  pthread_mutex_lock(&print_mutex);
  fprintf(stderr, "Bad protocol from %s:%d [%s]\n",inet_ntoa(addr),ntohs(port),
          str_time(s_time));
  pthread_mutex_unlock(&print_mutex);
very_end:
  tcpflush(client);
  if (read_s!=NULL) free(read_s);
  if (input!=NULL) free(input);
  if (output!=NULL) free(output);
  if (identifier!=NULL) free(identifier);
  if (client!=NULL) tcpclose(client);
  else close(connfd);
#ifdef VERBOSE
  sprintf(info, "Finishing thread %s:%d [%s]\n",
          inet_ntoa(addr),ntohs(port),
          str_time(s_time));
  pthread_mutex_lock(&print_mutex);
  fprintf(stderr,"%s",info);
  pthread_mutex_unlock(&print_mutex);
#endif
  return NULL;
}

void sig_usr1(int signo) {
  signal_from_usr1 = 1;
}

void sig_term(int signo) {
  signal_from_term = 1;
}

void get_arguments(int argc, char **argv) {
  int i = 1;
  int error = 0;
  char *argv0;

  continue_clients = 0;
  while (i<argc && !error) {
    if (strcmp(argv[i],"-c")==0) {
      continue_clients = 1;
      i++;
    } else if (strcmp(argv[i],"-r")==0) {
      socket_reuseadrr = 1;
      i++;
#ifdef LIBWRAP
    } else if (strcmp(argv[i],"-w")==0) {
      use_wrappers = 1;
      i++;
#endif
    } else if (strncmp(argv[i],"-t",2)==0) {
      if (strlen(argv[i])==2) {
        if (i+1<argc) {
          max_timeout = atoi(argv[i+1]);
          i+=2;
        }
        else error = 1;
      } else {
        max_timeout = atoi(argv[i]+2);
        i++;
      }
    } else if (strncmp(argv[i],"-s",2)==0) {
      if (strlen(argv[i])==2) {
        if (i+1<argc) {
          max_simultaneous = atoi(argv[i+1]);
          i+=2;
        }
        else error = 1;
      } else {
        max_simultaneous = atoi(argv[i]+2);
        i++;
      }
    } else if (strncmp(argv[i],"-q",2)==0) {
      if (strlen(argv[i])==2) {
        if (i+1<argc) {
          strcpyalloc(&queue_file_name,argv[i+1]);
          i+=2;
        }
        else error = 1;
      } else {
        strcpyalloc(&queue_file_name,argv[i]+2);
        i++;
      }
    } else break;
  }

  if (i<argc && !error) {
    password = NULL;
    strcpyalloc(&password,argv[i]);
    strcatalloc(&password,"\r\n");
    i++;
  }
  else error = 1;

  if (i<argc && !error) {
    port_nr = atoi(argv[i]);
    i++;
  }
  else error = 1;

  if (i<argc && !error) {
    timeout = atoi(argv[i]);
    i++;
  }
  else error = 1;

  if (error || i!=argc) {
    argv0 = strrchr(argv[0],'/');
    if (argv0==NULL) argv0 = argv[0];
    else argv0++;
    fprintf(stderr,"usage: %s [-c] [-r]",argv0);
#ifdef LIBWRAP
    fprintf(stderr," [-w]");
#endif
    fprintf(stderr," [-t number] [-s number] [-q filename] pass-key port time-out\n");
    exit(1);
  }
}

int main (int argc, char **argv) {
  int listenfd;
  struct sockaddr_in servaddr;
  socklen_t slen;
  pthread_t tid;
  arg_pass_type *arg;
  char s_time[1000];
  int reuseaddr = 1;
#ifdef LIBWRAP
  struct request_info request;
#endif

  get_arguments(argc,argv);

  if (signal(SIGUSR1,sig_usr1)==SIG_ERR ||
      signal(SIGTERM,sig_term)==SIG_ERR ||
      signal(SIGPIPE,SIG_IGN)==SIG_ERR) {
    fprintf(stderr,"Cannot set signal\n");
    exit(1);
  }

  setlinebuf(stdout);

  if (db_create(&queue_db,NULL,0)!=0 ||
      queue_db->open(queue_db,NULL,NULL,DB_BTREE,DB_CREATE|DB_THREAD,0) !=0 ||
      db_create(&use_db,NULL,0)!=0 ||
      use_db->open(use_db,NULL,NULL,DB_BTREE,DB_CREATE|DB_THREAD,0) !=0) {
    fprintf(stderr,"Unable to create database\n");
    exit(1);
  }

  fill_queue();

  pthread_create(&tid,NULL,repeat_de_queue,NULL);
  pthread_create(&tid,NULL,repeat_print_queue_and_die,NULL);

  listenfd=socket(AF_INET,SOCK_STREAM,0);

/* While this is good for allowing one to immediately restart the server
   if it exits for some reason, it does have security implications in which
   another local user on the server computer could hijack the connections.
*/
  if (socket_reuseadrr) {
    if (setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,
                   &reuseaddr,sizeof(int))!=0) {
      perror("Cannot set socket option\n");
      exit(1);
    }
  }

  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family=AF_INET;
  servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
  servaddr.sin_port=htons(port_nr);
  if (bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
  {
    perror("Could not bind");
    exit(1);
  }
  listen(listenfd,6);

  printf("Starting [%s]\n",str_time(s_time));

  stdinfile = stdinfdopen();

  while (1)
  {
    slen=sizeof(servaddr);
    arg = malloc(sizeof(arg_pass_type));
    arg->connfd = accept(listenfd,(struct sockaddr*)&servaddr,&slen);
    if (arg->connfd>=0) {
      arg->addr = servaddr.sin_addr;
      arg->port = servaddr.sin_port;
#ifdef LIBWRAP
      if (!use_wrappers ||
          hosts_access(request_init(&request,RQ_DAEMON,"dispense",RQ_FILE,arg->connfd,0)))
#endif
        pthread_create(&tid,NULL,process_client,(void*) arg);
#ifdef LIBWRAP
      else {
        pthread_mutex_lock(&print_mutex);
        fprintf(stderr,"Connection attempt from %s:%d [%s]\n",
                inet_ntoa(arg->addr),ntohs(arg->port),str_time(s_time));
        pthread_mutex_unlock(&print_mutex);
        close(arg->connfd);
      }
#endif
    }
  }
}
