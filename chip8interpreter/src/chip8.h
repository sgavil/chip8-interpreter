#pragma once


/*Chip8 interpreter, machine specs can be found at http://en.wikipedia.org/wiki/CHIP-8#Virtual_machine_description */
class chip8 {

public:
	chip8();

	// Initialize registers and memory once
	void initialize();

	//Simlates a cycle
	void emulate_cycle();

	//Updates the timers
	void timer_updates();

private:
	void fetch_code();
	void decode_execute_code();

	//To store current opcode which are 2bytes long (there are 35 opcodes)
	unsigned short opcode;

	//It has 4K memory 
	unsigned char memory[4096];

	//CPU Registers The Chip 8 has 15 8-bit general purpose registers named V0,V1 up to VE. The 16th register is used  for the ‘carry flag’
	unsigned char V[16];

	//Index register I and program counter (pc) values from 0x000 to 0xFFF 12-bits
	unsigned short I;
	unsigned short pc;

	/*
	----------- Systems memory map ---------------------------
	0x000-0x1FF - Chip 8 interpreter (contains font set in emu)
	0x050-0x0A0 - Used for the built in 4x5 pixel font set (0-F)
	0x200-0xFFF - Program ROM and work RAM
	
	*/

	//The screen has a total of 2048 pixels (64 x 32)
	unsigned char gfx[64 * 32];

	//Two timer registers that count at 60 Hz. When set above zero they will count down to zero.
	unsigned char delay_timer;
	unsigned char sound_timer; //The system’s buzzer sounds whenever the sound timer reaches zero.

	//Stack used to remember the current location before a jump is performed
	//The system has 16 levels of stack and in order to remember which level of the stack is used, we need to implement a stack pointer (sp).
	unsigned short stack[16];
	unsigned short sp;

	//HEX based keypad (0x0-0xF)
	unsigned char key[16];

	//Flag that is active when we need to redraw
	bool draw_flag = false;

	const unsigned char chip8_fontset[80] =
	{
	  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	  0x20, 0x60, 0x20, 0x20, 0x70, // 1
	  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};
};