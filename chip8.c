#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chip8.h"

const struct timespec sleepTime = {0, 12000000L};
const struct timespec sleepTime2 = {0, 900000L};

struct chip8 chup8;
IO io;

int timer = 0;

WORD getOpcode();

int getVnum(WORD opcode, int mask, int shift) {
  int vNum = opcode & mask;
  return vNum >> shift;
}

int getDelay() {
  return timer;
}

// 00E0	Display	disp_clear()	Clears the screen.
void clearDisplay(WORD opcode) {
  printf("Clear Display\n");
  for(int x = 0; x < 64; x++) {
    for(int y = 0; y < 32; y++) {
      chip8.videoMemory[x][y] = 0;
    }
  }
}

// 00EE	Flow	return;	Returns from a subroutine.
void returnFromSubroutine(WORD opcode) {
  chip8.pc = chip8.stack[--chip8.sp];
  printf("Return new pc is %#06x\n", chip8.pc);
}

// 1NNN goto NNN;
void jump(WORD opcode) {
  printf("Goto %#05x\n", opcode & 0xFFF);
  chip8.pc = opcode & 0xFFF;
}

// 2NNN call NNN
void call(WORD opcode) {
  printf("Call subroutine at %#05x\n",  opcode & 0xFFF);
  chip8.stack[chip8.sp++] = chip8.pc;
  chip8.pc = opcode & 0xFFF;
}

//3XNN	Cond	if(Vx==NN)	Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block)
void ifVxEqualsNN(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Skip if register = V%d if(%#04x==%#04x)\n", vX, chip8.V[vX], opcode & 0xFF);
  if(chip8.V[vX] == (opcode & 0xFF)) {
    chip8.pc += 2;
  }
}

// 4XNN	Cond	if(Vx!=NN)	Skips the next instruction if VX doesn't equal NN. (Usually the next
//instruction is a jump to skip a code block)
void ifVxNotEqualsNN(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("V%d if(%#04x!=%#04x)\n", vX, chip8.V[vX], opcode & 0xFF);
  if(chip8.V[vX] != (opcode & 0xFF)) {
    chip8.pc += 2;
  }
}

// 5XY0	Cond	if(Vx==Vy)	Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block)
void ifVxEqualsVY(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  int vY = getVnum(opcode, 0x00F0, 4);
  printf("V%d v%d if(%d==%d)\n", vX, vY, chip8.V[vX], chip8.V[vY]);
  if(chip8.V[vX] == chip8.V[vY]) {
    chip8.pc += 2;
  }
}

// 6XNN	Const	Vx = NN
void setVxToNN(WORD opcode) {
  int vX = getVnum(opcode, 0xF00, 8);
  printf("set V%d to %#04x\n", vX, opcode & 0xFF);
  chip8.V[vX] = opcode & 0xFF;
}

// 7XNN
void addNNToVx(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("add V%d to %#04x\n", vX, opcode & 0xFF);
  chip8.V[vX] += (opcode & 0xFF);
}

// 8XY0	Assign	Vx=Vy	Sets VX to the value of VY.
void setVxToVy(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  int vY = getVnum(opcode, 0x00F0, 4);
  printf("set V%d %d to V%d %d\n", vX, chip8.V[vX], vY, chip8.V[vY]);
  chip8.V[vX] = chip8.V[vY];
}

// 8XY1	BitOp	Vx=Vx|Vy	Sets VX to VX or VY. (Bitwise OR operation)
void setVxOrVy(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  int vY = getVnum(opcode, 0x00F0, 4);
  printf("set V%d to V%d | V%d\n", vX, vX, vY);
  chip8.V[vX] = chip8.V[vX] | chip8.V[vY];
}

// 8XY2	BitOp	Vx=Vx&Vy	Sets VX to VX and VY. (Bitwise AND operation)
void setVxToVxAndVy(WORD opcode) {
  int vX = getVnum(opcode, 0X0F00, 8);
  int vY = getVnum(opcode, 0X00F0, 4);
  printf("set V%d to V%d and V%d (V%d = %d & %d)\n", vX, vX, vY, vX, chip8.V[vX], chip8.V[vY]);
  chip8.V[vX] = chip8.V[vX] & chip8.V[vY];
}

// 8XY3	BitOp	Vx=Vx^Vy	Sets VX to VX xor VY
void setVxToVxXorVy(WORD opcode) {
  int vX = getVnum(opcode, 0X0F00, 8);
  int vY = getVnum(opcode, 0X00F0, 4);
  printf("set V%d to V%d and V%d (V%d = %d & %d)\n", vX, vX, vY, vX, chip8.V[vX], chip8.V[vY]);
  chip8.V[vX] = chip8.V[vX] ^ chip8.V[vY];
}

// 8XY4	Math	Vx += Vy	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
void setVxToVxAddVy(WORD opcode) { 
  int vX = getVnum(opcode, 0X0F00, 8);
  int vY = getVnum(opcode, 0X00F0, 4);
  chip8.V[0xF] = (chip8.V[vX] + chip8.V[vY]) > 0xFF;
  printf("set V%d to V%d and V%d V%d = %d + %d\n", vX, vX, vY, vX, chip8.V[vX], chip8.V[vY]);
  chip8.V[vX] = chip8.V[vX] + chip8.V[vY];
}

// 8XY5	Math	Vx -= Vy	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
void setVxEqualsMinusVy(WORD opcode) {
  int vX = getVnum(opcode, 0X0F00, 8);
  int vY = getVnum(opcode, 0X00F0, 4);
  chip8.V[0xF] = chip8.V[vX] <= chip8.V[vY];
  printf("set V%d to V%d and V%d ( V%d = %d - %d)\n", vX, vX, vY, vX, chip8.V[vX], chip8.V[vY]);
  chip8.V[vX] = chip8.V[vX] - chip8.V[vY];
}

// Vx = VX/2
//8XY6	BitOp	Vx=Vx>>1	Shifts VY right by one and stores the result to VX (VY remains unchanged).
//VF is set to the value of the least significant bit of VY before the shift.
void setVxToVyShiftedRightByOne(WORD opcode) {
  int vX = getVnum(opcode, 0X0F00, 8);
  chip8.V[0xF] = chip8.V[vX] & 1;
  printf("shifting V%#03x  %d right by 1 (%d / 2 = %d)", vX, chip8.V[vX], chip8.V[vX], chip8.V[vX] >> 1);
  chip8.V[vX] = chip8.V[vX] >> 1;
  printf(" result %d\n", chip8.V[vX]);
}

// 8XY7	Math	Vx=Vy-Vx	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.

void setVxtoVyMinusVx(WORD opcode) {
  int vX = getVnum(opcode, 0X0F00, 8);
  int vY = getVnum(opcode, 0X00F0, 4);
  chip8.V[0xF] = chip8.V[vY] <= chip8.V[vX]; 
  printf("V%#03x to V%#03x - V%#03x (%d = %d - %d)", vX, vY, vX,chip8.V[vX], chip8.V[vY], chip8.V[vX]);
  chip8.V[vX] = chip8.V[vY] - chip8.V[vX];
}

// 8XYE	BitOp	Vx=Vy<<1	Shifts VX left by one and copies the result to VX. VF is set to the value of the most significant bit of VX before the shift.[2]
void setVxToVxShiftedLeftByOne(WORD opcode) {
  int vX = getVnum(opcode, 0X0F00, 8);
  chip8.V[0xF] = chip8.V[vX] & 0xFF;
  printf("shifting V%#03x  %d left by 1 (%d / 2 = %d)", vX, chip8.V[vX], chip8.V[vX], chip8.V[vX] >> 1);
  chip8.V[vX] = chip8.V[vX] << 1;
  printf(" result %d\n", chip8.V[vX]);
}

//9XY0	Cond	if(Vx!=Vy)	Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block)
void skipIfVxNotEqualsVy(WORD opcode) {
  int vX = getVnum(opcode, 0X0F00, 8);
  int vY = getVnum(opcode, 0X00F0, 4);
  printf("skip if not equal V%d,  V%d  ( %d != %d)\n", vX, vY, chip8.V[vX], chip8.V[vY]);
  if(chip8.V[vX] != chip8.V[vY]) {
    chip8.pc += 2;
  }
}

// ANNN	MEM	I = NNN	Sets I to the address NNN.
void setIToNNN(WORD opcode) {
  printf("Set I to %#05x\n",  opcode & 0xFFF);
 chip8.I = opcode & 0xFFF;
}

// CXNN	Rand	Vx=rand()&NN	Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
void setVxToRand(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  int nn = opcode & 0x00FF;
  int randNumber = rand() % 255;
  int result = randNumber & nn;
  chip8.V[vX] = result;
  printf("set V%d to $%d randNumber: %d, nn: %#04x\n" , vX, result, randNumber, nn);
}

// DXYN	Disp	draw(Vx,Vy,N)
// Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each
// row of 8 pixels is read as bit-coded starting from chip8.memory location I; I value doesn’t change
// after the execution of this instruction. As described above, VF is set to 1 if any screen pixels
// are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen
void dxyn(WORD opcode) {

  nanosleep(&sleepTime, NULL);
  int vX = getVnum(opcode, 0x0F00, 8);
  int vY = getVnum(opcode, 0x00F0, 4);
  int height = opcode & 0x000F;
  chip8.V[0xF] = 0;
  printf("Drawing I %#06x at X:%d Y:%d height:%d from registers V%d, V%d\n", chip8.I, chip8.V[vX], chip8.V[vY], height, vX, vY);
  for (int yline = 0; yline < height; yline++) {
    BYTE line = chip8.memory[chip8.I+yline];
    // get each pixel in 8 bit line;
    for (int xColumn = 0; xColumn < 8; xColumn++) {
      int mask = 1 << (7 - xColumn);
      int value = (line & mask);
      if(value) {
        int x = chip8.V[vX] + xColumn;
        int y = chip8.V[vY] + yline;
        if(chip8.videoMemory[x][y]) {
          chip8.V[0xF] = 1;
        }
        chip8.videoMemory[x][y] ^= 1;
      }
    }
  }

  io.update(64,32, &chip8);

}

//EX9E	KeyOp	if(key()==Vx)	Skips the next instruction if the key stored in VX
//is pressed. (Usually the next instruction is a jump to skip a code block)
void skipIfKeyPressed(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Skipping if key in V%d is %d\n", vX, chip8.V[vX]);
  if(io.isKeyPressed(&chip8, chip8.V[vX])) {
    printf("Ispressed \n");
    chip8.pc += 2;
  } else {
    printf("Not pressed\n");
  }
}

//EXA1	KeyOp	if(key()!=Vx)	Skips the next instruction if the key stored in VX isn't pressed.
//(Usually the next instruction is a jump to skip a code block)
void skipIfKeyNotPressed(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Skipping if key in V%d is not %d\n", vX, chip8.V[vX]);
  if(!io.isKeyPressed(&chip8, chip8.V[vX])) {
    chip8.pc += 2;
  }
}

// FX07	Timer	Vx = get_delay()	Sets VX to the value of the delay timer.
void setVxToDelay(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  chip8.V[vX] = getDelay();
  printf("Setting V%d to value of delay timer %d\n", vX, chip8.V[vX]);
}

// FX15	Timer	delay_timer(Vx)	Sets the delay timer to VX.
void setDelayTimer(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Register V%d setting timer delay to %d\n", vX, chip8.V[vX]);
  //TODO remove *5 and make 60hz
  timer = chip8.V[vX]*5;
}

// FX0A	KeyOp	Vx = get_key()	A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
void waitForKeyPressStoreInVx(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Waiting for key press storing in V%d\n", vX);
  int c, m ;
  while( (c = getchar()) == '\n') {}
//TODO add error handling
  printf("pressed orig %d\n", c);
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
  printf("pressed %d\n", c);
  printf("mapped = %d\n", m);
  chip8.V[vX] = m;
}

// FX1E	MEM	I +=Vx	Adds VX to I.[3]
void setIToIPlusVx(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("I%#06x  += V%d (%d += %d)\n", chip8.I, vX, chip8.I, chip8.V[vX]);
  chip8.V[0xF] = chip8.I + chip8.V[vX] > 0xFFF;
  chip8.I += chip8.V[vX];
}

//FX29	MEM	I=sprite_addr[Vx]	Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
void setSpriteAddr(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Sets I register to the locaition of the sprite for the character in v%d \n", vX);
  printf("Character is %d location is %d\n", chip8.V[vX], chip8.V[vX]*5 );
  chip8.I = (chip8.V[vX] * 5);
}

//FX33	BCD	set_BCD(Vx); *(I+0)=BCD(3); *(I+1)=BCD(2); *(I+2)=BCD(1);
void setBCD(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Set binary coded decimal from %d in chip8.memory location %#06x\n", vX, chip8.I);
  BYTE value = chip8.V[vX];
  BYTE hundreds = value / 100;
  BYTE tens = (value /10) % 10;
  BYTE units = value % 10;
  chip8.memory[chip8.I] = hundreds;
  chip8.memory[chip8.I+1] = tens;
  chip8.memory[chip8.I+2] = units;
}

// FX55 MEM reg_dump(Vx,&I) Stores V0 to VX (including VX) in memory starting at address I. I is
// increased by 1 for each value written
void registerDump(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Stores V0 to V%#03x in chip8.memory starting at addres I %d\n", vX, chip8.I);
  for(int i = 0; i <= vX; i++) {
    chip8.memory[chip8.I++] = chip8.V[i];
  }
}

//FX65	MEM	reg_load(Vx,&I)	 Fills V0 to VX (including VX) with values from memory starting at address I. I is increased by 1 for each value written.
void registerLoad(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Fills V0 to V%#03x with values from chip8.memory starting at addres I %d\n", vX, chip8.I);
  for(int i = 0; i <= vX; i++) {
     chip8.V[i] = chip8.memory[chip8.I++];
  }
}

void initCharacters() {
  // 0
  chip8.memory[0] = 0b11110000;
  chip8.memory[1] = 0b10010000;
  chip8.memory[2] = 0b10010000;
  chip8.memory[3] = 0b10010000;
  chip8.memory[4] = 0b11110000;
  // 1
  chip8.memory[5] = 0b00110000;
  chip8.memory[6] = 0b00010000;
  chip8.memory[7] = 0b00010000;
  chip8.memory[8] = 0b00010000;
  chip8.memory[9] = 0b00111000;
  // 2
  chip8.memory[10] = 0b11110000;
  chip8.memory[11] = 0b00010000;
  chip8.memory[12] = 0b11110000;
  chip8.memory[13] = 0b10000000;
  chip8.memory[14] = 0b11110000;
  // 3
  chip8.memory[15] = 0b11110000;
  chip8.memory[16] = 0b00010000;
  chip8.memory[17] = 0b11110000;
  chip8.memory[18] = 0b00010000;
  chip8.memory[19] = 0b11110000;
  // 4
  chip8.memory[20] = 0b10100000;
  chip8.memory[21] = 0b10100000;
  chip8.memory[22] = 0b11110000;
  chip8.memory[23] = 0b00100000;
  chip8.memory[24] = 0b00100000;
  // 5
  chip8.memory[25] = 0b11110000;
  chip8.memory[26] = 0b10000000;
  chip8.memory[27] = 0b11110000;
  chip8.memory[28] = 0b00010000;
  chip8.memory[29] = 0b11110000;
  // 6
  chip8.memory[30] = 0b11110000;
  chip8.memory[31] = 0b10000000;
  chip8.memory[32] = 0b11110000;
  chip8.memory[33] = 0b10010000;
  chip8.memory[34] = 0b11110000;
  // 7 
  chip8.memory[35] = 0b11110000;
  chip8.memory[36] = 0b00010000;
  chip8.memory[37] = 0b00010000;
  chip8.memory[38] = 0b00010000;
  chip8.memory[39] = 0b00010000;
  // 8
  chip8.memory[40] = 0b11110000;
  chip8.memory[41] = 0b10010000;
  chip8.memory[42] = 0b11110000;
  chip8.memory[43] = 0b10010000;
  chip8.memory[44] = 0b11110000;
  // 9
  chip8.memory[45] = 0b11110000;
  chip8.memory[46] = 0b10010000;
  chip8.memory[47] = 0b11110000;
  chip8.memory[48] = 0b00010000;
  chip8.memory[49] = 0b11110000;
  // A
  chip8.memory[50] = 0b11110000;
  chip8.memory[51] = 0b10010000;
  chip8.memory[52] = 0b11110000;
  chip8.memory[53] = 0b10010000;
  chip8.memory[54] = 0b10010000;
  // B
  chip8.memory[55] = 0b11110000;
  chip8.memory[56] = 0b01010000;
  chip8.memory[57] = 0b01110000;
  chip8.memory[58] = 0b01010000;
  chip8.memory[59] = 0b11110000;
  // C
  chip8.memory[60] = 0b11110000;
  chip8.memory[61] = 0b10000000;
  chip8.memory[62] = 0b10000000;
  chip8.memory[63] = 0b10000000;
  chip8.memory[64] = 0b11110000;
  // D
  chip8.memory[65] = 0b11110000;
  chip8.memory[66] = 0b01010000;
  chip8.memory[67] = 0b01010000;
  chip8.memory[68] = 0b01010000;
  chip8.memory[69] = 0b11110000;
  // E
  chip8.memory[70] = 0b11110000;
  chip8.memory[71] = 0b10000000;
  chip8.memory[72] = 0b11110000;
  chip8.memory[73] = 0b10000000;
  chip8.memory[74] = 0b11110000;
  // F
  chip8.memory[75] = 0b11110000;
  chip8.memory[76] = 0b10000000;
  chip8.memory[77] = 0b11110000;
  chip8.memory[78] = 0b10000000;
  chip8.memory[79] = 0b10000000;
}

int main(int argc, char* argv[]) {
  io.update = stdio_update;
  io.init = stdio_init;
  io.isKeyPressed = stdio_isKeyPressed;
  io.updateKeys = stdio_updateKeys;
  if(argc < 2) {
    printf("Usage emulator <romfilename>");
    exit(1);
  }
  char* filename = argv[1];
  printf("Loading %s\n", filename);
  chip8.pc = 0x200;
  printf("start\n");
  FILE *in;
  in = fopen(filename, "rb" );
  if(!in) {
    printf("Error reading file %s", filename);
    exit(1);
  }
  fread( &chip8.memory[0x200], 0xFFF, 1, in);
  fclose(in);

  initCharacters();
  srand(time(NULL));

  printf("PC is %#05x\n", chip8.pc);

  for(;;) {
    if(timer > 0) {
      timer -= 1;
    }
    nanosleep(&sleepTime2, NULL);
    io.updateKeys(&chip8);
    WORD opcode = getOpcode();
    switch (opcode & 0xF000) {
      case 0x0000: //: starts with 0
        switch(opcode & 0x000F) {
          case 0x0000: // 0x00E0 Clear display
            clearDisplay(opcode);
            break;
          case 0x000E:// 0x00EE return
            returnFromSubroutine(opcode);
            break;
        }
        break;
      case 0x1000: // GOTO NNN
        jump(opcode);
        break;
      case 0x2000: // call subroutine
        call(opcode);
        break;
      case 0x3000: // skip next instruction if Vx==NN
        ifVxEqualsNN(opcode);
        break;
      case 0x4000: // skip next instruction if not equal
        ifVxNotEqualsNN(opcode);
        break;
      case 0x5000: // skip next instruction if Vx == Vy
        ifVxEqualsVY(opcode);
        break;
      case 0x6000: // set Vx to NN
        setVxToNN(opcode);
        break;
      case 0x7000: // Adds NN to VX. (Carry flag is not changed
        addNNToVx(opcode);
        break;
      case 0x8000: // set Vx to NN
        switch(opcode & 0x000F) {
          case 0x0000:
            setVxToVy(opcode);
            break;
          case 0x0001:
            setVxOrVy(opcode);
            break;
          case 0x0002:
            setVxToVxAndVy(opcode);
            break;
          case 0x0003:
            setVxToVxXorVy(opcode);
            break;
          case 0x0004:
            setVxToVxAddVy(opcode);
            break;
          case 0x0005:
            setVxEqualsMinusVy(opcode);
            break;
          case 0x0006:
            setVxToVyShiftedRightByOne(opcode);
            break;
          case 0x0007:
            setVxtoVyMinusVx(opcode);
            break;
          default: 
            printf("Unknown opcode\n");
            break;
        }
        break;
      case 0x9000:
        skipIfVxNotEqualsVy(opcode);
        break;
      case 0xA000:
        setIToNNN(opcode);
        break;
      case 0xC000:
        setVxToRand(opcode);
        break;
      case 0xD000:
        dxyn(opcode);
        break;
      case 0xE000:
        switch(opcode & 0x00F0){
          case 0x0090:
            skipIfKeyPressed(opcode);
            break;
          case 0x00A0:
            skipIfKeyNotPressed(opcode);
            break;
          default:
            printf("Unknown opcode\n");
            break;
        }
        break;
      case 0xF000:
        switch(opcode & 0x00F0) {
          case 0x0000:
            switch(opcode & 0x000F){
              case 0x0007:// FX07	Timer	Vx = get_delay()	Sets VX to the value of the delay timer.
                setVxToDelay(opcode);
                break;
              case 0x000A:
                waitForKeyPressStoreInVx(opcode);
                break;
                break;
              default:
                printf("Known instruction Pattern 0xF_0_\n");
                break;
            }
            break;
          case 0x0010:
            switch(opcode & 0x000F) {
              case 0x0005:// FX15	Timer	delay_timer(Vx)	Sets the delay timer to VX.
                setDelayTimer(opcode);
                break;
              case 0x0008://FX18	Sound	sound_timer(Vx)	Sets the sound timer to VX.
                printf("setting sound timer to Vx\n");
                break;
              case 0x000E://FX1E	MEM	I +=Vx	Adds VX to I.[3]
                setIToIPlusVx(opcode);
                break;
              default:
                printf("Unknown opcode\n");
                break;
            }
            break;
          case 0x0020://FX29	MEM	I=sprite_addr[Vx]	Sets I to the location of the sprite for the
            setSpriteAddr(opcode);
            break;
          case 0x0030:// FX33	BCD set binary coded decimal at 
            setBCD(opcode);
            break;
          case 0x0050:// FX55	MEM	reg_dump(Vx,&I)	Stores V0 to VX (including VX) in memory starting at address I. I is increased by 1 for each value written
            registerDump(opcode);
            break;
          case 0x0060://FX65	MEM	reg_load(Vx,&I)	Fills V0 to VX (including VX) with values from memory starting at address I. I is increased by 1 for each value written
            registerLoad(opcode);
            break;
        }
        break;
      default: 
        printf("Unknown opcode\n");
        break;
    }
  }
}

WORD getOpcode() {
  printf("%#06x ", chip8.pc);
  WORD opcode = 0;
  opcode = chip8.memory[chip8.pc++];
  opcode <<= 8;
  opcode |= chip8.memory[chip8.pc++];
  printf("%#06x ", opcode);
  return opcode;
}
