/*
 * Copyright (c) 2000 by Stephen Montgomery-Smith <stephen@math.missouri.edu>
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

char *description = 
"This program attempts to find all the ways of placing all five tetrominoes\n"
"and all twelve pentominoes into a rectangle.\n"
"\n";

#define nr_polyominoes 17
#define polyomino_len 5
#define area_rectangle 80

#define LEN(poly_no) len[poly_no]

#define BLANK_REGION_TEST		\
    while (c>=0 && c%4!=0) c -= 5;	\
    if (c < 0)

#define client_search_method 1
#define client_check_size_of_blank_regions 0

#define create_inputs_search_method 2
#define create_inputs_check_size_of_blank_regions 1

#define CHECK_PROGRESS

#include "polyomino.h"

void make_polyomino() {
  int i, k=0;

  load_tetromino;
  load_pentomino;
  limit_rotations();
}

void carve_out_shape() {
}
