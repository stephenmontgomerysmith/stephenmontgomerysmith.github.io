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
  char in_buffer[BUF_SIZE];
  size_t in_begin;
  size_t in_end;
  char out_buffer[BUF_SIZE];
  size_t out_end;
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

/* The input buffering in this suite is done by the get functions (like
   tcpgetsalloc).  It is a very simple buffer.  As much stuff as available
   is read into the buffer.  Then the get functions read from this buffer
   until it is all exhausted, at which time as much stuff as is available
   is again read into the buffer.  There are two variables associated
   with the buffer, in_begin and in_end.  in_end is determined by how much is
   read into the buffer.  in_begin is set to 0 when stuff is read into the
   buffer, and updated as stuff is read from the buffer.

   The output buffering is very very simple.
*/

TCPFILE *tcpfdopen(int fd) {
  TCPFILE *f;

  f = malloc(sizeof(TCPFILE));
  if (f==NULL) {
    fprintf(stderr,"Allocation error\n");
    exit(1);
  }
  f->fd = fd;
  f->in_begin = 0;
  f->in_end = 0;
  f->out_end = 0;
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
    if (f->in_end==f->in_begin) {
      f->in_begin = 0;
      r = read(f->fd,f->in_buffer,BUF_SIZE);
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
        f->in_end = r;
      }
    }
    if (f->in_end>f->in_begin) {
      eol = memchr(f->in_buffer+f->in_begin,'\n',f->in_end-f->in_begin);
      eol_found = eol!=NULL;
      if (!eol_found) eol = f->in_buffer+f->in_end;
      copy_amount = eol-(f->in_buffer+f->in_begin) + eol_found;
      if (at+copy_amount<len) {
        *s = realloc(*s,at+copy_amount+1);
        if (*s==NULL) {
          fprintf(stderr,"Allocation error\n");
          exit(1);
        }
        memmove((*s)+at,f->in_buffer+f->in_begin,copy_amount);
        at += copy_amount;
      }
      else
        at = len;
      f->in_begin+=copy_amount;
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

int tcpflush(TCPFILE *f) {
  size_t at; 
  int r;

  if (f->error || f->fd<0) return -1;
  at = 0;
  while (at<f->out_end) {
    r = write(f->fd,f->out_buffer+at,f->out_end-at);
    if (r<0) {
      if (errno!=EINTR) {
        f->error = 1;
        return -1;
      }
      else
        r = 0;
    }
    at += r;
  }
  f->out_end = 0;
  return 0;
}


int tcpputs(char *s, TCPFILE *f) {
  size_t at, left, copy_amt;

  if (f->error || f->fd<0) return -1;
  at = 0;
  left = strlen(s);
  while(left>0) {
    if (left > BUF_SIZE-f->out_end)
      copy_amt = BUF_SIZE-f->out_end;
    else
      copy_amt = left;
    memmove(f->out_buffer+f->out_end,s+at,copy_amt);
    f->out_end += copy_amt;
    at += copy_amt;
    left -= copy_amt;
    if (f->out_end >= BUF_SIZE)
      if (tcpflush(f) < 0)
        return -1;
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
