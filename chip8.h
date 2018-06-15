typedef unsigned char BYTE;
typedef unsigned short int WORD;

struct chip8 {
  BYTE memory[0xFFF];
  BYTE V[16];
  BYTE videoMemory[64][32];
  BYTE key[16];
  WORD stack[16];
  WORD sp;
  WORD pc;
  WORD I;
} chip8;

void stdio_init(void);
void stdio_update(int width, int height, struct chip8 *chip8);
int stdio_isKeyPressed(struct chip8 *chip8, BYTE key);
void stdio_updateKeys(struct chip8 *chip8);

typedef void (* UPDATE)(int width, int height, struct chip8 *chip8);
typedef void (* INIT)(void);
typedef int (* GETCHARNB)(struct chip8 *chip8);
typedef int (* ISKEYPRESSED)(struct chip8 *chip8, BYTE key);
typedef void (* UPDATEKEYS)(struct chip8 *chip8);

typedef struct IO {
  INIT init;
  UPDATE update;
  GETCHARNB getCharNb;
  UPDATEKEYS updateKeys;
  ISKEYPRESSED isKeyPressed;
} IO;
