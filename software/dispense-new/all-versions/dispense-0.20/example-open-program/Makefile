CC=cc -Wall -Werror -I.. -I/usr/include -I/usr/local/include -L.. -L/usr/lib -L/usr/local/lib

all:	example

example:	example.c ../libdispense.a
	${CC} example.c -o example -ldispense

../libdispense.a:
	cd ..; make libdispense.a

clean:
	rm -f example *core
