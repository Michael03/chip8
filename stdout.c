#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "./chip8.h"


//const char *OPEN =  "\e[38;2;255;255;255m_\e[38;2;0;0;0m";
const char *OPEN = " ";
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

void stdio_updateKeys(struct chip8 *chip8) {
  set_mode(1);
  int c, m;
/*  for(int i = 0; i <= 15; i++) {
    if(chip8->key[i] > 0) {
      chip8->key[i] = chip8->key[i] - 1;
    }
  }*/
  while((c = get_key())) {
   if(c == 49) m = 1;
   if(c == 50) m = 2;
   if(c == 51) m = 3;
   if(c == 52) m = 12;
   if(c == 113) m = 4;
   if(c == 119) m = 5;
   if(c == 101) m = 6;
   if(c == 114) m = 13;
   if(c == 97) m = 7;
   if(c == 115) m = 8;
   if(c == 100) m = 9;
   if(c == 102) m = 14;
   if(c == 122) m = 10;
   if(c == 120) m = 0;
   if(c == 99) m = 11;
   if(c == 118) m = 15;
//   printf("orig %d mapped %d\n", c, m);
   chip8->key[m] = 1;
  }
  set_mode(0);
}


int stdio_isKeyPressed(struct chip8 *chip8, BYTE key) {
  if(chip8->key[key]) {
    chip8->key[key] = 0;
    return 1;
  }
  return 0;
}

void stdio_update(int width, int height,struct chip8 *chip8 ) {
  fprintf(stderr, "\e[1;1H\e[2J");
  for(int y = 0; y < height; y++) {
    for(int i = 0; i < width; i++) {
      if (chip8->videoMemory[i][y]) {
        fprintf(stderr, "%s", CLOSED);
      } else {
        fprintf(stderr, "%s", OPEN);
      }
    }
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
}

