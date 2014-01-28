/* LC3 simulator file.
 * Complete the functions!
 * This is a front-end to the code you wrote in lc3.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "lc3.h"

void cmd_registers(lc3machine* mach);
void cmd_dump(lc3machine* mach, int start, int end);
void cmd_list(lc3machine* mach, int start, int end);
void cmd_setaddr(lc3machine* mach, int address, short value);
void cmd_setreg(lc3machine* mach, int reg, short value);


/* FIXME: change this to be a good prompt string */
#define PROMPT "Please enter a command: "

// Don't touch I will use this during grading.
#ifndef NO_MAIN_PLZ
int main(int argc, char **argv){

	const char* prog;

	/* We expect only one argument for the program... */
	if (argc != 2) 
	{
		fprintf(stderr, "Usage: %s file.obj\n", argv[0]);
		return 1;
	}
	/* We want to open the file and make sure that it exists. */
	prog = argv[1]; 
	
	/* Create a variable of type lc3machine here */
	lc3machine* machine = malloc(sizeof(lc3machine));
	/* Make a call to lc3_init to initialize your lc3machine */
	lc3_init(machine);
	//load the machine with the program
	lc3_load(machine, prog);

	/* FIXME: add your name here! */
	printf("LC-3 simulator and debugger\n");
	printf("Written by Nick Keller\n");
	printf("File given %s\n", prog);

	/* Run this loop until we are told to stop going. */
	char buffer[1024];
	char cmd[1024];
	char arg1[1024];
	char arg2[1024];
	printf("%s", PROMPT);
	while(fgets(buffer, sizeof(buffer), stdin)){
		//separate
		int numArgs = sscanf(buffer, "%s %s %s", cmd, arg1, arg2);
		//printf("Command = %s, arg1 = %s, arg2 = %s, numArgs = %d\n",cmd, arg1, arg2, numArgs);
		if(0 == strcasecmp(cmd, "quit")){
			printf("Goodbye!\n");
			break;
		}
		//did the user enter step?
		else if(0 == strcasecmp(cmd, "step")){
			//did the user specify a number of steps
			if(2 == numArgs){
				int numSteps = atoi(arg1);
				lc3_run(machine, numSteps);
			}
			//didn't specify, step one
			else{
				lc3_step_one(machine);
			}
		}
		
		//check to see if the user entered "continue"
		else if(0 == strcasecmp(cmd, "continue")){
			lc3_continue(machine);
		}
		
		//check to see if the user entered "registers"
		else if(0 == strcasecmp(cmd, "registers")){
			cmd_registers(machine);
		}
		
		//check to see if the user entered a dump command
		else if(0 == strcasecmp(cmd, "dump")){
			//check for number of arguments
			if(numArgs == 2){
				int start = (int)(strtol(&arg1[1],NULL,16));
				cmd_dump(machine, start, -1);
			}
			else{
				int start = (int)(strtol(&arg1[1],NULL,16));
				int end = (int)(strtol(&arg2[1],NULL,16));
				cmd_dump(machine, start, end);
			}
		}
		
		//check to see if the user entered a list command
		else if(0 == strcasecmp(cmd, "list")){
			//check for number of arguments
			if(numArgs == 2){
				int start = (int)(strtol(&arg1[1],NULL,16));
				cmd_list(machine, start, -1);
			}
			else{
				int start = (int)(strtol(&arg1[1],NULL,16));
				int end = (int)(strtol(&arg2[1],NULL,16));
				cmd_list(machine, start, end);
			}
		}
		
		//check and see if the user entered a setaddr command
		else if(0 == strcasecmp(cmd, "setaddr")){
			int address = (int)(strtol(&arg1[1],NULL,16));
			short value = (short)(atoi(arg2) & 0x0000FFFF);
			cmd_setaddr(machine, address, value);
		}
		
		//check and see fi the user entered a setreg command
		else if(0 == strcasecmp(cmd, "setreg")){
			const char* numReg = &arg1[1];
			int reg = atoi(numReg);
			short value = (short)(atoi(arg2) & 0x0000FFFF);
			cmd_setreg(machine, reg, value);
		}
		
		printf("%s", PROMPT);
		//clear the input buffers for the next go around
		bzero(buffer, sizeof(buffer));
		bzero(cmd, sizeof(cmd));
		bzero(arg1, sizeof(arg1));
		bzero(arg2, sizeof(arg2));
	}
	free(machine);
	return 0;
}
#endif // IFNDEF NO_MAIN_PLZ

/* cmd_step and cmd_continue 's functionality are provided in lc3_run
Therefore to execute the step and coninute commands you can just call lc3_run with the correct parameters*/

/* cmd_registers
Should print out all of the registers and the PC and CC in both hex and signed decimal. The format should be as follows

R0 dec|hex	R1 dec|hex	R2 dec|hex	R3 dec|hex
R4 dec|hex	R5 dec|hex	R6 dec|hex	R7 dec|hex
CC n/z/p
PC hex

hex is to be preceded with only an x and you should always print out 4 characters representing the hexadecimal representation the hex letters are CAPITALIZED.
n/z/p will only be one of the characters n, z, or p
between each register's information is a single tab.

Example output 
R0 0|x0000	R1 -1|xFFFF	R2 2|x0002	R3 32767|x7FFF
R4 31|x001F	R5 -32768|x8000	R6 6|x0006	R7 11111|x2B67
CC z
PC x3000
*/
void cmd_registers(lc3machine* mach){
	for(int i = 0; i < 4; i++){
		short temp = mach->regs[i];
		printf("R%d %d|x%04x ", i, temp, temp);
	}
	printf("\n");
	for(int i = 4; i < 8; i++){
		short temp = mach->regs[i];
		printf("R%d %d|x%04x ", i, temp, temp);
	}
	printf("\n");
	char* cc = "p";
	if(mach->cc == 4){
		cc = "n";
	}
	if(mach->cc == 2){
		cc = "z";
	}
	printf("CC %s\n", cc);
	printf("PC x%04x\n", mach->pc);
}

/* cmd_dump
Should print out the contents of memory from start to end
If end is -1 then just print out memory[start]

Output format
addr: dec|hex

Example format
x0000: 37|x0025
x0001: 25|x0019
*/
void cmd_dump(lc3machine* mach, int start, int end){
	if(end == -1){
		unsigned short temp = mach->mem[start];
		printf("x%04x: %d|x%04x\n", start, temp, temp);
	}
	else{
		for(int i = start; i <= end; i++){
			unsigned short temp = mach->mem[i];
			printf("x%04x: %d|x%04x\n", i, temp, temp);
		}
	}
	printf("\n");
}

/* cmd_list
Should interpret the contents of memory from start to end as an assembled instruction
and disassemble it. e.g. if the data was x1000 then the output will be x3000
If end is -1 then just disassemble memory[start]

You will be calling lc3_disassemble to do the actual disassembling!
*/
void cmd_list(lc3machine* mach, int start, int end){
	if(end == -1){
		lc3_disassemble(mach->mem[start]);
	}
	else{
		for(int i = start; i <= end; i++){
			lc3_disassemble(mach->mem[i]);
		}
	}
}

/* cmd_setaddr
 Should set a memory address to some value
*/
void cmd_setaddr(lc3machine* mach, int address, short value){
	mach->mem[address] = value;
	printf("Memory address %d set to %d\n", address, value);
}

/* cmd_setreg
  Should set a register to some value passed in
*/
void cmd_setreg(lc3machine* mach, int reg, short value){
	mach->regs[reg] = value;
	printf("Register %d set to %d\n", reg, value);
}

