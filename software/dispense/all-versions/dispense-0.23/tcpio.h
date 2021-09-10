#define BUF_SIZE (1<<16)

typedef struct {
  int fd;
  char in_buffer[BUF_SIZE];
  size_t in_begin;
  size_t in_end;
  int error, eof;
} TCPFILE;

TCPFILE *tcpfdopen(int fd);
int tcpclose(TCPFILE *f);
char *tcpgetsalloc(char **s, int len, TCPFILE *f);
int tcpflush(TCPFILE *f);
int tcpputs(char *s, TCPFILE *f);
int tcpeof(TCPFILE *f);
int tcperror(TCPFILE *f);
