OBJECT = interceptor
CFLAGS = -W -Wall -g
OBJS = main.o ip.o crafting.o data_header.o udp.o debug.o

all: ${OBJS}
	${CC} ${CFLAGS} -o ${OBJECT} ${OBJS}

clean:
	rm -f ${OBJS} ${OBJECT}

fclean: clean

re: clean all 

.c.o:
	${CC} ${CFLAGS} -c -o $*.o $<

