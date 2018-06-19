#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

void writeEndian(unsigned short int val, FILE *fd);

int main(int argc, char *argv[]) {
  FILE *fd;
  FILE *ofd;

  unsigned short int buffer[1];
  char *line = NULL;
  size_t len = 0;
  ssize_t read = 0;
  if(argc < 2) {
    printf("Usage assembler <filename>\n");
    exit(1);
  }
  char *filename = argv[1];
  fd = fopen(filename, "r");
  if(!fd) {
    printf("failed reading file %s\n", filename);
  }
  ofd = fopen("game.ch8", "wb");
  if(!ofd) {
    printf("unable to open file for writing\n");
    exit(1);
  }

  printf("Starting assembling %s\n", filename);
  while ((read = getline(&line, &len, fd)) != -1) {
    printf("%s", line);
    int x = 0, y = 0,n = 0,nn = 0,nnn = 0;
    char command[4];
    if(strncmp(line, "mov", 3) == 0) {
      sscanf(line, "%s %d %d", command, &x, &y);
      printf("%#06x\n",0x6000 | (x<<8) | y);
      unsigned short int val = 0x6000 | (x<<8) | y;
      writeEndian(val, ofd);
    } else if (strncmp(line, "chr", 3) == 0) {
      sscanf(line, "%s %d %d", command, &x, &y);
      unsigned short int val = 0xF029 | (x<<8);
      printf("%#06x\n", val);
      writeEndian(val, ofd);
    } else if (strncmp(line, "drw", 3) == 0) {
      sscanf(line, "%s %d %d %d", command, &x, &y, &n);
      unsigned short int val = 0xD000 | (x<<8) | (y<<4) | (n);
      printf("%#06x\n", val);
      writeEndian(val, ofd);
    }
    printf("%s, %d, %d\n", command, x, y);


    //unsigned short int val2 = 0x2111;
    //buffer[0] = (0x6000 | (x<<8) | y);
    //fwrite(&val2 , sizeof(val), 1, ofd); 
    /*
       if(strncmp(line, "mov", 3) == 0) {
       printf("is mov\n"); 
       while(!(*line >= '0' && *line <= '9')) {
       line++;
       }
       if (sscanf(line, "%d", &x) == 1) {
       printf("X=%d\n", x);
       }
       line = strstr(line, " ");
       while(!(*line >= '0' && *line <= '9')) {
       line++;
       }
       if (sscanf(line, "%d", &y) == 1) {
       printf("Y=%d\n", y);
       }

       }
       */
  }
  exit(0);
}

void writeEndian(unsigned short int val, FILE *fd) {
  unsigned short int fixedVal = ((val >> 8) & 0x00FF)|((val<<8) & 0xFF00);
  fwrite(&fixedVal, sizeof(val), 1, fd);
}
