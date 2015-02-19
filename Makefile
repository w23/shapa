CC?=cc
CFLAGS=-Wall -Werror -pedantic -O3
LDFLAGS=-lX11 -lGL -lrt

OBJS := main.o os_x11.o os_linux.o shader.o

shapa: Makefile ${OBJS}
	${CC} ${LDFLAGS} ${OBJS} -o shapa

%.o: %.c Makefile common.h
	${CC} ${CFLAGS} -c -o $@ $<

clean:
	@rm -f ${OBJS} shapa
