all:emulator

emulator: chip8.o stdout.o
	cc -ggdb -o $@ $^

chip8.o: chip8.c
	gcc -c $<

stdout.o: stdout.c
	gcc -c $<

build:
	cc -ggdb -o emulator  chip8.c 

.PHONY: clean
clean:
	rm -rf emulator.dSYM/
	rm *.o
	rm emulator

run:
	./emulator PONG.ch8

#$@: the target filename.
#$*: the target filename without the file extension.
#$<: the first prerequisite filename.
#$^: the filenames of all the prerequisites, separated by spaces, discard duplicates.
#$+: similar to $^, but includes duplicates.
#$?: the names of all prerequisites that are newer than the target, separated by spaces.
