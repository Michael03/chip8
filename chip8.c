#include <stdio.h>
#include <stdlib.h>

typedef unsigned char BYTE;
typedef unsigned short int WORD;

BYTE memory[0xFFF];
BYTE V[16];
WORD I;
WORD pc;
BYTE videoMemory[64][32];

WORD stack[16];
WORD sp;
BYTE key[16];

int timer = 0;

WORD getNextOpcode();

int getVnum(WORD opcode, int mask, int shift) {
  int vNum = opcode & mask;
  return vNum >> shift;
}

int getDelay() {
  return timer;
}

void returnFromSubroutine(WORD opcode) {
  pc = stack[--sp];
  printf("Return new pc is %#06x\n", pc);
}

// 1NNN goto NNN;
void jump(WORD opcode) {
  printf("Goto %#05x\n", opcode & 0xFFF);
  pc = opcode & 0xFFF;
}

// 2NNN call NNN
void call(WORD opcode) {
  printf("Call subroutine at %#05x\n",  opcode & 0xFFF);
  stack[sp++] = pc;
  pc = opcode & 0xFFF;
}

//3XNN	Cond	if(Vx==NN)	Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block)
void ifVxEqualsNN(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("V%d if(%d==%#04x)\n", vX, V[vX], opcode & 0xFF);
  if(V[vX] == (opcode & 0xFF)) {
    pc += 2;
  }
}

// 6XNN	Const	Vx = NN
void setVxToNN(WORD opcode) {
  int vX = getVnum(opcode, 0xF00, 8);
  printf("set V%d to %#04x\n", vX, opcode & 0xFF);
  V[vX] = opcode & 0xFF;
}

// 7XNN
void addNNToVx(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("add V%d to %#04x\n", vX, opcode & 0xFF);
  V[vX] += (opcode & 0xFF);
}
//
// 8XY1	BitOp	Vx=Vx|Vy	Sets VX to VX or VY. (Bitwise OR operation)
void setVxOrVy(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  int vY = getVnum(opcode, 0x00F0, 4);
  printf("set V%d to V%d | V%d\n", vX, vX, vY);
  V[vX] = V[vX] | V[vY];
}

// 8XY2	BitOp	Vx=Vx&Vy	Sets VX to VX and VY. (Bitwise AND operation)
void setVxToVxAndVy(opcode) {
    int vX = getVnum(opcode, 0X0F00, 8);
    int vY = getVnum(opcode, 0X00F0, 4);
    printf("set V%d to V%d and V%d\n", vX, V[vX], V[vY]);
    V[vX] = V[vX] & V[vY];
}

// ANNN	MEM	I = NNN	Sets I to the address NNN.
void setIToNNN(WORD opcode) {
  printf("Set I to %#05x\n",  opcode & 0xFFF);
  I = opcode & 0xFFF;
}

// DXYN	Disp	draw(Vx,Vy,N)
// Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. Each
// row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change
// after the execution of this instruction. As described above, VF is set to 1 if any screen pixels
// are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen
void dxyn(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  int vY = getVnum(opcode, 0x00F0, 4);
  int height = opcode & 0x000F;
  printf("Drawing at X:%d Y:%d height:%d from registers V%d, V%d\n", V[vX], V[vY], height, vX, vY);
  for (int yline = 0; yline < height; yline++) {
    BYTE line = memory[I+yline];
    // get each pixel in 8 bit line;
    for (int xColumn = 0; xColumn < 8; xColumn++) {
      int mask = 1 << (7 - xColumn);
      int value = (line & mask);
      if(value) {
        int x = V[vX] + xColumn;
        int y = V[vY] + yline;
        if(videoMemory[x][y]) {
          V[0xF] = 1;
        }
        videoMemory[x][y] ^= 1;
      }
    }
  }

  char open = '_';
  char closed = '0';
  for(int y = 0; y < 32; y++) {
    for(int i = 0; i < 64; i++) {
      if (videoMemory[i][y]) {
        printf("%c", closed);
      } else {
        printf("%c", open);
      }
    }
    printf("\n");
  }
  printf("\n");
}

//EX9E	KeyOp	if(key()==Vx)	Skips the next instruction if the key stored in VX
//is pressed. (Usually the next instruction is a jump to skip a code block)
void skipIfKeyPressed(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Skipping if key in V%d is %d\n", vX, V[vX]);
}

// FX07	Timer	Vx = get_delay()	Sets VX to the value of the delay timer.
void setVxToDelay(WORD opcode) {
  int vXi = getVnum(opcode, 0x0F00, 8);
  V[vXi] = getDelay();
  printf("Setting V%d to value of delay timer %d\n", vXi, V[vXi]);
}

// FX15	Timer	delay_timer(Vx)	Sets the delay timer to VX.
void setDelayTimer(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Register V%d setting timer delay to %d\n", vX, V[vX]);
  timer = V[vX];
}

//FX29	MEM	I=sprite_addr[Vx]	Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
void setSpriteAddr(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Sets I register to the locaition of the sprite for the character in v%d \n", vX);
  printf("Character is %d\n", V[vX]);
  I = 0;
}

//FX33	BCD	set_BCD(Vx); *(I+0)=BCD(3); *(I+1)=BCD(2); *(I+2)=BCD(1);
void setBCD(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Set binary coded decimal from %d in memory location %#06x\n", vX, I);
  BYTE value = V[vX];
  BYTE hundreds = value / 100;
  BYTE tens = (value /10) % 10;
  BYTE units = value % 10;
  memory[I] = hundreds;
  memory[I+1] = tens;
  memory[I+2] = units;
}

//FX65	MEM	reg_load(Vx,&I)	 Fills V0 to VX (including VX) with values from memory starting at address I. I is increased by 1 for each value written.
void registerLoad(WORD opcode) {
  int vX = getVnum(opcode, 0x0F00, 8);
  printf("Fills V0 to Vx with values from memory starting at addres i\n");
  for(int i =0; i < vX; i++) {
    memory[I++] = V[i];
  }
}


void initCharacters() {
  // 0
  memory[0] = 0b11110000;
  memory[1] = 0b10010000;
  memory[2] = 0b10010000;
  memory[3] = 0b10010000;
  memory[4] = 0b11110000;
  // 1
  memory[5] = 0b00110000;
  memory[6] = 0b00010000;
  memory[7] = 0b00010000;
  memory[8] = 0b00010000;
  memory[9] = 0b00111000;
  // 2
  memory[10] = 0b11110000;
  memory[11] = 0b00010000;
  memory[12] = 0b11110000;
  memory[13] = 0b10000000;
  memory[14] = 0b11110000;
  // 3
  memory[15] = 0b11110000;
  memory[16] = 0b00010000;
  memory[17] = 0b11110000;
  memory[18] = 0b10000000;
  memory[19] = 0b11110000;
  // 4
  memory[20] = 0b10100000;
  memory[21] = 0b10100000;
  memory[22] = 0b11110000;
  memory[23] = 0b00100000;
  memory[24] = 0b00100000;
  // 5
  memory[25] = 0b11110000;
  memory[26] = 0b10000000;
  memory[27] = 0b11110000;
  memory[28] = 0b00010000;
  memory[29] = 0b11110000;
  // 6
  memory[30] = 0b11110000;
  memory[31] = 0b10000000;
  memory[32] = 0b11110000;
  memory[33] = 0b10010000;
  memory[34] = 0b11110000;
  // 7 
  memory[36] = 0b11110000;
  memory[37] = 0b00010000;
  memory[38] = 0b00010000;
  memory[39] = 0b00010000;
  memory[40] = 0b00010000;
  // 8
  memory[41] = 0b11110000;
  memory[42] = 0b10010000;
  memory[43] = 0b11110000;
  memory[44] = 0b10010000;
  memory[45] = 0b11110000;
  // 9
  memory[46] = 0b11110000;
  memory[47] = 0b10010000;
  memory[48] = 0b11110000;
  memory[49] = 0b00010000;
  memory[50] = 0b11110000;
  // A
  memory[51] = 0b11110000;
  memory[52] = 0b10010000;
  memory[53] = 0b11110000;
  memory[54] = 0b10010000;
  memory[55] = 0b10010000;
  // B
  memory[56] = 0b11110000;
  memory[57] = 0b01010000;
  memory[58] = 0b01110000;
  memory[59] = 0b01010000;
  memory[60] = 0b11110000;
  // C
  memory[61] = 0b11110000;
  memory[62] = 0b10000000;
  memory[63] = 0b10000000;
  memory[64] = 0b10000000;
  memory[65] = 0b11110000;
  // D
  memory[66] = 0b11110000;
  memory[67] = 0b01010000;
  memory[68] = 0b01010000;
  memory[69] = 0b01010000;
  memory[70] = 0b11110000;
  // E
  memory[71] = 0b11110000;
  memory[72] = 0b10000000;
  memory[73] = 0b11110000;
  memory[74] = 0b10000000;
  memory[75] = 0b11110000;
  // F
  memory[76] = 0b11110000;
  memory[77] = 0b10000000;
  memory[78] = 0b11110000;
  memory[79] = 0b10000000;
  memory[80] = 0b10000000;
}

int main(int argc, char* argv[]) {
  if(argc < 2) {
    printf("Usage emulator <romfilename>");
    exit(1);
  }
  char* filename = argv[1];
  printf("Loading %s\n", filename);
  pc = 0x200;
  printf("start\n");
  FILE *in;
  in = fopen(filename, "rb" );
  if(!in) {
    printf("Error reading file %s", filename);
    exit(1);
  }
  fread( &memory[0x200], 0xFFF, 1, in);
  fclose(in);
  initCharacters();
  printf("PC is %#05x\n", pc);
  for(int i =0; i < 123; i++) {
    int temppc = pc;
    if(timer > 0) {
      timer -= 1;
    }
    WORD opcode = getNextOpcode();
    switch (opcode & 0xF000) {
      case 0x0000: //: starts with 0
        switch(opcode & 0x000F) {
          case 0x0000: // 0x00E0 Clear display
            printf("Clear Display\n");
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
      case 0x4000: // skup next instruction if not equal
        printf("if(V%#03x!=%#04x)\n", opcode & 0xF00, opcode & 0xFF);
        break;
      case 0x5000: // skip next instruction if Vx == Vy
        printf("if(V%#03x==V%#03x)\n", opcode & 0xF00, opcode & 0xF0);
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
            printf("set V%#03x to V%#03x\n", opcode & 0xF00, opcode & 0xF0);
            break;
          case 0x0001:
            setVxOrVy(opcode);
            break;
          case 0x0002:
            setVxToVxAndVy(opcode);
            break;
          default: 
            printf("%#06x Unknown opcode\n");
            break;
        }
        break;
      case 0xA000:
        setIToNNN(opcode);
        break;
      case 0xD000:
        dxyn(opcode);
        break;
      case 0xE000:
        switch(opAcode & 0x00F0){
          case 0x0090:
            skipIfKeyPressed(opcode);
            break;
          case 0x00A0:
            printf("Opcode not handled\n");
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
                //FX0A	KeyOp	Vx = get_key()	A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
                printf("Waiting for keypress and storing in VX\n");
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
                printf("I += Vx");
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
            printf("Stores V0 to Vx in memory starting at adress I \n");
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

WORD getNextOpcode() {
  printf("%#06x ", pc);
  WORD opcode = 0;
  opcode = memory[pc++];
  opcode <<= 8;
  opcode |= memory[pc++];
  printf("%#06x ", opcode);
  return opcode;
}
