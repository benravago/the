
the.4l: bin \
  bin/box.o bin/colour.o bin/column.o \
  bin/comm1.o bin/comm2.o bin/comm3.o bin/comm4.o bin/comm5.o \
  bin/commset1.o bin/commset2.o \
  bin/commsos.o bin/commutil.o \
  bin/cursor.o bin/default.o bin/directry.o \
  bin/edit.o bin/error.o bin/execute.o \
  bin/file.o bin/getch.o \
  bin/linked.o bin/mouse.o bin/nonansi.o \
  bin/parser.o bin/prefix.o bin/print.o \
  bin/query.o bin/query1.o bin/query2.o \
  bin/reserved.o bin/scroll.o bin/show.o \
  bin/single.o bin/sort.o bin/target.o \
  bin/the.o bin/util.o \
  bin/rexx.o bin/imc/rexx.o \
  bin/imc/calc.o bin/imc/globals.o bin/imc/interface.o \
  bin/imc/rxfn.o bin/imc/shell.o bin/imc/util.o
	gcc -m64 -lncurses -lm $(filter %.o, $^) -o $@
	strip $@

bin: 
	rm -fr bin
	mkdir -pv bin/imc

bin/%.o: src/%.c
	cc -g -c -O3 -Wall -fomit-frame-pointer -I./src -I./src/imc -o $@ $<

bin/imc/%.o: src/imc/%.c
	cc -c -O3 -Wall -fomit-frame-pointer -I./src/imc -o $@ $<

clean:
	rm -fr bin

