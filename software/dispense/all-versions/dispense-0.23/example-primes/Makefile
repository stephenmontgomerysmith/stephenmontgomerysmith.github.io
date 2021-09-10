CC=cc -O6 -W -Wall

all:	count-primes

count-primes:	count-primes.c
	${CC} count-primes.c -o count-primes -lm

count-primes.c:	count-primes-template.c list make-count-primes
	./make-count-primes list

clean:
	rm -f count-primes *core count-primes.c
