CC:= gcc
LD:= gcc

CFLAGS:= -c -Wall -w
LDFLAGS:= -pthread 
DEPS:= mylib.h utils.h child.h
BIN:= matrixMath

OBJS:= main.o mylib.o utils.o child.o

all: $(BIN)

clean:
	@rm *.o $(BIN)

mylib.o: mylib.c mylib.h
	@echo $@
	@$(CC) $(CFLAGS) -o $@ $<

main.o: main.c $(DEPS)
	@echo $@
	@$(CC) $(CFLAGS) -o $@ $<

utils.o: utils.c utils.h
	@echo $@
	@$(CC) $(CFLAGS) -o $@ $<

child.o: child.c child.h utils.h
	@echo $@
	@$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BIN): $(OBJS)
	@$(LD) $(LDFLAGS) -o $@ $^

.PHONY: all clean
