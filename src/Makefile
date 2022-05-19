
all: the 

clean:
	rm -f *.o imc/*.o
	rm -f the

the: box.o colour.o column.o comm1.o comm2.o comm3.o comm4.o comm5.o commset1.o commset2.o commsos.o commutil.o cursor.o default.o directry.o edit.o error.o execute.o file.o getch.o linked.o mouse.o memory.o nonansi.o parser.o prefix.o print.o query.o query1.o query2.o regex.o reserved.o rexx.o scroll.o show.o single.o sort.o target.o the.o util.o imc/calc.o imc/globals.o imc/interface.o imc/rexx.o imc/rxfn.o imc/shell.o imc/util.o
	cc -lncurses -lm -o $@ $^

%.o: %.c
	cc -c -O3 -Wall -fomit-frame-pointer -I. -I./imc -o $@ $<

box.c: the.h proto.h
colour.c: the.h proto.h
column.c: the.h proto.h
comm1.c: the.h proto.h
comm2.c: the.h proto.h
comm3.c: the.h proto.h
comm4.c: the.h proto.h 
comm5.c: the.h proto.h
commset1.c: the.h proto.h
commset2.c: the.h proto.h
commsos.c: the.h proto.h
commutil.c: the.h proto.h key.h command.h
cursor.c: the.h proto.h
default.c: the.h proto.h
directry.c: the.h proto.h directry.h
edit.c: the.h proto.h
error.c: the.h proto.h
execute.c: the.h proto.h
file.c: the.h proto.h directry.h
getch.c: getch.h
linked.c: the.h proto.h
memory.c: the.h proto.h
mouse.c: the.h proto.h
nonansi.c: the.h proto.h
parser.c: the.h proto.h
prefix.c: the.h proto.h
print.c: the.h proto.h
query1.c: the.h proto.h query.h
query2.c: the.h proto.h query.h
query.c: the.h proto.h query.h
regex.c: regex.h
reserved.c: the.h proto.h
rexx.c: the.h proto.h query.h therexx.h
scroll.c: the.h proto.h
show.c: the.h proto.h
single.c: the.h proto.h
sort.c: the.h proto.h
target.c: the.h proto.h
the.c: the.h proto.h thedefs.h
util.c: the.h proto.h

imc/%.o: imc/%.c
	cc -c -O3 -Wall -fomit-frame-pointer -DNO_LDL -DFSTAT_FOR_CHARS -DDAY=25 -DMONTH=2 -DYEAR=102 -DREXXIMC=\"/usr/local/bin\" -DREXXLIB=\"/usr/local/lib\" -I. -o $@ $<

imc/calc.c: imc/const.h imc/globals.h imc/functions.h imc/rexxsaa.h
imc/globals.c: imc/const.h imc/globals.h imc/rexxsaa.h
imc/interface.c: imc/const.h imc/globals.h imc/functions.h imc/rexxsaa.h
imc/rexx.c: imc/const.h imc/globals.h imc/functions.h imc/rexxsaa.h
imc/rxfn.c: imc/const.h imc/globals.h imc/functions.h imc/rexxsaa.h
imc/shell.c: imc/const.h imc/functions.h
imc/util.c: imc/const.h imc/globals.h imc/functions.h imc/rexxsaa.h

