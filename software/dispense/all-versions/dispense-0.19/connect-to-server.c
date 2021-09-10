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

#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <tcpio.h>

TCPFILE *connect_to_server(char *hostname, int port_nr) {
  struct sockaddr_in servaddr;
  struct hostent *hptr;
  int sockfd;
  TCPFILE *f;

  signal(SIGPIPE,SIG_IGN);

  hptr=gethostbyname(hostname);
  if (hptr==NULL)
  {
    perror("Host not known");
    return NULL;
  }
  if (hptr->h_addrtype != AF_INET)
  {
    perror("Unknown address type");
    return NULL;
  }

  sockfd=socket(AF_INET,SOCK_STREAM,0);
  if (sockfd < 0)
  {
    perror("Unable to open socket");
    return NULL;
  }

  bzero(&servaddr,sizeof(servaddr));
  servaddr.sin_family=AF_INET;
  servaddr.sin_port=htons(port_nr);
/*
  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
  {
    close(sockfd);
    perror("Unusable IP address");
    return NULL;
  }
*/
  memcpy(&servaddr.sin_addr,*(hptr->h_addr_list),sizeof(struct in_addr));

  if (connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
  {
    close(sockfd);
    perror("Unable to connect");
    return NULL;
  }

  f = tcpfdopen(sockfd);
  if (f==NULL) {
    close(sockfd);
    perror("Unable to create file");
  }
  return f;
}
