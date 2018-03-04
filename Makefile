CC=gcc
CFLAGS=-Wall 
LFLAGS= -ldl -rdynamic
D= DNOTHING


all: tesh.o CommandStruct.o StringManagement.o main.o Defines.h
	$(CC) main.o tesh.o CommandStruct.o StringManagement.o -o tesh $(CFLAGS) $(LFLAGS) -$(D)
	rm tesh.o CommandStruct.o StringManagement.o main.o

main.o : main.c
	$(CC) -c main.c -o main.o $(CFLAGS) $(LFLAGS) -$(D)

tesh.o : tesh.c
	$(CC) -c tesh.c -o tesh.o $(CFLAGS) -$(D)

CommandStruct.o : CommandStruct.c Defines.h
	$(CC) -c CommandStruct.c -o CommandStruct.o $(CFLAGS) -$(D)

StringManagement.o : StringManagement.c Defines.h
	$(CC) -c StringManagement.c -o StringManagement.o $(CFLAGS) -$(D)



