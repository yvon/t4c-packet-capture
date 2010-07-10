OBJECT = t4c
CFLAGS = -W -Wall -O -mno-cygwin -I /usr/include/winpcap
OBJS = t4c.o
LIBS = -lwpcap -lWs2_32

all: ${OBJS}
	${CC} ${CFLAGS} -o ${OBJECT} ${OBJS} ${LIBS}

clean:
	rm -f ${OBJS} ${OBJECT}

.c.o:
	${CC} ${CFLAGS} -c -o $*.o $<
