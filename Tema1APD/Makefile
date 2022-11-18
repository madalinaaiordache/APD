

CC=g++
CFLAGS=-I
DEPS = Map-Reducer.h
FILES = ./out_*.txt

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
build: Map-Reducer.o
	$(CC) -pthread -o tema1 Map-Reducer.o

.PHONY: clean

clean:
	rm *.o tema1
	for file in out_*.txt; do \
  		if [ -e "$$file" ]; then \
  		  rm "$$file" || exit 1; \
  		fi; \
  	done
