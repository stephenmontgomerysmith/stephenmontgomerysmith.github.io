#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <limits.h>
#include <errno.h>

#define BUF_DELTA (1<<16)

typedef struct {
  int in_fd, out_fd;
  char *in_buffer;
  size_t in_begin;
  size_t in_end;
  size_t in_allocated;
  char *out_buffer;
  size_t out_begin;
  size_t out_end;
  size_t out_allocated;
  int error;
} BPFILE;


/* A BPFILE is a buffering wrapper around a pair of file descriptors, one
   for reading, and one for writing.  Typically they will be ends of pipes,
   both going to the same process.  A write operation is always accompanied
   by a read operation, so that the other process doesn't get into a log-jam.
*/

/* Creates a new BPFILE using the given file descriptors.  Returns NULL on
   failure. */
BPFILE *bpfdopen(int in_fd, int out_fd);

/* Flush and close the output file descriptor.  Often this is necessary
   before the process will finish writing back to the input file descriptor.
   Returns a negative number on failure.
*/
int bpclose_out(BPFILE *f);

/* Flush and close and deallocate the BPFILE.  Returns a negative number
   on failure.
*/
int bpclose(BPFILE *f);

/* Flush the output part of the BPFILE.  Returns a negative number on failure.
*/
int bpflush(BPFILE *f);

/* Get a "\n" terminated string from the input part of the BPFILE.  If end
   of file is encountered, the string might not be "\n" terminated.  Sets *s
   to NULL if end of file is encountered before any more characters are 
   encountered, or if there is an error.
   It assumes that *s is either NULL, or a pointer created by malloc or such
   like.  It realloc's *s to the appropriate size.
   The return value is *s.
   It is implemented as a macro.

char *bpgetsalloc(char **s, BPFILE *f);
*/

/* Just like bpgetsalloc, except it will not block if there is no string
   available, in which case *s points to the empty string "".

char *bpgetsalloc_no_block(char **s, BPFILE *f);
*/

/* Puts the string pointed to by s into the input part of the BPFILE. */
int bpputs(char *s, BPFILE *f);

/* Returns 1 if eof was encountered in the input part of BPFILE (and there
   is no error), otherwise returns 0.
*/
int bpeof_in(BPFILE *f);

/* Returns 1 if an error happened in a previous BPFILE operation.
*/
int bperror(BPFILE *f);

/* General Description of Buffering Process:

   The buffers are reallocated as necessary so that they are sufficiently
   large.  It is particularly important that the read buffer be allowed to
   read as much as required, so that the writing doesn't get blocked in the
   other process we are communicating with.

   Each of the two buffers (in_buffer and out_buffer) has three variables
   associated with it, begin, end and allocated.  Allocated is the amount
   of memory allocated to it, and begin and end mark the beginning and end of
   the useful data.  Information is added to the buffer (by read or bpputs
   at the end, and read from the buffer (by write or bpgets..) at the
   beginning.

   The program is sure to keep allocated at least as large as end, allocating
   more space if necessary.  It also tries to keep begin less than BUF_DELTA -
   if this value is exceeded, then all the stuff in the buffer is memmoved
   to the beginning of buffer, and begin is set to 0.  (These memmoves don't
   seem to add a lot of overhead.)

   Also, the program (in bpputs) attempts to keep out_end-out_begin less than
   BUF_DELTA.
*/

BPFILE *bpfdopen(int in_fd, int out_fd) {
  BPFILE *f;

  if (fcntl(in_fd,F_SETFL,O_NONBLOCK|fcntl(in_fd,F_GETFL))<0 ||
      fcntl(out_fd,F_SETFL,O_NONBLOCK|fcntl(out_fd,F_GETFL))<0)
    return NULL;
  f = malloc(sizeof(BPFILE));
  if (f==NULL) {
    fprintf(stderr,"Allocation error\n");
    exit(1);
  }
  f->in_fd = in_fd;
  f->out_fd = out_fd;
  f->in_buffer = NULL;
  f->in_begin = 0;
  f->in_end = 0;
  f->in_allocated = 0;
  f->out_buffer = NULL;
  f->out_begin = 0;
  f->out_end = 0;
  f->out_allocated = 0;
  f->error = 0;
  return f;
}

/* This is the work-horse of this package.  It looks at the read and write
   file descriptors of the BPFILE, and reads from them/writes to them if
   they are ready.  The reading and writing is done to and from the buffers.
*/


int bpread_write(BPFILE *f, int block) {
  fd_set readfds,writefds;
  int nfds = f->in_fd>f->out_fd?f->in_fd+1:f->out_fd+1;
  struct timeval t = {0,0};
  ssize_t r;

  if (f->error) return -1;
  if (f->in_fd<0 && (f->out_fd<0||f->out_end==f->out_begin)) return 0;
  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  if (f->in_fd>=0) FD_SET(f->in_fd,&readfds);
  if (f->out_fd>=0 && f->out_end>f->out_begin) FD_SET(f->out_fd,&writefds);
  select(nfds, &readfds, &writefds, NULL, block?NULL:&t);

  if (f->in_fd>=0 && FD_ISSET(f->in_fd,&readfds)) {
    if (f->in_allocated-f->in_end<BUF_DELTA) {
      f->in_allocated+=BUF_DELTA;
      f->in_buffer=realloc(f->in_buffer,f->in_allocated);
      if (f->in_buffer==NULL) {
        fprintf(stderr,"Allocation error\n");
        exit(1);
      }
    }
    r = read(f->in_fd,f->in_buffer+f->in_end,f->in_allocated-f->in_end);
    if (r<0) {
      if (errno!=EAGAIN && errno!=EINTR) {
        f->error = 1;
        return -1;
      }
    }
    else if (r==0) {
      close(f->in_fd);
      f->in_fd = -1;
    }
    else f->in_end+=r;
  }
  if (f->out_fd>=0 && FD_ISSET(f->out_fd,&writefds)) {
/*
    r = write(f->out_fd,f->out_buffer+f->out_begin,
              (f->out_end-f->out_begin)>PIPE_BUF?
              PIPE_BUF:(f->out_end-f->out_begin));
*/
    r = write(f->out_fd,f->out_buffer+f->out_begin,
              (f->out_end-f->out_begin));
    if (r<0) {
      if (errno!=EAGAIN && errno!=EINTR) {
        f->error = 1;
        return -1;
      }
    }
    else {
      f->out_begin+=r;
      if (f->out_begin>=BUF_DELTA) {
        memmove(f->out_buffer,f->out_buffer+f->out_begin,
                f->out_end-f->out_begin);
        f->out_end -= f->out_begin;
        f->out_begin = 0;
        f->out_allocated = f->out_end + BUF_DELTA;
        f->out_buffer = realloc(f->out_buffer,f->out_allocated);
        if (f->out_buffer==NULL) {
          fprintf(stderr,"Allocation error\n");
          exit(1);
        }
      }
    }
  }
  return 0;
}

int bpflush(BPFILE *f) {
  if (f->error) return -1;
  while (f->out_end>f->out_begin)
    if (bpread_write(f,1)<0) return -1;
  return 0;
}

char *bpgetsalloc_blockq(char **s, BPFILE *f, int block) {
  char *eol;
  int go_again;
  int len;

  if (f->error) return NULL;
  if (f->in_fd<0 && f->in_end==f->in_begin)  {
    if (*s!=NULL) {
      free(*s);
      *s = NULL;;
    }
    return NULL;
  }
  eol = memchr(f->in_buffer+f->in_begin,'\n',f->in_end-f->in_begin);
  do {
    go_again = 0;
    if (eol==NULL) {
      if (bpread_write(f,block)<0) {
        if (*s!=NULL) {
          free(*s);
          *s = NULL;
        }
      }
    }
    eol = memchr(f->in_buffer+f->in_begin,'\n',f->in_end-f->in_begin);
    if (eol==NULL) {
      if (f->in_fd<0) {
        if (f->in_end==f->in_begin) {
          if (*s!=NULL) {
            free(*s);
            *s = NULL;
          }
        }
        else {
          *s = realloc(*s,f->in_end-f->in_begin+1);
          if (*s==NULL) {
            fprintf(stderr,"Allocation error\n");
            exit(1);
          }
          memmove(*s,f->in_buffer+f->in_begin,f->in_end-f->in_begin);
          (*s)[f->in_end-f->in_begin] = '\0';
          f->in_begin = f->in_end;
        }
        if (f->in_buffer!=NULL) {
          free(f->in_buffer);
          f->in_buffer = NULL;
          f->in_allocated = 0;
        }
      }
      else {
        if (block) go_again = 1;
        else {
          *s = realloc(*s,1);
          if (*s==NULL) {
            fprintf(stderr,"Allocation error\n");
            exit(1);
          }
          strcpy(*s,"");
        }
      }
    }
    else {
      len = eol-(f->in_buffer+f->in_begin)+1;
      *s = realloc(*s,len+1);
      if (*s==NULL) {
        fprintf(stderr,"Allocation error\n");
        exit(1);
      }
      memmove(*s,f->in_buffer+f->in_begin,len);
      (*s)[len] = '\0';
      f->in_begin += len;
      if (f->in_begin>=BUF_DELTA) {
        memmove(f->in_buffer,f->in_buffer+f->in_begin,
                f->in_end-f->in_begin);
        f->in_end -= f->in_begin;
        f->in_begin = 0;
        f->in_allocated = f->in_end + BUF_DELTA;
        f->in_buffer = realloc(f->in_buffer,f->in_allocated);
        if (f->in_buffer==NULL) {
          fprintf(stderr,"Allocation error\n");
          exit(1);
        }
      }
    }
  } while(go_again);
  return *s;
}

#define bpgetsalloc(s,f) bpgetstring_blockq(s, f, 1)

#define bpgetsalloc_no_block(s,f) bpgetstring_blockq(s, f, 0)

int bpeof_in(BPFILE *f) {
  return(!f->error && f->in_fd<0);
}

int bperror(BPFILE *f) {
  return f->error;
}

int bpputs(char *s, BPFILE *f) {
  int len = strlen(s);

  if (f->error) return -1;
  if (f->out_end+len>f->out_allocated) {
    f->out_allocated = f->out_end+len+BUF_DELTA;
    f->out_buffer = realloc(f->out_buffer,f->out_allocated);
    if (f->out_buffer==NULL) {
      fprintf(stderr,"Allocation error\n");
      exit(1);
    }
  }
  memmove(f->out_buffer+f->out_end,s,len);
  f->out_end += len;
  while (f->out_end-f->out_begin>=BUF_DELTA)
    if (bpread_write(f,1)<0) return -1;
  return 0;
}

int bpclose_out(BPFILE *f) {
  int r = 0, s = 0;

  r = bpflush(f);
  if (f->out_fd>=0) {
    s = close(f->out_fd)<0;
    f->out_fd = -1;
  }
  if (f->out_buffer!=NULL) {
    free(f->out_buffer);
    f->out_buffer = NULL;
  }
  return r<0||s<0?-1:0;
}

int bpclose(BPFILE *f) {
  int r = 0, s = 0;

  r = bpclose_out(f);
  if (f->in_fd>=0)
    s = close(f->in_fd);
  if (f->in_buffer!=NULL) {
    free(f->in_buffer);
    f->in_buffer = NULL;
  }
  free(f);
  return r<0||s<0?-1:0;
}
