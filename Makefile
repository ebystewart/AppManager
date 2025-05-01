CC=gcc
CFLAGS=-g
LIBS= -libc
OBJS= AppManager.o \
      glueThread/glthread.o

AppManager.bin:${OBJS}
	${CC} ${CFLAGS} ${OBJS} -o AppManager.bin -L ${LIBS}

AppManager.o:AppManager.c
	${CC} ${CFLAGS} -c AppManager.c -I . -o AppManager.o

glthread.o:glueThread/glthread.c
	${CC} ${CFLAGS} -c glueThread/glthread.c -I . -o glueThread/glthread.o

all:
	make

clean:
	rm *.o
	rm *.bin