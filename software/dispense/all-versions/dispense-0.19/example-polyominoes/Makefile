CLIENTS=pentomino-10x6-client \
        one-sided-pentomino-10x9-client \
        tetr-pentomino-20x4-client \
        tetr-pentomino-10x8-client
CREATE_INPUTS=pentomino-10x6-create-inputs \
              one-sided-pentomino-10x9-create-inputs \
              tetr-pentomino-20x4-create-inputs \
              tetr-pentomino-10x8-create-inputs
INCLUDES=polyomino.h poly-list.h
CC=cc -W -Wall -O4 -funroll-loops -lm

all:	${CLIENTS} ${CREATE_INPUTS}

pentomino-10x6-client:	pentomino.c ${INCLUDES}
	${CC} pentomino.c -o pentomino-10x6-client -DCLIENT \
	-Dwidth=10 -Dheight=6

pentomino-10x6-create-inputs:	pentomino.c ${INCLUDES}
	${CC} pentomino.c -o pentomino-10x6-create-inputs -DCREATE_INPUTS \
	-Dwidth=10 -Dheight=6 -Dcreate_inputs_max_nr_attached=2

one-sided-pentomino-10x9-client:	one-sided-pentomino.c ${INCLUDES}
	${CC} one-sided-pentomino.c -o one-sided-pentomino-10x9-client -DCLIENT \
	-Dwidth=10 -Dheight=9

one-sided-pentomino-10x9-create-inputs:	one-sided-pentomino.c ${INCLUDES}
	${CC} one-sided-pentomino.c -o one-sided-pentomino-10x9-create-inputs -DCREATE_INPUTS \
	-Dwidth=10 -Dheight=9 -Dcreate_inputs_max_nr_attached=4

tetr-pentomino-20x4-client:	tetr-pentomino.c ${INCLUDES}
	${CC} tetr-pentomino.c -o tetr-pentomino-20x4-client -DCLIENT \
	-Dwidth=20 -Dheight=4

tetr-pentomino-20x4-create-inputs:	tetr-pentomino.c ${INCLUDES}
	${CC} tetr-pentomino.c -o tetr-pentomino-20x4-create-inputs -DCREATE_INPUTS \
	-Dwidth=20 -Dheight=4 -Dcreate_inputs_max_nr_attached=4

tetr-pentomino-10x8-client:	tetr-pentomino.c ${INCLUDES}
	${CC} tetr-pentomino.c -o tetr-pentomino-10x8-client -DCLIENT \
	-Dwidth=10 -Dheight=8

tetr-pentomino-10x8-create-inputs:	tetr-pentomino.c ${INCLUDES}
	${CC} tetr-pentomino.c -o tetr-pentomino-10x8-create-inputs -DCREATE_INPUTS \
	-Dwidth=10 -Dheight=8 -Dcreate_inputs_max_nr_attached=4

clean:
	rm -f ${CLIENTS} ${CREATE_INPUTS} *.core

