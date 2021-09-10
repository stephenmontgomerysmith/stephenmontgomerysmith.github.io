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

#include "bipipeio.h"
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>

/* Returns a stream that reads and writes to the stdout and stdin from the
   program program_name.  Returns NULL if there is any error.

   It uses the /dev/pty?? and /dev/tty?? to communicate with program_name.
   This is so that stdout in program_name is line buffered.
*/

BPFILE *open_program_pty(char *program_name);

/* Returns a stream that reads and writes to the stdout and stdin from the
   program program_name.  Returns NULL if there is any error.
*/

BPFILE *open_program(char *program_name);

BPFILE *open_program_pty(char *program_name) {
  pid_t pid;
  char name[] = "/dev/ptyXX";
  char *cp1, *cp2;
  int master = -1, slave = -1;
  BPFILE *f = NULL;

  for (cp1 = "pqrsPQRS"; *cp1; cp1++) {
    name[8] = *cp1;
    for (cp2 = "0123456789abcdefghijklmnopqrstuv"; *cp2; cp2++) {
      name[5] = 'p';
      name[9] = *cp2;
      if ((master = open(name, O_RDWR, 0))==-1) {
        if (errno==ENOENT) goto error; /* out of ptys */
      } else {
        name[5] = 't';
        if ((slave = open(name,O_RDWR,0))!=-1) goto done;
        close(master);
      }
    }
  }
done:

  if ((pid=fork())<0) goto error;
  else if (pid==0) {
    close(master);
    if (setpgid(0,getpid())>=0 &&
        dup2(slave,STDIN_FILENO)>=0 &&
        dup2(slave,STDOUT_FILENO)>=0)
      execlp("/bin/sh","sh","-c",program_name,NULL);
    _exit(127);
  }

  if (close(slave)<0) goto error;
  slave = -1;
  setpgid(pid,pid);
  f = bpfdopen(master,master);
  if (f==NULL) goto error;
  return f;

error:
  if (f!=NULL)
    bpclose(f);
  else
    if (master>=0) close(master);
  if (slave>=0) close(slave);
  return NULL;
}

BPFILE *open_program(char *program_name) {
  pid_t pid;
  int in_fd[2] = {-1,-1};
  int out_fd[2] = {-1,-1};
  BPFILE *f = NULL;

  if(pipe(in_fd)<0 || pipe(out_fd)<0) goto error;

  if ((pid=fork())<0) goto error;
  else if (pid==0) {
    close(in_fd[1]);
    close(out_fd[0]);
    if (setpgid(0,getpid())>=0 &&
        dup2(in_fd[0],STDIN_FILENO)>=0 &&
        dup2(out_fd[1],STDOUT_FILENO)>=0)
      execlp("/bin/sh","sh","-c",program_name,NULL);
    _exit(127);
  }

  if (close(in_fd[0])<0) goto error;
  in_fd[0] = -1;
  if (close(out_fd[1])<0) goto error;
  out_fd[1] = -1;
  setpgid(pid,pid);
  f = bpfdopen(out_fd[0],in_fd[1]);
  if (f==NULL) goto error;
  return f;

error:
  if (f!=NULL)
    bpclose(f);
  else {
    if (in_fd[1]>=0) close(in_fd[1]);
    if (out_fd[0]>=0) close(out_fd[0]);
  }
  if (in_fd[0]>=0) close(in_fd[0]);
  if (out_fd[1]>=0) close(out_fd[1]);
  return NULL;
}
