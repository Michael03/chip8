build:
	cc -ggdb -o emulator  chip8.c 
	./emulator PONG.ch8
