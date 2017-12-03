#
# Makefile for C-
# Gnu C Version
#

CC = gcc

CFLAGS =

OBJS = main.o util.o scan.o

OBJS_FLEX = lex.yy.o

cminus: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o cminus

#by flex
cminus_flex: $(OBJS_FLEX)
	$(CC) $(CFLAGS) main.o util.o -o

main.o: main.c globals.h util.h scan.h
	$(CC) $(CFLAGS) -c main.c

util.o: util.c util.h globals.h
	$(CC) $(CFLAGS) -c util.c

scan.o: scan.c scan.h util.h globals.h
	$(CC) $(CFLAGS) -c scan.c

# lex.yy.o: cminus.l scan.h util.h globals.h
# 	flex cminus.l
# 	$(CC) $(CFLAGS) -c lex.yy.c -lfl

clean:
	-rm cminus
	-rm $(OBJS)
	-rm lex.yy.*

all: cminus
