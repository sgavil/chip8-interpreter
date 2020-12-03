#include "chip8.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

void chip8::initialize()
{
	pc = 0x200;		// Program counter starts at 0x200
	opcode = 0;		// Reset current opcode	
	I = 0;			// Reset index register
	sp = 0;			// Reset stack pointer

	// Clear display
	for (int i = 0; i < 2048; ++i)
		gfx[i] = 0;

	// Clear stack
	for (int i = 0; i < 16; ++i)
		stack[i] = 0;

	for (int i = 0; i < 16; ++i)
		key[i] = V[i] = 0;

	// Clear memory
	for (int i = 0; i < 4096; ++i)
		memory[i] = 0;

	// Load fontset fontset should be loaded in memory location 0x50 == 80
	for (int i = 0; i < 80; ++i)
		memory[i] = chip8_fontset[i];

	// Reset timers
	delay_timer = 0;
	sound_timer = 0;

	// Clear screen once
	draw_flag = true;

	srand(time(NULL));
}

void chip8::emulate_cycle()
{
	// Fetch Opcode
	fetch_code();

	// Decode and execute Opcode
	decode_execute_code();

	// Update timers
	timer_updates();
}

void chip8::timer_updates()
{
	if (delay_timer > 0)
		--delay_timer;

	if (sound_timer > 0)
	{
		if (sound_timer == 1)
			printf("BEEP!\n");
		--sound_timer;
	}
}

void chip8::fetch_code()
{
	/*data is stored in an array in which each address contains one byte.
	As one opcode is 2 bytes long, we will need to fetch two successive bytes and
	merge them to get the actual opcode.*/

	opcode = memory[pc] << 8 | memory[pc + 1];
}

void chip8::decode_execute_code()
{
	/*Because every instruction is 2 bytes long, we need to increment the program counter by two after every executed opcode.
	This is true unless you jump to a certain address in the memory or if you call a subroutine*/

	switch (opcode & 0xF000)
	{
	case 0x0000:
	{
		switch (opcode & 0x000F)
		{
		case 0x0000: // 0x00E0: Clears the screen        

			break;

		case 0x000E: // 0x00EE: Returns from subroutine          

			break;
		default:
			break;
		}
	}

	case 0x2000: //Calls subroutine at NNN.
	{
		stack[sp] = pc;
		++sp; // increase the stack pointer to prevent overwriting the current stack
		pc = opcode & 0x0FFF;
		break;
	}

	case 0x8000:
	{
		switch (opcode & 0x000F)
		{
		case 0x0004: //Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't.
		{
			//If the sum of VX and VY is larger than 255, we use the carry flag to 
			//let the system know that the total sum of both values was indeed larger than 255
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
				V[0xF] = 1; //carry
			else
				V[0xF] = 0;
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		}


		default:
			break;
		}
	}

	case 0xA000: // ANNN: Sets I to the address NNN
	{
		I = opcode & 0x0FFF;
		pc += 2;
		break;
	}

	case 0xD000: // Draws a sprite at coordinate (VX, VY) of 8xN+1 pixels	
	{
		//We use a XOR operation to avoid rendering the same if it is there yet
		//If the current value is different from the value in the memory, the bit value will be 1. If both values match, the bit value will be 0.

		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		V[0xF] = 0;
		for (int yline = 0; yline < height; yline++)
		{
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)
						V[0xF] = 1;
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		draw_flag = true;
		pc += 2;
		break;
	}

	case 0xE000: 
	{

		switch (opcode & 0x00FF)
		{
			// EX9E: Skips the next instruction 
			// if the key stored in VX is pressed
		case 0x009E:
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
				pc += 4;
			else
				pc += 2;
			break;
		}
	}
	case 0xF000:
	{
		switch (opcode & 0x00FF)
		{

		/*Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I,
		the middle digit at I plus 1, and the least significant digit at I plus 2*/
		case 0x0033:
		{
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
			break;
		}
		default:
			break;
		}
	}

	default:
		printf("Unknown opcode: 0x%X\n", opcode);
		break;
	}
}
