CC=gcc
CFLAGS=-g

AppManager.bin:AppManager.o
	${CC} ${CFLAGS} AppManager.o -o AppManager.bin

AppManager.o:AppManager.c
	${CC} ${CFLAGS} -c AppManager.c -I . -o AppManager.o

all:
	make

clean:
	rm *.o
	rm *.bin