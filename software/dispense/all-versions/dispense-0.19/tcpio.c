#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <limits.h>
#include <errno.h>

#define BUF_SIZE (1<<16)

typedef struct {
  int fd;
  char buffer[BUF_SIZE];
  size_t begin;
  size_t end;
  int error, eof;
} TCPFILE;


/* A TCPFILE is a buffering wrapper around a file descriptor that is a
   tcp/ip connection.
*/

/* Creates a new TCPFILE using the given file descriptor.  Returns NULL on
   failure. */
TCPFILE *tcpfdopen(int fd);

/* Close and deallocate the TCPFILE.  Returns a negative number
   on failure.
*/
int tcpclose(TCPFILE *f);

/* Get a "\n" terminated string from the input part of the TCPFILE.  If it 
   cannot find a "\n" terminated string of length less than len, or before
   end of file, *s is NULL, and sets the error flag.  If it reachs end of
   file before encountering any other characters, *s is NULL.  Returns *s,
   which is reallocated or freed as necessary.
*/
char *tcpgetsalloc(char **s, int len, TCPFILE *f);

/* Writes out the string pointed to by s into the TCPFILE. */
int tcpputs(char *s, TCPFILE *f);

/* Returns 1 if eof was encountered in the input part of TCPFILE (and there
   is no error), otherwise returns 0.
*/
int tcpeof(TCPFILE *f);

/* Returns 1 if an error happened in a previous TCPFILE operation.
*/
int tcperror(TCPFILE *f);

/* The only buffering in this suite is done by the get functions (like
   tcpgetsalloc).  It is a very simple buffer.  As much stuff as available
   is read into the buffer.  Then the get functions read from this buffer
   until it is all exhausted, at which time as much stuff as is available
   is again read into the buffer.  There are two variables associated
   with the buffer, begin and end.  end is determined by how much is read
   into the buffer.  begin is set to 0 when stuff is read into the buffer,
   and updated as stuff is read from the buffer.
*/

TCPFILE *tcpfdopen(int fd) {
  TCPFILE *f;

  f = malloc(sizeof(TCPFILE));
  if (f==NULL) {
    fprintf(stderr,"Allocation error\n");
    exit(1);
  }
  f->fd = fd;
  f->begin = 0;
  f->end = 0;
  f->error = 0;
  return f;
}

char *tcpgetsalloc(char **s, int len, TCPFILE *f) {
  int at, eol_found, copy_amount, r;
  char *eol;

  if (f->error || f->fd<0) return NULL;
  at = 0;
  eol_found = 0;
  while (1) {
    if (f->end==f->begin) {
      f->begin = 0;
      r = read(f->fd,f->buffer,BUF_SIZE);
      if (r<0 && errno!=EINTR) {
        f->error = 1;
        if (*s!=NULL) {
          free(*s);
          *s = NULL;
        }
        return *s;
      }
      else if (r==0) {
        close(f->fd);
        f->fd = -1;
        if (*s!=NULL) {
          free(*s);
          *s = NULL;
        }
        return *s;
      }
      else {
        f->end = r;
      }
    }
    if (f->end>f->begin) {
      eol = memchr(f->buffer+f->begin,'\n',f->end-f->begin);
      eol_found = eol!=NULL;
      if (!eol_found) eol = f->buffer+f->end;
      copy_amount = eol-(f->buffer+f->begin) + eol_found;
      if (at+copy_amount<len) {
        *s = realloc(*s,at+copy_amount+1);
        if (*s==NULL) {
          fprintf(stderr,"Allocation error\n");
          exit(1);
        }
        memmove((*s)+at,f->buffer+f->begin,copy_amount);
        at += copy_amount;
      }
      else
        at = len;
      f->begin+=copy_amount;
    }
    if (eol_found) {
      if (at<len) {
        (*s)[at] = '\0';
        return *s;
      }
      else {
        f->error = 1;
        if (*s!=NULL) {
          free(*s);
          *s = NULL;
        }
        return *s;
      }
    }
  }
}

int tcpputs(char *s, TCPFILE *f) {
  int at, left, r;

  at = 0;
  left = strlen(s);
  while (left>0) {
    r = write(f->fd,s+at,left);
    if (r<0 && errno!=EINTR) {
      f->error = 0;
      return -1;
    }
    at += r;
    left -= r;
  }
  return 0;
}

int tcpeof(TCPFILE *f) {
  return(!f->error && f->fd<0);
}

int tcperror(TCPFILE *f) {
  return f->error;
}

int tcpclose(TCPFILE *f) {
  int r = 0;

  if (f->fd>=0)
    r = close(f->fd);
  free(f);
  return r;
}
