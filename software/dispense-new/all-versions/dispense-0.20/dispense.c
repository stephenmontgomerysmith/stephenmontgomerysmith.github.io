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

#undef VERBOSE

char rules_file_name[] = "dispense.allow";
char *password = NULL;
int timeout;
int max_timeout=1<<30;
int max_simultaneous=1<<30;
char *queue_file_name = NULL;
int port_nr;
int continue_clients = 0;
int finished = 0;
int signal_from_usr1 = 0;
int signal_from_usr2 = 0;
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

#ifdef VERBOSE
  fprintf(stderr,"De-queue - %s\n",regular?"regular":"not regular");
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
      queue_data.timeout--;
    }

#ifdef VERBOSE
    pthread_mutex_lock(&print_mutex);
    fprintf(stderr,"De-queue: %d %d %s",queue_data.timeout,
            queue_data.simultaneous, (char*)key.data);
    pthread_mutex_unlock(&print_mutex);
#endif

    if (queue_data.timeout<0) {
      if (regular) queue_data.simultaneous=0;
      if (queue_db->del(queue_db,NULL,&key,0) != 0 ||
          use_db->put(use_db,NULL,&key,&data,0) != 0) {
        fprintf(stderr,"Unable to delete from or put in db\n");
        exit(1);
      }
    }
    else {
      if (queue_db->del(queue_db,NULL,&key,0) != 0 ||
          queue_db->put(queue_db,NULL,&key,&data,0) != 0) {
        fprintf(stderr,"Unable to delete from or put in db\n");
        exit(1);
      }
    }
  }
  if (cursor->c_close(cursor)!=0) {
    fprintf(stderr,"Cannot close cursor\n");
    exit(1);
  }

  if (!ret_val) {
    if (use_db->cursor(use_db,NULL,&cursor,0) != 0) {
      fprintf(stderr,"Cannot create cursor for use database\n");
      exit(1);
    }
    ret_val = cursor->c_get(cursor,&key,&data,DB_NEXT)==0;
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

  if (queue_db->put(queue_db,NULL,&key,&data,0) != 0) {
    fprintf(stderr,"Unable to put to db\n");
    exit(1);
  }
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
          fprintf(stderr, "duplicate inputs %s\n",*input);
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
  int query_number,i;
  char query_answer[259];

  free(arg);

  pthread_detach(pthread_self());

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
    if (signal_from_usr1) {
      signal_from_usr1 = 0;
      set_finished("Project terminated with SIGUSR1",0 /*don't exit*/);
    }

    if (tcpgetsalloc(&read_s,TCP_MAX_LINE,client)==NULL) goto very_end;

    if (strcmp(read_s,"request data\r\n")==0) {
      while (1) {
        if (!check_finished()) {
          if (get_from_use_db(&input,local_timeout)) break;
          stdin_status = get_from_stdin(&input,local_timeout);
          if (stdin_status==1) break;
          if (stdin_status==2)  goto very_end;
          de_queue_status = de_queue(0);
          if (de_queue_status==0)
            set_finished("Finished",continue_clients);
          else if (de_queue_status==2)
            goto very_end;
        }
        else {
          strcpyalloc(&input,"Finished\n");
          break;
        }
      }
      n2rn(&input);
      tcpputs(input,client);
#ifdef VERBOSE
      sprintf(info, "Data to %s:%d [%s]\n",inet_ntoa(addr),ntohs(port),
              str_time(s_time));
      fprintf(stderr,"%s%s",info,input);
#endif
    }

    else if (strcmp(read_s,"sending data\r\n")==0) {
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
/* Important to put the following mutex unlock here rather than slightly
   earlier, to avoid a possible race condition in which program might
   quit before writing data to stdout. */
      pthread_mutex_unlock(&db_mutex); 
    }

    else if (strncmp(read_s,"query ",6)==0 
             && strcmp(read_s+strlen(read_s)-2,"\r\n")==0) {
      query_number = atoi(read_s+6);
      if (query_number<1 || query_number>256) goto end;
      for (i=0;i<query_number;i++) {
        if (tcpgetsalloc(&input,TCP_MAX_LINE,client)==NULL ||
            rn2n(&input)<0) goto end;
        pthread_mutex_lock(&db_mutex);
        key.data = input;
        key.size = strlen(input)+1;
        found = queue_db->del(queue_db,NULL,&key,0)==0 ||
                use_db->del(use_db,NULL,&key,0)==0;
        query_answer[i] = found?'y':'n';
        add_to_queue_db(input,max_timeout,1);
        pthread_mutex_unlock(&db_mutex);
      }
      query_answer[query_number] = '\r';
      query_answer[query_number+1] = '\n';
      query_answer[query_number+2] = '\0';
      tcpputs(query_answer,client);
    }

    else goto end;
  }
end:
  pthread_mutex_lock(&print_mutex);
  fprintf(stderr, "Bad protocol from %s:%d [%s]\n",inet_ntoa(addr),ntohs(port),
          str_time(s_time));
  pthread_mutex_unlock(&print_mutex);
very_end:
  if (read_s!=NULL) free(read_s);
  if (input!=NULL) free(input);
  if (output!=NULL) free(output);
  if (identifier!=NULL) free(identifier);
  if (client!=NULL) tcpclose(client);
  else close(connfd);
  return NULL;
}

void sig_usr1(int signo) {
  signal_from_usr1 = 1;
}

void sig_usr2(int signo) {
  signal_from_usr2 = 1;
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

  password = NULL;
  if (i<argc) {
    strcpyalloc(&password,argv[i]);
    strcatalloc(&password,"\r\n");
    i++;
  }
  else error = 1;

  if (i<argc) {
    port_nr = atoi(argv[i]);
    i++;
  }
  else error = 1;

  if (i<argc) {
    timeout = atoi(argv[i]);
    i++;
  }
  else error = 1;

  if (error || i!=argc) {
    argv0 = strrchr(argv[0],'/');
    if (argv0==NULL) argv0 = argv[0];
    else argv0++;
    fprintf(stderr,"usage: %s [-c] [-t number] [-s number] [-q filename]",argv0);
    fprintf(stderr," pass-key port time-out\n");
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
  FILE *queue_file;
  char *queue_line=NULL;
/*int reuseaddr = 1; */

  get_arguments(argc,argv);

  if (signal(SIGUSR1,sig_usr1)==SIG_ERR ||
      signal(SIGUSR2,sig_usr2)==SIG_ERR ||
      signal(SIGTERM,sig_term)==SIG_ERR ||
      signal(SIGPIPE,SIG_IGN)==SIG_ERR) {
    fprintf(stderr,"Cannot set signal\n");
    exit(1);
  }

  if (get_rules(rules_file_name)<0) {
    fprintf(stderr,"Malformed %s\n",rules_file_name);
    exit(1);
  }
  fprintf(stderr,"Rules for who may connect:\n");
  print_rules();

  setlinebuf(stdout);

  if (db_create(&queue_db,NULL,0)!=0 ||
      queue_db->open(queue_db,NULL,NULL,DB_BTREE,DB_CREATE|DB_THREAD,0) !=0 ||
      db_create(&use_db,NULL,0)!=0 ||
      use_db->open(use_db,NULL,NULL,DB_BTREE,DB_CREATE|DB_THREAD,0) !=0) {
    fprintf(stderr,"Unable to create database\n");
    exit(1);
  }

  if (queue_file_name != NULL) {
    queue_file = fopen(queue_file_name,"r");
    while (fgetsalloc(&queue_line,queue_file)!=NULL)
      add_to_queue_db(queue_line,max_timeout,1);
  }

  pthread_create(&tid,NULL,repeat_de_queue,NULL);
  pthread_create(&tid,NULL,repeat_print_queue_and_die,NULL);

  listenfd=socket(AF_INET,SOCK_STREAM,0);

/* While this is good for allowing one to immediately restart the server
   if it exits for some reason, it does have security implications in which
   another local user on the server computer could hijack the connections.
  if (setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,
                 &reuseaddr,sizeof(int))!=0) {
    perror("Cannot set socket option\n");
    exit(1);
  }
*/

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
    if (signal_from_usr2) {
      pthread_mutex_lock(&print_mutex);
      if (get_rules(rules_file_name)<0)
        fprintf(stderr,"Malformed %s\n",rules_file_name);
      fprintf(stderr,"Rules for who may connect [%s]:\n",str_time(s_time));
      print_rules();
      pthread_mutex_unlock(&print_mutex);
      signal_from_usr2 = 0;
      signal(SIGUSR2,sig_usr2);
    }
    if (arg->connfd>=0) {
      arg->addr = servaddr.sin_addr;
      arg->port = servaddr.sin_port;
      if (check_rule(ntohl(arg->addr.s_addr)))
        pthread_create(&tid,NULL,process_client,(void*) arg);
      else {
        pthread_mutex_lock(&print_mutex);
        fprintf(stderr,"Connection attempt from %s:%d [%s]\n",
                inet_ntoa(arg->addr),ntohs(arg->port),str_time(s_time));
        pthread_mutex_unlock(&print_mutex);
        close(arg->connfd);
      }
    }
  }
}
