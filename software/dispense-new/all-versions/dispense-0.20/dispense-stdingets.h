#define BUF_DELTA (4096)

typedef struct {
  char *buffer;
  size_t length;
  size_t allocated;
  int error, eof;
} STDINFILE;

STDINFILE *stdinfdopen();
int stdinclose(STDINFILE *f);
char *stdingetsalloc(char **s, STDINFILE *f);
int stdineof(STDINFILE *f);
int stdinerror(STDINFILE *f);
