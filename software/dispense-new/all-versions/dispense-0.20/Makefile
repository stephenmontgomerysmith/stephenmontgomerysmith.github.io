CLIENT=enact
SERVER=dispense filter-input filter-against
OBJECT=string-utilities.o str-time.o fgetsalloc.o bipipeio.o tcpio.o \
       connect-to-server.o open-program.o enact-utilities.o firewall.o
INCLUDES=bipipeio.h dispense.h
CC=cc -W -Wall -D_THREAD_SAFE \
   -I. -I/usr/include -I/usr/local/include -L. -L/usr/lib -L/usr/local/lib

all:	all-enact all-dispense
	cd example-add; make

install:	install-enact install-dispense install-man

all-enact:	${CLIENT} libdispense.a

all-dispense:	${SERVER}

enact-utilities.o:	enact-utilities.c	${INCLUDES}
	${CC} ${CFLAGS} -c enact-utilities.c

dispense:	dispense.c dispense-stdingets.o libdispense.a ${INCLUDES}
	${CC} ${CFLAGS} dispense.c -o dispense dispense-stdingets.o -ldispense -ldb3 -pthread

enact:	enact.c libdispense.a ${INCLUDES}
	${CC} ${CFLAGS} enact.c -o enact -ldispense -pthread

filter-input:	filter-input.c libdispense.a
	${CC} ${CFLAGS} filter-input.c -o filter-input -ldispense -ldb3

filter-against:	filter-against.c libdispense.a
	${CC} ${CFLAGS} filter-against.c -o filter-against -ldispense -ldb3

libdispense.a:	${OBJECT}
	rm -f libdispense.a
	ar qc libdispense.a ${OBJECT}

README:	README.html
	lynx -dump README.html > README
	perl -p -i -e 's/\[\d+\]http/http/g' README
	perl -p -i -e 's/\[\d+\]dispense/dispense/g' README
	perl -i -lne '$$ref=1 if /References/;print if !$$ref' README

install-enact: all-enact install-man
	install -c enact /usr/local/bin
	cp -p libdispense.a /usr/local/lib
	cp -p dispense.h /usr/local/include
	cp -p bipipeio.h /usr/local/include
	cp -p tcpio.h /usr/local/include

install-dispense: all-dispense install-man
	install -c dispense /usr/local/bin
	install -c filter-input /usr/local/bin
	install -c filter-against /usr/local/bin
	cp -p randomize /usr/local/bin
	cp -p check-ips /usr/local/bin

install-man:
	mkdir -p /usr/local/man/man1
	cp -p enact.1 /usr/local/man/man1/enact.1
	cp -p dispense.1 /usr/local/man/man1/dispense.1

clean:
	rm -f ${CLIENT} ${SERVER} *.o libdispense.a *core
	cd example-add; make clean
	cd example-primes; make clean
	cd example-open-program; make clean
	cd example-polyominoes; make clean
