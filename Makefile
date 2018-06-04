build:
	cc -ggdb -o emulator  chip8.c 

run:
	./emulator PONG.ch8
