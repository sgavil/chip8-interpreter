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
	clear_display();

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
		{
			clear_display();
			pc += 2;
			break;
		}
			

		case 0x000E: // 0x00EE: Returns from subroutine          
		{
			--sp; //To prevent overwrite
			pc = stack[sp];
			pc += 2;
			break;
		}

		default:
			printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
			break;
		}
	}

	case 0x1000: //Jumps to address NNN.
	{
		pc = opcode & 0x0FFF;
		break;
	}

	case 0x2000: //Calls subroutine at NNN.
	{
		stack[sp] = pc;
		++sp; // increase the stack pointer to prevent overwriting the current stack
		pc = opcode & 0x0FFF;
		break;
	}

	case 0x3000: // 3XNN: Skips the next instruction if VX equals NN
	{
		if ((V[opcode & 0x0F00 >> 8] ) == (opcode & 0x00FF))
			pc += 4;
		else pc += 2;
		break;
	}

	case 0x4000: // 4XNN: Skips the next instruction if VX doesn't equal NN
	{
		if ((V[opcode & 0x0F00 >> 8] ) != (opcode & 0x00FF))
			pc += 4;
		else pc += 2;
		break;
	}

	case 0x5000: // 5XY0: Skips the next instruction if VX equals VY
	{
		if ((V[opcode & 0x0F00 >> 8] ) == V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else pc += 2;
		break;
	}

	case 0x6000: // 6XNN: Sets VX to NN.
	{
		V[opcode & 0x0F00 >> 8] = opcode & 0x00FF;
		pc += 2;
		break;
	}

	case 0x7000: // 7XNN: Adds NN to VX. (Carry flag is not changed)
	{
		V[opcode & 0x0F00 >> 8] += opcode & 0x00FF;
		pc += 2;
		break;
	}

	case 0x8000:
	{
		switch (opcode & 0x000F)
		{
		case 0x0000: // 8XY0 : Sets VX to the value of VY.
		{
			V[opcode & 0x0F00 >> 8] = V[opcode & 0x00F0 >> 4];
			pc += 2;
			break;
		}
		case 0x0001: // 8XY1 : Sets VX to VX or VY. (Bitwise OR operation)
		{
			V[opcode & 0x0F00 >> 8] |= V[opcode & 0x00F0 >> 4];
			pc += 2;
			break;
		}
		case 0x0002: // 8XY2 : Sets VX to VX and VY. (Bitwise AND operation)
		{
			V[opcode & 0x0F00 >> 8] &= V[opcode & 0x00F0 >> 4];
			pc += 2;
			break;
		}
		case 0x0003: // 8XY3 : Sets VX to VX xor VY.
		{
			V[opcode & 0x0F00 >> 8] ^= V[opcode & 0x00F0 >> 4];
			pc += 2;
			break;
		}
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
		case 0x0005: // 8XY5 : VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		{
			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
				V[0xF] = 0; // there is a borrow
			else
				V[0xF] = 1;
			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;
		}
		case 0x0006: // 8XY6 : Stores the least significant bit of VX in VF and then shifts VX to the right by 1
		{
			V[0xF] = V[opcode & 0x0F00 >> 8] & 0x1;
			V[opcode & 0x0F00 >> 8] >>= 1;
			pc += 2;
			break;
		}
		case 0x0007: // 8XY7 : Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't.
		{
			if (V[opcode & 0x0F00 >> 8] > V[opcode & 0x00F0 >> 4]) //if VX > VY there is borrow
				V[0xF] = 0;
			else V[0xF] = 1;

			V[opcode & 0x0F00 >> 8] = V[opcode & 0x00F0 >> 4] - V[opcode & 0x0F00 >> 8];
			pc += 2;
			break;
		}
		case 0x000E: //8XYE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1
		{
			V[0xF] = V[opcode & 0x0F00 >> 8] >> 7;
			V[opcode & 0x0F00 >> 8] <<= 1;
			pc += 2;
			break;
		}

		default:
			break;
		}
	}
	case 0x9000: // 9XY0: Skips the next instruction if VX doesn't equal VY
	{
		if (V[opcode & 0x0F00 >> 8] != V[opcode & 0x00F0 >> 4])
			pc += 4;
		else pc += 2;
		break;
	}

	case 0xA000: // ANNN: Sets I to the address NNN
	{
		I = opcode & 0x0FFF;
		pc += 2;
		break;
	}

	case 0xB000: // BNNN: Jumps to the address NNN plus V0.
	{
		pc = (opcode & 0x0FFF) + V[0x0];
		break;
	}

	case 0xC000: // CNNN: Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
	{
		V[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
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
					gfx[(x + xline + ((y + yline) * 64)) % (64 * 32)] ^= 1;
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
			
			
		case 0x009E: // EX9E: Skips the next instruction if the key stored in VX is pressed
		{
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
				pc += 4;
			else
				pc += 2;
			break;
		}
		case 0x00A1: // EXA1: Skips the next instruction if the key stored in VX isn't pressed
		{
			if (key[V[(opcode & 0x0F00) >> 8]] == 0)
				pc += 4;
			else
				pc += 2;
			break;
		}

		default:
			printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
			break;
		}
	}
	case 0xF000:
	{
		switch (opcode & 0x00FF)
		{
		case 0x0007: // FX07: Sets VX to the value of the delay timer.
		{
			V[opcode & 0x0F00 >> 8] = delay_timer;
			pc += 2;
			break;
		}
		case 0x000A: // FX0A: A key press is awaited, and then stored in VX
		{
			bool key_pressed = false;

			for (int i = 0; i < 16; ++i)
			{
				if (key[i] != 0)
				{
					V[(opcode & 0x0F00) >> 8] = i;
					key_pressed = true;
				}
			}

			// If we didn't received a keypress, skip this cycle and try again.
			if (!key_pressed)
				return;

			pc += 2;
			break;
		}
		case 0x0015: // FX15: Sets the delay timer to VX.
		{
			delay_timer = V[opcode & 0x0F00 >> 8];
			pc += 2;
			break;
		}
		case 0x0018: // FX18: Sets the sound timer to VX
		{
			sound_timer = V[opcode & 0x0F00 >> 8];
			pc += 2;
			break;
		}
		case 0x001E: // FX1E: Adds VX to I. VF is not affected
		{
			I += V[opcode & 0x0F00 >> 8];
			pc += 2;
			break;
		}

		case 0x0029: // FX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
		{
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			break;
		}

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

		case 0x0055: // FX55: Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 
						//1 for each value written, but I itself is left unmodified.
		{
			for (int i = 0; i <= (opcode & 0x0F00); i++)
				memory[I + i] = V[i];

			pc += 2;
			break;
		}

		case 0x0065: // FX65: Fills V0 to VX (including VX) with values from memory starting at address I. 
					//The offset from I is increased by 1 for each value written, but I itself is left unmodified
		{
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				V[i] = memory[I + i];

			pc += 2;
			break;
		}

		default:
			printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
			break;
		}
	}

	default:
		printf("Unknown opcode: 0x%X\n", opcode);
		break;
	}
}

void chip8::clear_display()
{
	for (int i = 0; i < 2048; ++i)
		gfx[i] = 0;

	draw_flag = true;
}

bool chip8::load_application(const char* filename)
{
	initialize();
	printf("Loading: %s\n", filename);

	// Open file
	FILE* pFile = fopen(filename, "rb");
	if (pFile == NULL)
	{
		fputs("File error", stderr);
		return false;
	}

	// Check file size
	fseek(pFile, 0, SEEK_END);
	long lSize = ftell(pFile);
	rewind(pFile);
	printf("Filesize: %d\n", (int)lSize);

	// Allocate memory to contain the whole file
	char* buffer = (char*)malloc(sizeof(char) * lSize);
	if (buffer == NULL)
	{
		fputs("Memory error", stderr);
		return false;
	}

	// Copy the file into the buffer
	size_t result = fread(buffer, 1, lSize, pFile);
	if (result != lSize)
	{
		fputs("Reading error", stderr);
		return false;
	}

	// Copy buffer to Chip8 memory
	if ((4096 - 512) > lSize)
	{
		for (int i = 0; i < lSize; ++i)
			memory[i + 512] = buffer[i];
	}
	else
		printf("Error: ROM too big for memory");

	// Close file, free buffer
	fclose(pFile);
	free(buffer);

	return true;
}
