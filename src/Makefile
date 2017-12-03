#
# Makefile for C-Minus
# Gnu C Version
# K. Louden 2/3/98
#
# Modified by Seongjun Kim, 2015

CC = gcc

CFLAGS = -Wall -g

OBJS = y.tab.o lex.yy.o main.o util.o symtab.o analyze.o

cminus: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o cminus

main.o: main.c globals.h util.h scan.h analyze.h
	$(CC) $(CFLAGS) -c main.c

util.o: util.c util.h globals.h
	$(CC) $(CFLAGS) -c util.c

symtab.o: symtab.c symtab.h
	$(CC) $(CFLAGS) -c symtab.c

analyze.o: analyze.c globals.h symtab.h analyze.h
	$(CC) $(CFLAGS) -c analyze.c

lex.yy.o: cminus.l scan.h util.h globals.h
	flex -o lex.yy.c cminus.l
	$(CC) $(CFLAGS) -c lex.yy.c

y.tab.o: cminus.y globals.h
	bison -d cminus.y --yacc
	$(CC) $(CFLAGS) -c y.tab.c

clean:
	-rm -f cminus
	-rm -f y.tab.c
	-rm -f y.tab.h
	-rm -f lex.yy.c
	-rm -f $(OBJS)

test: cminus
	-./cminus gcd.cm

all: cminus
