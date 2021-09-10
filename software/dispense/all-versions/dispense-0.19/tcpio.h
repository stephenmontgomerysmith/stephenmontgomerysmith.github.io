#define BUF_SIZE (1<<16)

typedef struct {
  int fd;
  char buffer[BUF_SIZE];
  size_t begin;
  size_t end;
  int error, eof;
} TCPFILE;

TCPFILE *tcpfdopen(int fd);
int tcpclose(TCPFILE *f);
char *tcpgetsalloc(char **s, int len, TCPFILE *f);
int tcpputs(char *s, TCPFILE *f);
int tcpeof(TCPFILE *f);
int tcperror(TCPFILE *f);
