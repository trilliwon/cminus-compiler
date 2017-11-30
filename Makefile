#
# Makefile for C-
# Gnu C Version
#

BISON = bison
LEX = flex
CC = gcc

TARGET = cminus
OBJS = main.o util.o cminus.tab.c lex.yy.c

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) -ly -ll

main.o: main.c globals.h util.h scan.h cminus.tab.h
	$(CC) -o $@ -c main.c

util.o: util.c util.h globals.h cminus.tab.h
	$(CC) -o $@ -c util.c

lex.yy.c: cminus.l globals.h util.h scan.h cminus.tab.h
	$(LEX) -w cminus.l

cminus.tab.h cminus.tab.c: cminus.y globals.h util.h scan.h
	$(BISON) -d cminus.y

clean:
	rm -rf *.o lex.yy.c cminus cminus.tab.h cminus.tab.c cminus.output