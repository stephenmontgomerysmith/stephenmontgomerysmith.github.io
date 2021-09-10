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

#include <bipipeio.h>
#include <tcpio.h>
#include <stdio.h>

#define TCP_MAX_LINE 50000 /* The longest length allowed for lines sent via
                              TCP/IP. */

char *fgetsalloc(char **s, FILE *f);

TCPFILE *connect_to_server(char *hostname, int port_nr);
int n2rn(char **s);
int rn2n(char **s);
char *strcpyalloc(char **dest, char *src);
char *strcatalloc(char **dest, char *src);

char *str_time(char *s);

char *prepare_enact(int argc, char **argv, char *extra_options,
                    unsigned int nr_extra, ...);
void get_dispensed(char **input);
void send_enacted(char *output);
void send_intermediate(char *output);
char *enact_save_filename();
