OBJECT = interceptor
CFLAGS = -W -Wall -g
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
INCLUDES = includes

all: ${OBJS}
	${CC} ${CFLAGS} -o ${OBJECT} ${OBJS}

clean:
	rm -f ${OBJS} ${OBJECT}

fclean: clean

re: clean all 

.c.o:
	${CC} ${CFLAGS} -I ${INCLUDES} -c -o $*.o $<

