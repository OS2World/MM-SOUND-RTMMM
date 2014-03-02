ALL : musicin.exe musicout.exe

CFLAGS =  -O2
LDFLAGS= 

.c.o: 
	gcc -c $(CFLAGS) $*.c


musicin.exe: musicin.o common.o encode.o subs.o psy.o tonal.o
	gcc $(LDFLAGS) -o musicin.exe musicin.o common.o encode.o subs.o psy.o tonal.o -lm

musicout.exe: musicout.o common.o decode.o subs.o
	gcc $(LDFLAGS)  musicout.o common.o decode.o subs.o -o musicout.exe -lm
