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

#undef SIMPLE

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
#ifndef SIMPLE
#include <pthread.h>
#endif

/* This is the suite of functions via which enact programs communicate
   with the server.  The APIs are three functions, prepare_enact which
   sets everything up, get_dispensed which gets data from the server,
   and send_enacted which sends data back to the server.

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

   communicate_with_server also periodically calls query_queue which checks
   with the server which items on its queue are timed out, and asks for an
   extension for the timeout on the items in its queue.

   If SIMPLE is defined, then the functions are done more simply, without any
   queues or threads or using select.  This is in hope that it will be easy to
   port to Windows.
*/

typedef struct {
  char *input;
  char *output;
  int being_computed;
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
  char *identifier;
  char *save_filename;
  char *intermediate_to_send;
  int nr_intermediate_to_send;
} global_info_t;

global_info_t glob;

#ifndef SIMPLE
pthread_mutex_t glob_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t data_ready_cond = PTHREAD_COND_INITIALIZER;

static void *communicate_with_server(void *arg);
#endif

static void queue_to_file();
static void file_to_queue();

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
#ifndef SIMPLE
  pthread_t tid;
#endif
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
  glob.identifier = NULL;
  glob.save_filename = NULL;

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
    } else if (strncmp(argv[i],"-i",2)==0) {
      if (strlen(argv[i])==2) {
        if (i+1<argc) {
          strcpyalloc(&glob.identifier,argv[i+1]);
          i+=2;
        }
        else error = 1;
      } else {
        strcpyalloc(&glob.identifier,argv[i]+2);
        i++;
      }
    } else if (strncmp(argv[i],"-s",2)==0) {
      if (strlen(argv[i])==2) {
        if (i+1<argc) {
          strcpyalloc(&glob.save_filename,argv[i+1]);
          i+=2;
        }
        else error = 1;
      } else {
        strcpyalloc(&glob.save_filename,argv[i]+2);
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
    fprintf(stderr,"usage: %s [-f] [-n number] [-q number] [-t number] [-i identifier] [-s save-filename]",argv0);
    for (n=0;n<strlen(extra_options);n++)
      fprintf(stderr, " [-%c]",extra_options[n]);
    for (n=0;n<nr_extra;n++)
      fprintf(stderr," %s",arg_descr[n]);
    fprintf(stderr," pass-key hostname port\n");
    exit(1);
  }

#ifdef SIMPLE
  if (glob.queue_size!=1) {
    fprintf(stderr,"You are only allowed a queue size of 1.\n");
    exit(1);
  }
#endif

  free(arg_descr);
  free(arg_ptr);

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

  if (setpriority(PRIO_PROCESS,0,nice)<0) {
    fprintf(stderr,"Cannot set nice level\n");
    exit(1);
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
    glob.io_queue[m].being_computed = 0;
  }
  glob.queue_len = 0;
  glob.next_to_do = 0;
  glob.retry_count = 0;

  glob.intermediate_to_send = NULL;
  glob.nr_intermediate_to_send = 0;

  file_to_queue();

#ifndef SIMPLE
  pthread_create(&tid,NULL,communicate_with_server,NULL);
#endif

  return return_options;
}

#ifndef SIMPLE
static int get_input(void);
static int send_output(void);
static int send_out_intermediate(void);
static int query_queue(void);

static void *communicate_with_server(void *arg) {
  int recent_communication = 0;
  struct timespec abstime;
  time_t time_of_query = 0;

  pthread_detach(pthread_self());

  pthread_mutex_lock(&glob_mutex);
  while (1) {
    if (time(NULL)>time_of_query+3600) {
      if (query_queue()>=0)
        time_of_query = time(NULL);
    }
    if(glob.queue_len<glob.queue_size) {
      if (get_input()==0) {
        pthread_cond_broadcast(&data_ready_cond);
        recent_communication = 1;
      }
    } else if (glob.next_to_do>0) {
      if (send_output()==0)
        recent_communication = 1;
    } else if (glob.nr_intermediate_to_send>0 && glob.intermediate_to_send!=NULL) {
      if (send_out_intermediate()==0)
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
#endif

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
  if (glob.identifier==NULL) {
    if (tcpputs("connection from enact\r\n",glob.server)<0) goto retry;
  } else {
    if (tcpputs("connection from enact: ",glob.server)<0) goto retry;
    if (tcpputs(glob.identifier,glob.server)<0) goto retry;
    if (tcpputs("\r\n",glob.server)<0) goto retry;
  }

  if (tcpputs(glob.password,glob.server)<0) goto retry;
  sprintf(s,"%d\r\n",glob.timeout_request);
  if (tcpputs(s,glob.server)<0) goto retry;
  if (tcpflush(glob.server)<0) goto retry;
}

static int get_input() {
  char *in2 = NULL;

#ifndef SIMPLE
  pthread_mutex_unlock(&glob_mutex);
#endif
  establish_connection_with_server();
  if (tcpputs("request data\r\n",glob.server)<0) goto error;
  if (tcpflush(glob.server)<0) goto error;
  if (tcpgetsalloc(&in2,TCP_MAX_LINE,glob.server)==NULL ||
      rn2n(&in2)<0) goto error;
  if (strcmp(in2,"Finished\n")==0) {
    fprintf(stderr,"Program closed by server.\n");
    exit(0);
  }
  if (strcmp(in2,"No data\n")==0) goto error;
#ifndef SIMPLE
  pthread_mutex_lock(&glob_mutex);
#endif
  strcpyalloc(&glob.io_queue[glob.queue_len].input,in2);
  glob.queue_len++;
  queue_to_file();
  if (in2!=NULL) free(in2);
  return 0;
error:
  if (in2!=NULL) free(in2);
#ifndef SIMPLE
  pthread_mutex_lock(&glob_mutex);
#endif
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
#ifndef SIMPLE
  pthread_mutex_unlock(&glob_mutex);
#endif
  if (n2rn(&in2)<0) goto error;
  if (n2rn(&out2)<0) goto error;
  establish_connection_with_server();
  if (tcpputs("sending data\r\n",glob.server)<0) goto error;
  if (tcpputs(in2,glob.server)<0) goto error;
  if (tcpputs(out2,glob.server)<0) goto error;
  if (tcpflush(glob.server)<0) goto error;
  if (in2!=NULL) free(in2);
  if (out2!=NULL) free(out2);
#ifndef SIMPLE
  pthread_mutex_lock(&glob_mutex);
#endif
/* It is important that we rotate out the queue elements rather than overwrite
   them, because they contain pointers that point to valid regions of data. */
  memcpy(&temp,&glob.io_queue[0],sizeof(io_info_t));
  for (i=0;i<glob.queue_len-1;i++)
    memcpy(&glob.io_queue[i],&glob.io_queue[i+1],sizeof(io_info_t));
  memcpy(&glob.io_queue[glob.queue_len-1],&temp,sizeof(io_info_t));
  glob.queue_len--;
  glob.next_to_do--;
  queue_to_file();
  return 0;
error:
  if (in2!=NULL) free(in2);
  if (out2!=NULL) free(out2);
#ifndef SIMPLE
  pthread_mutex_lock(&glob_mutex);
#endif
  tcpclose(glob.server);
  glob.server = NULL;
  return -1;
}

#ifndef SIMPLE
static int send_out_intermediate() {
  char send_line[1000];

  pthread_mutex_unlock(&glob_mutex);
  establish_connection_with_server();
  pthread_mutex_lock(&glob_mutex);
  sprintf(send_line,"sending intermediate %d\r\n",glob.nr_intermediate_to_send);
  if (tcpputs(send_line,glob.server)<0) goto error;
  if (tcpputs(glob.intermediate_to_send,glob.server)<0) goto error;
  if (tcpflush(glob.server)<0) goto error;
  glob.nr_intermediate_to_send = 0;
  free(glob.intermediate_to_send);
  glob.intermediate_to_send = NULL;
  queue_to_file();
  return 0;
error:
  tcpclose(glob.server);
  glob.server = NULL;
  return -1;
}
#endif

void get_dispensed(char **input) {
#ifndef SIMPLE
  struct timespec abstime;

  pthread_mutex_lock(&glob_mutex);
  while (glob.next_to_do==glob.queue_len) {
    abstime.tv_sec = time(NULL)+60;
    abstime.tv_nsec = 0;
    pthread_cond_timedwait(&data_ready_cond,&glob_mutex,&abstime);
  }
  strcpyalloc(input,glob.io_queue[glob.next_to_do].input);
  glob.io_queue[glob.next_to_do].being_computed = 1;
  pthread_mutex_unlock(&glob_mutex);
#else
  if(glob.queue_len<glob.queue_size)
    while (get_input()<0) /* do nothing */;
  glob.retry_count = 0;
  strcpyalloc(input,glob.io_queue[glob.next_to_do].input);
#endif
}

void send_enacted(char *output) {
#ifndef SIMPLE
  pthread_mutex_lock(&glob_mutex);
  strcpyalloc(&glob.io_queue[glob.next_to_do].output,output);
  glob.io_queue[glob.next_to_do].being_computed = 0;
  glob.next_to_do++;
  queue_to_file();
  pthread_cond_broadcast(&data_ready_cond);
  pthread_mutex_unlock(&glob_mutex);
#else
  strcpyalloc(&glob.io_queue[glob.next_to_do].output,output);
  glob.next_to_do++;
  queue_to_file();
  while (send_output()<0) /* do nothing */;
  glob.retry_count = 0;
#endif
}

#ifndef SIMPLE
void send_intermediate(char *output) {
  pthread_mutex_lock(&glob_mutex);
  if (glob.intermediate_to_send==NULL)
    strcpyalloc(&(glob.intermediate_to_send),"");
  strcatalloc(&(glob.intermediate_to_send),
              glob.io_queue[glob.next_to_do].input);
  n2rn(&(glob.intermediate_to_send));
  strcatalloc(&(glob.intermediate_to_send),output);
  n2rn(&(glob.intermediate_to_send));
  (glob.nr_intermediate_to_send)++;
  queue_to_file();
  pthread_cond_broadcast(&data_ready_cond);
  pthread_mutex_unlock(&glob_mutex);
}
#endif

static void queue_to_file() {
  FILE *save_file;
  char *temp_filename=NULL;
  int i;

  if (glob.save_filename==NULL) return;
  strcpyalloc(&temp_filename,glob.save_filename);
  strcatalloc(&temp_filename,".tmp");
  save_file = fopen(temp_filename,"w");
  if (save_file==NULL) {
    perror("Could not open save file\n");
    exit(1);
  }

  fprintf(save_file,"%d\n%d\n",glob.queue_len,glob.next_to_do);
  for (i=0;i<glob.queue_len;i++) {
    fprintf(save_file,"%s",glob.io_queue[i].input);
    if (i<glob.next_to_do)
      fprintf(save_file,"%s",glob.io_queue[i].output);
  }

  if (glob.nr_intermediate_to_send>0 && glob.intermediate_to_send!=NULL) {
    fprintf(save_file,"%d\n%s",glob.nr_intermediate_to_send,glob.intermediate_to_send);
  }

  if (fclose(save_file)<0) {
    perror("Could not close save file");
    exit(1);
  }
  if (rename(temp_filename,glob.save_filename)<0) {
    perror("Could not rename save file");
    exit(1);
  }
  if (temp_filename!=NULL) free(temp_filename);
}

static void file_to_queue() {
  FILE *save_file;
  int i;
  char *data=NULL;

  if (glob.save_filename==NULL) return;
  save_file = fopen(glob.save_filename,"r");
  if (save_file==NULL) return;

  fgetsalloc(&data,save_file);
  if (sscanf(data,"%d",&(glob.queue_len))!=1 || glob.queue_len>glob.queue_size
      || glob.queue_len<0) {
    fprintf(stderr,"unusuable queue file\n");
    exit(1);
  }
  fgetsalloc(&data,save_file);
  if (sscanf(data,"%d",&(glob.next_to_do))!=1 || glob.next_to_do>glob.queue_len
      || glob.next_to_do<0) {
    fprintf(stderr,"unusuable queue file\n");
    exit(1);
  }
  for (i=0;i<glob.queue_len;i++) {
    fgetsalloc(&(glob.io_queue[i].input),save_file);
    if (glob.io_queue[i].input==NULL) {
      fprintf(stderr,"unusuable queue file\n");
      exit(1);
    }
    if (i<glob.next_to_do) {
      fgetsalloc(&(glob.io_queue[i].output),save_file);
      if (glob.io_queue[i].output==NULL) {
        fprintf(stderr,"unusuable queue file\n");
        exit(1);
      }
    }
    glob.io_queue[i].being_computed = 0;
  }

  if (fgetsalloc(&data,save_file)!=NULL) {
    if (sscanf(data,"%d",&(glob.nr_intermediate_to_send))!=1
      || glob.nr_intermediate_to_send<0) {
      fprintf(stderr,"unusuable queue file\n");
      exit(1);
    }
    for (i=0;i<glob.nr_intermediate_to_send;i++) {
      fgetsalloc(&data,save_file);
      if (data==NULL) {
        fprintf(stderr,"unusuable queue file\n");
        exit(1);
      }
      if (i==0) strcpyalloc(&(glob.intermediate_to_send),data);
      else      strcatalloc(&(glob.intermediate_to_send),data);
    }
  }

  if (fclose(save_file)<0) {
    perror("Could not close save file");
    exit(1);
  }
  if (data!=NULL) free(data);
}

char *enact_save_filename() {
  return glob.save_filename;
}

#ifndef SIMPLE

static int query_queue() {
  char *in2=NULL, out[1024];
  int i,k;
  io_info_t temp;

  if (glob.queue_len==glob.next_to_do) return 0;
  pthread_mutex_unlock(&glob_mutex);
  establish_connection_with_server();
  pthread_mutex_lock(&glob_mutex);
  if (glob.queue_len==glob.next_to_do) return 0;
  sprintf(out,"query %d\r\n",glob.queue_len-glob.next_to_do);
  if (tcpputs(out,glob.server)<0) goto error;
  for (k=glob.next_to_do;k<glob.queue_len;k++) {
    strcpyalloc(&in2,glob.io_queue[k].input);
    if (n2rn(&in2)<0) goto error;
    if (tcpputs(in2,glob.server)<0) goto error;
  }
  if (tcpflush(glob.server)<0) goto error;
  if (tcpgetsalloc(&in2,TCP_MAX_LINE,glob.server)==NULL) goto error;
  if (!rn2n(&in2)<0 || (int)strlen(in2)!=glob.queue_len-glob.next_to_do+1) goto error;
  for (i=glob.next_to_do,k=glob.next_to_do;i<glob.queue_len;i++) {
    if (in2[i-glob.next_to_do]=='y' || glob.io_queue[i].being_computed==1) {
      if (i!=k) {
/* It is important that we rotate out the queue elements rather than overwrite
   them, because they contain pointers that point to valid regions of data. */
        memcpy(&temp,&glob.io_queue[i],sizeof(io_info_t));
        memcpy(&glob.io_queue[i],&glob.io_queue[k],sizeof(io_info_t));
        memcpy(&glob.io_queue[k],&temp,sizeof(io_info_t));
      }
      k++;
    }
  }
  glob.queue_len = k;
  if (i!=k) queue_to_file();
  if (in2!=NULL) free(in2);
  return 0;
error:
  pthread_mutex_unlock(&glob_mutex);
  if (in2!=NULL) free(in2);
  tcpclose(glob.server);
  glob.server = NULL;
  pthread_mutex_lock(&glob_mutex);
  return -1;
}

#endif
