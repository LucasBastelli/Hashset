PROGRAMS = hashHD hashHD-DB

CC = gcc

all:	$(PROGRAMS)

hashHD:	hashHD.c
	gcc -o hash hashHD.c -pthread

hashHD-DB:
	gcc -o hash-DB hashHD.c -pthread -DDEBUG


clean:
	rm -f $(PROGRAMS) *.o
