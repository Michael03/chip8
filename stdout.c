#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "./chip8.h"


const char OPEN = ' ';
const char *CLOSED = "\e[48;2;255;255;255m\e[38;2;255;255;255m█\e[48;2;0;0;0m\e[38;2;0;0;0m";

void stdio_init() {

}

void set_mode(int want_key)
{
	static struct termios old, new;
	if (!want_key) {
		tcsetattr(STDIN_FILENO, TCSANOW, &old);
		return;
	}

	tcgetattr(STDIN_FILENO, &old);
	new = old;
	new.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &new);
}

int get_key()
{
	int c = 0;
	struct timeval tv;
	fd_set fs;
	tv.tv_usec = tv.tv_sec = 0;

	FD_ZERO(&fs);
	FD_SET(STDIN_FILENO, &fs);
	select(STDIN_FILENO + 1, &fs, 0, 0, &tv);

	if (FD_ISSET(STDIN_FILENO, &fs)) {
		c = getchar();
		set_mode(0);
	}
	return c;
}

int stdio_getCharNb() {
  set_mode(1);
  return get_key();
}

void stdio_update(int width, int height, BYTE videoMemory[width][height]) {
    fprintf(stderr, "\e[1;1H\e[2J");

  for(int y = 0; y < height; y++) {
    for(int i = 0; i < width; i++) {
      if (videoMemory[i][y]) {
        fprintf(stderr, "%s", CLOSED);
      } else {
        fprintf(stderr, "%c", OPEN);
      }
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
}

