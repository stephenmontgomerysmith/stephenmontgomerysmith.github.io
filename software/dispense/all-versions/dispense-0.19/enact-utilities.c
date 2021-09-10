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
#include <stdarg.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>

/* This is the suite of functions via which enact programs communicate
   with the server.  The APIs are three functions, prepare_enact which
   sets everything up, get_dispensed which gets data from the server,
   and send enacted which sends data back to the server.

   This is how it works.  The global data glob contains most everything that
   is needed - including a queue, io_queue, of input and output values.
   The queue is controled by queue_size (the maximum length of the queue),
   queue_len (the actual number of items in the queue), and next_to_do (the
   index of the item that the program should be working on).

   get_dispensed returns the value of the input that is indexed by next_to_do,
   and send_enacted copies the output to the item indexed by next_to_do,
   and then increments next_to_do.

   Meanwhile, there is a background thread - communicate_with_server.  If there
   is space left in the queue, it fetches more input lines from the server.
   If next_to_do is positive, it sends input/ouput lines back to the server.
*/

typedef struct {
  char *input;
  char *output;
} io_info_t;


typedef struct {
  char *hostname;
  int port_nr;
  char *password;
  TCPFILE *server;
  int timeout_request;
  int queue_size;
  int queue_len;
  int next_to_do;
  io_info_t *io_queue;
  int retry_count;
} global_info_t;

global_info_t glob;

pthread_mutex_t glob_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_ready_cond = PTHREAD_COND_INITIALIZER;

static void *communicate_with_server(void *arg);

char *prepare_enact(int argc, char **argv, char *extra_options,
                    unsigned int nr_extra, ...) {
  int i=1;
  int foreground = 0;
  int nice = 20;
  int error = 0;
  char *argv0;
  unsigned int n;
  char **arg_descr, ***arg_ptr;
  va_list ap;
  int nullfd;
  char *return_options = malloc(strlen(extra_options)+1);
  int nr_return_options = 0;
  pthread_t tid;
  int m;

  if ((arg_descr=malloc(nr_extra*sizeof(char*)))==NULL ||
      (arg_ptr=malloc(nr_extra*sizeof(char**)))==NULL) {
    fprintf(stderr,"Allocation error\n");
    exit(1);
  }
  va_start(ap,nr_extra);
  for (n=0;n<nr_extra;n++)
    if ((arg_descr[n] = va_arg(ap,char*))==NULL ||
        (arg_ptr[n] = va_arg(ap,char**))==NULL) {
      fprintf(stderr,"Internal error\n");
      exit(1);
    }
  va_end(ap);

  glob.queue_size = 1;
  glob.timeout_request = 0;

  while (i<argc && !error) {
    if (strcmp(argv[i],"-f")==0) {
      foreground = 1;
      i++;
    } else if (strncmp(argv[i],"-n",2)==0) {
      if (strlen(argv[i])==2) {
        if (i+1<argc) {
          nice = atoi(argv[i+1]);
          i+=2;
        }
        else error = 1;
      } else {
        nice = atoi(argv[i]+2);
        i++;
      }
    } else if (strncmp(argv[i],"-q",2)==0) {
      if (strlen(argv[i])==2) {
        if (i+1<argc) {
          glob.queue_size = atoi(argv[i+1]);
          i+=2;
        }
        else error = 1;
      } else {
        glob.queue_size = atoi(argv[i]+2);
        i++;
      }
    } else if (strncmp(argv[i],"-t",2)==0) {
      if (strlen(argv[i])==2) {
        if (i+1<argc) {
          glob.timeout_request = atoi(argv[i+1]);
          i+=2;
        }
        else error = 1;
      } else {
        glob.timeout_request = atoi(argv[i]+2);
        i++;
      }
    } else if (strlen(argv[i])==2 && argv[i][0]=='-' && 
               strchr(extra_options,argv[i][1])!=NULL && 
               strchr(return_options,argv[i][1])==NULL) {
      return_options[nr_return_options] = argv[i][1];
      nr_return_options++;
      i++;
    } else if (argv[i][0]=='-') {
      error = 1;
    } else break;
  }


  return_options[nr_return_options] = '\0';

  error = glob.timeout_request<0 || glob.queue_size<1;

  for (n=0;n<nr_extra && !error;n++)
    if (i<argc) {
      *(arg_ptr[n]) = argv[i];
      i++;
    }
    else error = 1;

  glob.password = NULL;
  if (i<argc) {
    strcpyalloc(&glob.password,argv[i]);
    strcatalloc(&glob.password,"\r\n");
    i++;
  }
  else error = 1;

  if (i<argc) {
    glob.hostname = argv[i];
    i++;
  }
  else error = 1;

  if (i<argc) {
    glob.port_nr = atoi(argv[i]);
    i++;
  }
  else error = 1;

  if (error || i!=argc) {
    argv0 = strrchr(argv[0],'/');
    if (argv0==NULL) argv0 = argv[0];
    else argv0++;
    fprintf(stderr,"usage: %s [-f] [-n number] [-q number] [-t number]",argv0);
    for (n=0;n<strlen(extra_options);n++)
      fprintf(stderr, " [-%c]",extra_options[n]);
    for (n=0;n<nr_extra;n++)
      fprintf(stderr," %s",arg_descr[n]);
    fprintf(stderr," pass-key hostname port\n");
    exit(1);
  }

  free(arg_descr);
  free(arg_ptr);

  if (setpriority(PRIO_PROCESS,0,nice)<0) {
    fprintf(stderr,"Cannot set nice level\n");
    exit(1);
  }

  if (!foreground) {
    switch (fork()) {
      case -1: fprintf(stderr,"Unable to go into background\n");
               exit(1);
      case 0: break;
      default: _exit(0);
    }
    nullfd = open("/dev/null",O_RDWR,0);
    if (nullfd<0 || setsid()<0) {
      fprintf(stderr, "Unable to go into background\n");
      exit(1);
    }
    dup2(nullfd,STDIN_FILENO);
    dup2(nullfd,STDOUT_FILENO);
    dup2(nullfd,STDERR_FILENO);
  }

  glob.server = NULL;

  glob.io_queue = malloc(glob.queue_size*sizeof(io_info_t));
  if (glob.io_queue==NULL) {
    fprintf(stderr,"Allocation error\n");
    exit(1);
  }
  for (m=0;m<glob.queue_size;m++) {
    glob.io_queue[m].input = NULL;
    glob.io_queue[m].output = NULL;
  }
  glob.queue_len = 0;
  glob.next_to_do = 0;
  glob.retry_count = 0;

  pthread_create(&tid,NULL,communicate_with_server,NULL);

  return return_options;
}

static int get_input(void);
static int send_output(void);

static void *communicate_with_server(void *arg) {
  int recent_communication = 0;
  struct timespec abstime;

  pthread_detach(pthread_self());

  pthread_mutex_lock(&glob_mutex);
  while (1) {
    if(glob.queue_len<glob.queue_size) {
      if (get_input()==0) {
        pthread_cond_broadcast(&data_ready_cond);
        recent_communication = 1;
      }
    } else if (glob.next_to_do>0) {
      if (send_output()==0)
        recent_communication = 1;
    } else {
      if (!recent_communication && glob.server!=NULL) {
        tcpclose(glob.server);
        glob.server = NULL;
        glob.retry_count = 0;
      }
      abstime.tv_sec = time(NULL)+60;
      abstime.tv_nsec = 0;
      if (pthread_cond_timedwait(&data_ready_cond,&glob_mutex,
                                 &abstime)==ETIMEDOUT) {
        recent_communication = 0;
      }
    }
  }
}

static void establish_connection_with_server(void) {
  char s[100];

  if (glob.server != NULL) return;
retry:
  if (glob.server!=NULL) tcpclose(glob.server);
/* retry every minute up to 2 days. */
  if (glob.retry_count>2*24*60) exit(1);
  if (glob.retry_count>0) sleep(60);
  glob.retry_count++;

  glob.server = connect_to_server(glob.hostname,glob.port_nr);
  if (glob.server==NULL) goto retry;
  if (tcpputs("connection from enact\r\n",glob.server)<0) goto retry;
  if (tcpputs(glob.password,glob.server)<0) goto retry;
  sprintf(s,"%d\r\n",glob.timeout_request);
  if (tcpputs(s,glob.server)<0) goto retry;
}

static int get_input() {
  char *in2 = NULL;

  pthread_mutex_unlock(&glob_mutex);
  establish_connection_with_server();
  if (tcpputs("request data\r\n",glob.server)<0) goto error;
  if (tcpgetsalloc(&in2,TCP_MAX_LINE,glob.server)==NULL ||
      rn2n(&in2)<0) goto error;
  if (strcmp(in2,"Finished\n")==0) {
    fprintf(stderr,"Program closed by server.\n");
    exit(0);
  }
  pthread_mutex_lock(&glob_mutex);
  strcpyalloc(&glob.io_queue[glob.queue_len].input,in2);
  glob.queue_len++;
  if (in2==NULL) free(in2);
  return 0;
error:
  if (in2==NULL) free(in2);
  pthread_mutex_lock(&glob_mutex);
  tcpclose(glob.server);
  glob.server = NULL;
  return -1;
}

static int send_output() {
  char *in2=NULL, *out2=NULL;
  int i;
  io_info_t temp;

  strcpyalloc(&in2,glob.io_queue[0].input);
  strcpyalloc(&out2,glob.io_queue[0].output);
  pthread_mutex_unlock(&glob_mutex);
  if (n2rn(&in2)<0) goto error;
  if (n2rn(&out2)<0) goto error;
  establish_connection_with_server();
  if (tcpputs("sending data\r\n",glob.server)<0) goto error;
  if (tcpputs(in2,glob.server)<0) goto error;
  if (tcpputs(out2,glob.server)<0) goto error;
  if (in2==NULL) free(in2);
  if (out2==NULL) free(out2);
  pthread_mutex_lock(&glob_mutex);
  memcpy(&temp,&glob.io_queue[0],sizeof(io_info_t));
  for (i=0;i<glob.queue_len-1;i++)
    memcpy(&glob.io_queue[i],&glob.io_queue[i+1],sizeof(io_info_t));
  memcpy(&glob.io_queue[glob.queue_len-1],&temp,sizeof(io_info_t));
  glob.queue_len--;
  glob.next_to_do--;
  return 0;
error:
  if (in2==NULL) free(in2);
  if (out2==NULL) free(out2);
  pthread_mutex_lock(&glob_mutex);
  tcpclose(glob.server);
  glob.server = NULL;
  return -1;
}

void get_dispensed(char **input) {
  struct timespec abstime;

  pthread_mutex_lock(&glob_mutex);
  while (glob.next_to_do==glob.queue_len) {
    abstime.tv_sec = time(NULL)+60;
    abstime.tv_nsec = 0;
    pthread_cond_timedwait(&data_ready_cond,&glob_mutex,&abstime);
  }
  strcpyalloc(input,glob.io_queue[glob.next_to_do].input);
  pthread_mutex_unlock(&glob_mutex);
}

void send_enacted(char *output) {
  pthread_mutex_lock(&glob_mutex);
  strcpyalloc(&glob.io_queue[glob.next_to_do].output,output);
  glob.next_to_do++;
  pthread_cond_broadcast(&data_ready_cond);
  pthread_mutex_unlock(&glob_mutex);
}
