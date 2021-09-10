#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <limits.h>
#include <errno.h>

#define BUF_DELTA (PIPE_BUF)

typedef struct {
  char *buffer;
  size_t length;
  size_t allocated;
  int error, eof;
} STDINFILE;

/* A STDINFILE is a buffering wrapper around non-blocking stdin.
*/

/* Creates a new TCPFILE using the given file descriptor.  Returns NULL on
   failure. */
STDINFILE *stdinfdopen();

/* Deallocate the STDINFILE.  Returns a negative number
   on failure.
*/
int stdinclose(STDINFILE *f);

/* Get a "\n" terminated string from stdin.  If it cannot find a "\n"
   terminated string before end of file, 
   *s is NULL, and sets the error flag.  If it reachs end of file before 
   encountering any other characters, *s is NULL.  If there is no string to
   return, but eof is not yet reached, *s points to "".  Returns *s,
   which is reallocated or freed as necessary.
*/
char *stdingetsalloc(char **s, STDINFILE *f);

/* Returns 1 if eof was encountered in the input part of TCPFILE (and there
   is no error), otherwise returns 0.
*/
int stdineof(STDINFILE *f);

/* Returns 1 if an error happened in a previous TCPFILE operation.
*/
int stdinerror(STDINFILE *f);

STDINFILE *stdinfdopen() {
  STDINFILE *f;
  int stdin_status;

  stdin_status = fcntl(STDIN_FILENO,F_GETFL);
  fcntl(STDIN_FILENO,F_SETFL,stdin_status|O_NONBLOCK);
  f = malloc(sizeof(STDINFILE));
  if (f==NULL) {
    fprintf(stderr,"Allocation error\n");
    exit(1);
  }
  f->buffer = NULL;
  f->length = 0;
  f->allocated = 0;
  f->error = 0;
  return f;
}

char *stdingetsalloc(char **s, STDINFILE *f) {
  int copy_amount, r, eagain, eof_found;
  char *eol;

  if (f->error) return NULL;
  eagain = 0;
  eof_found = 0;
  while (1) {
    if (f->allocated-f->length < BUF_DELTA)
      f->allocated = f->length+BUF_DELTA;
    f->buffer = realloc(f->buffer,f->allocated);
    if (f->buffer==NULL) {
      fprintf(stderr,"Allocation error\n");
      exit(1);
    }
    r = read(STDIN_FILENO,f->buffer+f->length,BUF_DELTA);
    if (r<0 && (errno==EAGAIN || errno==EINTR)) {
      eagain = 1;
    }
    else if (r<0) {
      f->error = 1;
      if (*s!=NULL) {
        free(*s);
        *s = NULL;
      }
      return *s;
    }
    else if (r==0) {
      eof_found = 1;
    }
    else {
      f->length += r;
    }

    if (f->length==0) {
      if (eof_found) {
        f->eof = 1;
        if (*s!=NULL) {
          free(*s);
          *s = NULL;
        }
      }
      if (eagain) {
        *s = realloc(*s,1);
        if (*s==NULL) {
          fprintf(stderr,"Allocation error\n");
          exit(1);
        }
        strcpy(*s,"");
      }
      return *s;
    }
    else {
      eol = memchr(f->buffer,'\n',f->length);
      if (eol!=NULL) {
        copy_amount = eol-f->buffer + 1;
        *s = realloc(*s,copy_amount+1);
        if (*s==NULL) {
          fprintf(stderr,"Allocation error\n");
          exit(1);
        }
        memmove(*s,f->buffer,copy_amount);
        (*s)[copy_amount] = '\0';
        f->length -= copy_amount;
        memmove(f->buffer,f->buffer+copy_amount,f->length);
        if (f->allocated-f->length > BUF_DELTA) {
          f->allocated = f->length;
          f->buffer = realloc(f->buffer,f->allocated);
          if (f->buffer==NULL) {
            fprintf(stderr,"Allocation error\n");
            exit(1);
          }
        }
        return *s;
      }
      else if (eof_found) {
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

int stdineof(STDINFILE *f) {
  return(!f->error);
}

int stdinerror(STDINFILE *f) {
  return f->error;
}

int stdinclose(STDINFILE *f) {
  free(f);
  return 0;
}
