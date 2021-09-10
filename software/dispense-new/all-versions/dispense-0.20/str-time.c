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

#include <time.h>
#include <stdio.h>

/* Puts an ascii version of the current time (including time zone info)
   into s. */

char *str_time(char *s) {
  time_t clock;
  char s_clock[26];
  struct tm t_clock;
  int zone;

  clock=time(NULL);
  if (clock != (time_t)-1) {
    localtime_r(&clock,&t_clock);
    asctime_r(&t_clock,s_clock);
    if (strlen(s_clock)>0) s_clock[strlen(s_clock)-1] = '\0';
    zone = abs(t_clock.tm_gmtoff);
    sprintf(s,"%s %s%d:%d%d",
            s_clock,
            t_clock.tm_gmtoff>=0?"":"-",
             zone/3600,
             (zone%3600)/600,
             (zone%600)/60);
  } else
    strcpy(s,"Time not available");
  return s;
}
