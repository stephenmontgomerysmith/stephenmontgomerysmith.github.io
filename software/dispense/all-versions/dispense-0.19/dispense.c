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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include <db3/db.h>

char rules_file_name[] = "dispense.allow";
char *password = NULL;
int timeout;
int max_timeout=1<<30;
int port_nr;
int continue_clients = 0;
int finished = 0;
int signal_from_usr1 = 0;
int signal_from_usr2 = 0;

pthread_mutex_t print_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t db_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
  int connfd;
  struct in_addr addr;
  u_short port;
}
arg_pass_type;

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

int de_queue(void) {
  DBT key = {NULL,0,0,0,0,DB_DBT_REALLOC};
  DBT data = {NULL,0,0,0,0,DB_DBT_REALLOC};
  DBC *cursor;
  int *queue_time;
  int ret_val = 0;

  pthread_mutex_lock(&db_mutex);
  if (queue_db->cursor(queue_db,NULL,&cursor,0) != 0) {
    fprintf(stderr,"Cannot create cursor for queue database\n");
    exit(1);
  }
  while(cursor->c_get(cursor,&key,&data,DB_NEXT)==0) {
    ret_val = 1;
    queue_time = (int*)data.data;
    (*queue_time)--;
/*
    pthread_mutex_lock(&print_mutex);
    fprintf(stderr,"De-queue: %d %s",*queue_time, (char*)key.data);
    pthread_mutex_unlock(&print_mutex);
*/
    if (*queue_time<0) {
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
  if (data.data!=NULL) free(data.data);

  return ret_val;
}

void *repeat_de_queue(void *arg) {
  pthread_detach(pthread_self());
  while(1) {
    sleep(60);
    de_queue();
  }
}

void add_to_queue_db(char *input,int timeout) {
  DBT key = {input,strlen(input)+1,strlen(input)+1,0,0,DB_DBT_USERMEM};
  DBT data = {&timeout,sizeof(int),sizeof(int),0,0,DB_DBT_USERMEM};

  if (queue_db->put(queue_db,NULL,&key,&data,0) != 0) {
    fprintf(stderr,"Unable to put to db\n");
    exit(1);
  }
}

int get_from_use_db(char **input,int timeout) {
  DBC *cursor=NULL;
  DBT key = {NULL,0,0,0,0,DB_DBT_REALLOC};
  int queue_time;
  DBT data = {&queue_time,sizeof(int),sizeof(int),0,0,DB_DBT_USERMEM};
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
    add_to_queue_db(*input,timeout);
  }
  pthread_mutex_unlock(&db_mutex);
  if (key.data!=NULL) free(key.data);
  if (cursor!=NULL) cursor->c_close(cursor);
  return found;
}

int get_from_stdin(char **input,int timeout) {
  DBT key;
  int queue_time;
  DBT data = {&queue_time,sizeof(int),sizeof(int),0,0,DB_DBT_USERMEM};
  int found = 0;
  int duplicate;

  do {
    duplicate = 0;
    pthread_mutex_lock(&db_mutex);
    if (fgetsalloc(input,stdin)!=NULL && strlen(*input)>=1 &&
        (*input)[strlen(*input)-1]=='\n') {
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
        add_to_queue_db(*input,timeout);
      }
    }
    pthread_mutex_unlock(&db_mutex);
  } while (duplicate);
  return found;
}

void *process_client(void *arg) {
  int connfd = ((arg_pass_type*)arg)->connfd;
  struct in_addr addr = ((arg_pass_type*)arg)->addr;
  u_short port = ((arg_pass_type*)arg)->port;

  TCPFILE *client;
  char *read_s=NULL, *input=NULL, *output=NULL;
  char info[1000];
  char s_time[1000];
  DBT key;
  int found;
  int local_timeout;

  free(arg);

  pthread_detach(pthread_self());

  if ((client=tcpfdopen(connfd))==NULL) goto very_end;
  if (tcpgetsalloc(&read_s,TCP_MAX_LINE,client)==NULL) goto very_end;
  if (strcmp(read_s,"connection from enact\r\n")!=0) goto end;
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
          if (get_from_stdin(&input,local_timeout)) break;
          if (!de_queue())
            set_finished("Finished",continue_clients);
        }
        else {
          strcpyalloc(&input,"Finished\n");
          break;
        }
      }
      n2rn(&input);
      tcpputs(input,client);
    }

    else if (strcmp(read_s,"sending data\r\n")==0) {
      if (tcpgetsalloc(&input,TCP_MAX_LINE,client)==NULL ||
          rn2n(&input)<0) goto end;
      if (tcpgetsalloc(&output,TCP_MAX_LINE,client)==NULL ||
          rn2n(&output)<0) goto end;
      sprintf(info, "Data from %s:%d [%s]\n",inet_ntoa(addr),ntohs(port),
              str_time(s_time));
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
    fprintf(stderr,"usage: %s [-c] [-t number]",argv0);
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
/*int reuseaddr = 1; */

  get_arguments(argc,argv);

  if (signal(SIGUSR1,sig_usr1)==SIG_ERR ||
      signal(SIGUSR2,sig_usr2)==SIG_ERR ||
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

  pthread_create(&tid,NULL,repeat_de_queue,NULL);

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
