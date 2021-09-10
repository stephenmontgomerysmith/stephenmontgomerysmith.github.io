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
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* N is the length of the interval to work on with each batch.
   Should be an even number. 
   30 N should be less that the largest signed integer.  If there is a
   desire to make N larger than this requirement, some of the code below
   will need to be changed so that int becomes long long as appropriate.

   The absolute largest numbers this program can check is N^2.
*/
#define N 10000000
#define NRBITS ((N+1)/2)
#define USEMEM ((NRBITS+7)/8)

/* MAX_NEG - if checking whether p, p-a_1, ...,  p-a_k are primes,
   this at least as large as the maximum value of a_k. */
#define MAX_NEG $max_neg
#define MAX_NEG_BITS (MAX_NEG<<1)
#define MAX_NEG_MEM ((MAX_NEG_BITS+7)/8)

/* The number of batches to do each time. */
#define BATCH 100

int *small_prime, nr_small_primes;

/*
bit n of bitarray represents 2n+1, or base*N+2n+1
(and n might be negative, down to -MAX_NEG).
*/
unsigned char whole_bit_array[USEMEM+MAX_NEG_MEM];
unsigned char *bitarray=whole_bit_array+MAX_NEG_MEM;
#define TEST(b,n) (b[(n)>>3] & 1<<((n)&0x7))
#define SET(b,n) (b)[(n)>>3] |= 1<<((n)&0x7)

/* Compute primes up to N. */

void make_small_primes() {
  int m,n;
  int max_n;

  max_n = (floor(sqrt(N))-1)/2;
  bzero(whole_bit_array,sizeof(whole_bit_array));
  for (n=1;n<=max_n;n++)
    if (!TEST(bitarray,n)) {
      for (m=n+(2*n+1);m<NRBITS;m+=(2*n+1))
        SET(bitarray,m);
    }

  nr_small_primes = 0;
  for (n=1;n<NRBITS;n++)
    if (!TEST(bitarray,n))
      nr_small_primes++;

  small_prime = malloc((nr_small_primes+1)*sizeof(int));
  m = 0;
  for (n=1;n<NRBITS;n++)
    if (!TEST(bitarray,n)) {
      small_prime[m] = 2*n+1;
      m++;
    }
  small_prime[m] = 1<<30;
}

/* Compute primes in [base*N-MAX_NEG,(base+1)*N).
   It requires that base<N .*/

void make_large_primes(int base) {
  int *m;
  int max_p;
  int start,n,s2;
  long long s;

/* max_p = sqrt(N*(base+1)).  (+1 to be safe for rounding errors). */
  max_p = sqrt((double)N * (double)(base+1)) + 1;
  bzero(whole_bit_array,sizeof(whole_bit_array));
  if (base==0) for (n=0;n>=-MAX_NEG_BITS;n--)
    SET(bitarray,n);
  s = (long long)N * (long long)base;
  for (m=small_prime;*m<=max_p;m++) {
    if (base==0) {
      start = 3*(*m);
      for (n=start>>1;n<NRBITS;n+=*m)
        SET(bitarray,n);
    }
/* No need to set these bits, as they are never checked.
    else if (*m <= 5) {
      s2 = -s%((long long)(2*(*m)));
      start = s2 + (*m);
      while (start >= 1-MAX_NEG)
        start -= 2*(*m);
      start += 2*(*m);
      for (n=start>>1;n<NRBITS;n+=*m)
        SET(bitarray,n);
    }
*/
    else {
/* By looking at everything mod 30, we are able to hard code away certain
   checks knowing that they are never needed.  This seems to result in
   noticable time savings. */
      s2 = -s%((long long)(30*(*m)));
#define check_mod30(k) \
      start = s2 + k*(*m); \
      while (start >= 1-MAX_NEG) \
        start -= 30*(*m); \
      start += 30*(*m); \
      for (n=start>>1;n<NRBITS;n+=15*(*m)) \
        SET(bitarray,n)
      check_mod30(1);
      check_mod30(7);
      check_mod30(11);
      check_mod30(13);
      check_mod30(17);
      check_mod30(19);
      check_mod30(23);
      check_mod30(29);
    }
  }
}

/* Count how many primes in the interval [base*N,(base+1)*N) have the
   required properties. */

void count_large_primes(int *count, int base);

/* count_primes tests all of the numbers.  count_primesxx only tests those
   numbers that are xx mod 30 (so a lot of the tests are known in advance
   to fail, and so we do not test for them).
*/

void count_primes(int *count) {
  int n;
  int $count_dec;

  for (n=0;n<NRBITS;n++) if (!TEST(bitarray,n)) {$test}
}

void count_primes01(int *count, int base) {
  int n;
  int $count_dec01;
  int start;

  start = 1 - ((long long)N*(long long)base)%30l;
  if (start<=0) start+=30;
  for (n=start>>1;n<NRBITS;n+=15) if (!TEST(bitarray,n)) {$test01}
}

void count_primes07(int *count, int base) {
  int n;
  int $count_dec07;
  int start;

  start = 7 - ((long long)N*(long long)base)%30l;
  if (start<=0) start+=30;
  for (n=start>>1;n<NRBITS;n+=15) if (!TEST(bitarray,n)) {$test07}
}

void count_primes11(int *count, int base) {
  int n;
  int $count_dec11;
  int start;

  start = 11 - ((long long)N*(long long)base)%30l;
  if (start<=0) start+=30;
  for (n=start>>1;n<NRBITS;n+=15) if (!TEST(bitarray,n)) {$test11}
}

void count_primes13(int *count, int base) {
  int n;
  int $count_dec13;
  int start;

  start = 13 - ((long long)N*(long long)base)%30l;
  if (start<=0) start+=30;
  for (n=start>>1;n<NRBITS;n+=15) if (!TEST(bitarray,n)) {$test13}
}

void count_primes17(int *count, int base) {
  int n;
  int $count_dec17;
  int start;

  start = 17 - ((long long)N*(long long)base)%30l;
  if (start<=0) start+=30;
  for (n=start>>1;n<NRBITS;n+=15) if (!TEST(bitarray,n)) {$test17}
}

void count_primes19(int *count, int base) {
  int n;
  int $count_dec19;
  int start;

  start = 19 - ((long long)N*(long long)base)%30l;
  if (start<=0) start+=30;
  for (n=start>>1;n<NRBITS;n+=15) if (!TEST(bitarray,n)) {$test19}
}

void count_primes23(int *count, int base) {
  int n;
  int $count_dec23;
  int start;

  start = 23 - ((long long)N*(long long)base)%30l;
  if (start<=0) start+=30;
  for (n=start>>1;n<NRBITS;n+=15) if (!TEST(bitarray,n)) {$test23}
}

void count_primes29(int *count, int base) {
  int n;
  int $count_dec29;
  int start;

  start = 29 - ((long long)N*(long long)base)%30l;
  if (start<=0) start+=30;
  for (n=start>>1;n<NRBITS;n+=15) if (!TEST(bitarray,n)) {$test29}
}

void count_large_primes(int *count, int base) {
  make_large_primes(base);

  if (base==0)
    count_primes(count);
  else {
    count_primes01(count,base);
    count_primes07(count,base);
    count_primes11(count,base);
    count_primes13(count,base);
    count_primes17(count,base);
    count_primes19(count,base);
    count_primes23(count,base);
    count_primes29(count,base);
  }
}

int main() {
  int base, b;
  int count[$nr_counts];
  int c;
  char input[1000];

  setlinebuf(stdout);

  make_small_primes();

  fprintf(stderr,
"This program counts the number of primes p between\n"
"%ub and %u(b+1)\n"
"for which p, p-a_1,...,p-a_k are prime, for various lists a_1,...,a_k.\n"
"Enter b (must be less than %u):\n",
N*BATCH,N*BATCH,N/BATCH);

  while (1) {
    if (fgets(input,1000,stdin)==NULL) exit(0);
    if (sscanf(input,"%d",&b)!=1)
      printf("Invalid input\n");
    else if (b>=N/BATCH)
      printf("Input too large\n");
    else if (b<0)
      printf("Input is negative\n");
    else {
      for (c=0;c<$nr_counts;c++)
        count[c] = 0;
      for (base=b*BATCH;base<b*BATCH+BATCH;base++)
        count_large_primes(count,base);
      for (c=0;c<$nr_counts;c++) {
        if (c!=0) printf(" ");
        printf("%d",count[c]);
      }
      printf("\n");
    }
  }
}
