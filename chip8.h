typedef unsigned char BYTE;
typedef unsigned short int WORD;

void stdio_init(void);
void stdio_update(int width, int height, BYTE videoMemory[width][height]);
int stdio_getCharNb(void);

typedef void (* UPDATE)(int width, int height, BYTE videoMemory[width][height]);
typedef void (* INIT)(void);
typedef int (* GETCHARNB)(void);

typedef struct IO {
  INIT init;
  UPDATE update;
  GETCHARNB getCharNb;
} IO;
