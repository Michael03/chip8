#include <stdlib.h>
#include <stdio.h>
#include "./stdout.h"

const char OPEN = ' ';
const char *CLOSED = "\e[48;2;255;255;255m\e[38;2;255;255;255m█\e[48;2;0;0;0m\e[38;2;0;0;0m";

void init() {

}

void update(int width, int height, BYTE videoMemory[width][height]) {
//  system("clear");
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
