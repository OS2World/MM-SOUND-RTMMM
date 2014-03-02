CFLAGS = -Zmt -Zomf -c -O3 -fstrength-reduce -fexpensive-optimizations -ffast-math -m486 -g -I../library
;CFLAGS = -Zmt -Zomf -c -ffast-math -m486 -g 
LFLAGS = -Zmt -Zomf -O3 -ffast-math -m486 -g -Zlinker /DE -Zlinker /E:2
;LFLAGS = -Zmt -Zomf -O3 -ffast-math -m486 
LIBS=  -los2  -lMMPM2

CC=gcc  $(CFLAGS)
LINK = gcc -s $(LFLAGS)


all: rtmmm.exe rtmmminfo.exe test1.exe 

rtmmm.exe: rtmmm.obj rtmmmlib.obj rtmmm.def
	$(LINK) rtmmm.obj rtmmmlib.obj $(LIBS) rtmmm.def  -o rtmmm.exe

rtmmm.obj: rtmmm.c ../library/rtmmm.h
	$(CC)  rtmmm.c

rtmmmlib.obj: rtmmmlib.c ../library/rtmmm.h
	$(CC)  rtmmmlib.c

rtmmminfo.exe: rtmmminfo.obj rtmmmlib.obj rtmmminfo.def
	$(LINK) rtmmminfo.obj rtmmmlib.obj  $(LIBS) rtmmminfo.def  -o rtmmminfo.exe

rtmmminfo.obj: rtmmminfo.c ../library/rtmmm.h
	$(CC)  rtmmminfo.c

test1.exe: test1.obj rtmmmlib.obj rtmmm.def
	$(LINK) test1.obj rtmmmlib.obj  $(LIBS) test1.def  -o test1.exe

test1.obj: test1.c ../library/rtmmm.h
	$(CC)  test1.c


