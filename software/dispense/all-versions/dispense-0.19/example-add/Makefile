CC=cc -Wall -Werror -I.. -I/usr/include -I/usr/local/include -L.. -L/usr/lib -L/usr/local/lib

all:	add enact-add

add:	add.c
	${CC} add.c -o add

enact-add:	enact-add.c ../libdispense.a
	${CC} enact-add.c -o enact-add -ldispense -pthread

../libdispense.a:
	cd ..; make libdispense.a

clean:
	rm -f add enact-add *core
