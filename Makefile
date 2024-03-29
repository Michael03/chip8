all:emulator

emulator: chip8.o stdout.o
	cc -ggdb -o $@ $^

chip8.o: chip8.c
	cc -ggdb -c $<

stdout.o: stdout.c
	cc -ggdb -c $<

build:
	cc -ggdb -o emulator  chip8.c 

.PHONY: clean
clean:
	rm -rf emulator.dSYM/
	rm *.o
	rm emulator

.PHONY: run
run:
	./emulator PONG.ch8

assembler: assembler.c
	cc -Wall -Wextra -ggdb $< -o $@

#$@: the target filename.
#$*: the target filename without the file extension.
#$<: the first prerequisite filename.
#$^: the filenames of all the prerequisites, separated by spaces, discard duplicates.
#$+: similar to $^, but includes duplicates.
#$?: the names of all prerequisites that are newer than the target, separated by spaces.
