#include <strings.h>
#include <stdlib.h>
#include "lc3.h"
//declare an array of function pointers to lc3 actions
void (*lc3_actions[16])(lc3machine* state, unsigned short instruction) = {lc3_br, lc3_add, lc3_ld, lc3_st, lc3_jsr, lc3_and, lc3_ldr, lc3_str, junk, lc3_not, lc3_ldi, lc3_sti, lc3_jmp, junk, lc3_lea, trap};
//declare an array of function pointers to the disassembling actions
void (*lc3_disassemble_actions[16])(unsigned short instruction) = {lc3_disassemble_br, lc3_disassemble_add, lc3_disassemble_ld, lc3_disassemble_st, lc3_disassemble_jsr, lc3_disassemble_and, lc3_disassemble_ldr, lc3_disassemble_str, lc3_disassemble_junk, lc3_disassemble_not, lc3_disassemble_ldi, lc3_disassemble_sti, lc3_disassemble_jmp, lc3_disassemble_junk, lc3_disassemble_lea, lc3_disassemble_trap};

void lc3_init(lc3machine* state) 
{
	// Initialize the lc3 state according to the assignment.
	state->pc = 0x3000;
	state->cc = LC3_ZERO;
	state->halted = 0;
}

void lc3_load(lc3machine* state, const char* program)
{
	/* FIXME: Add in the code to initialize the memory according to the
	          contents of the program. */
	// Reread the section on the .obj file format.
	// Note you will have to load the file from disk and do File I/O.
	
	//open the file
	//printf("opening file %s\n", program);
	FILE *prog = fopen(program, "rb");
	if(prog == NULL){
		printf("Error, %s doesn't exist!\n", program);
		exit(1);
	}
	//allocate space for the shorts to be read in
	char buffer[3];
	char addrBuffer[3];
	char elementBuffer[3];
	char bstop = 0;
	//run the loop until end of file
	while(!bstop){
		//read in the starting address and number of elements
		addrBuffer[2] = fread(addrBuffer, sizeof(char), 2, prog);
		elementBuffer[2] = fread(elementBuffer, sizeof(char), 2, prog);
		//check for EOF
		if((addrBuffer[2] < 2) || (elementBuffer[2] < 2)){
			//EOF reached, break
			bstop = 1;
		}
		//pack the elements of addrBuffer into a short, don't need to reverse order 
		unsigned short address = (unsigned short)(addrBuffer[0]);
		//printf("address: %4x\n", address);
		address = address << 8;
		//printf("address: %4x\n", address);
		address |= (((unsigned short)(addrBuffer[1])) & 0x00FF);
		//printf("address: %4x\n", address);
		//pack the elements of elementBuffer into a short, don't need to reverse order
		unsigned short numElements = (unsigned short)(elementBuffer[1]);
		numElements = numElements << 8;
		numElements |= (((unsigned short)(elementBuffer[0])) & 0x00FF);
		//assign elements to memory
		for(int i = 0; i < numElements; i++, address++){
			//read in an element
			buffer[2] = fread(buffer, sizeof(char), 2, prog);
			//pack into a short
			unsigned short data = (unsigned short)(buffer[0]);
			data = data << 8;
			data |= (((unsigned short)(buffer[1])) & 0x00FF);
			data &= 0x0000FFFF;
			//place data in memory
			state->mem[address] = data;
			//printf("%4x, %4x written into address %4x\n",data, state->mem[address], address);
			//clear buffer
			bzero(buffer, sizeof(buffer));
		}
		//clear the address and element buffers
		bzero(addrBuffer, sizeof(addrBuffer));
		bzero(elementBuffer, sizeof(elementBuffer));
		
	}
	//close the file
	fclose(prog);
}

void lc3_step_one(lc3machine* state){
	// If the machine is not halted
	// Fetch an instruction
	// And call lc3_execute 
	if(!state->halted){
		unsigned short instruction = lc3_fetch(state);
		lc3_execute(state, instruction);
	}
}

void lc3_run(lc3machine* state, int num_steps){
	for(int i = 0; i < num_steps; i++){
		unsigned short instruction = lc3_fetch(state);
		lc3_execute(state, instruction);
	}
}

void lc3_continue(lc3machine* state){
	while(!state->halted){
		lc3_step_one(state);
	}
}

unsigned short lc3_fetch(lc3machine* state)
{
	/* Think back to when we first started assembly to code this */
	/* What happens during the fetch stage? */
	unsigned short instruction = state->mem[state->pc];
	state->pc++;
	return instruction;
}

void lc3_execute(lc3machine* state, unsigned short instruction){
	//if the machine isn't halted, do the command
	if(!state->halted){
		if(DEBUG)printf("Instruction: ");
		if(DEBUG)lc3_disassemble(instruction);
		//get the opcode from the instruction
		unsigned short opcode = ((instruction >> 12) & 0x000F);
		//call the appropriate function in the array
		lc3_actions[opcode](state, instruction);
	}
}

void lc3_disassemble(unsigned short instruction){
	//get the opcode from the instruction
	unsigned short opcode = (instruction >> 12) & 0x000F;
	//call the appropriate disassembling function
	lc3_disassemble_actions[opcode](instruction);
}

void lc3_trap(lc3machine* state, unsigned char vector8){
	//check and see what vector is being done
	//check for GETC
	if(vector8 == 0x20){
		//read a character in from stdin, and place it in R0
		int character = getchar();
		state->regs[0] = (char)(character & 0x000000FF);
	}
	//check for OUT
	else if(vector8 == 0x21){
		//print out the character in R0
		printf("%c\n", (char)(state->regs[0]));
	}
	//check for PUTS
	else if(vector8 == 0x22){
		//print out characters, starting with the address contained in R0
		unsigned short address = state->regs[0];
		while(state->mem[address] != 0x0000){
			printf("%c", (char)((state->mem[address]) & 0x00FF));
			address++;
		}
		printf("\n");
	}
	//check for IN
	else if(vector8 == 0x23){
		//print a prompt and read in a character
		printf("Please enter in a character: ");
		int temp = getchar();
		char value = (char)(temp & 0x000000FF);
		printf("%c\n", value);
		//store the character in R0
		state->regs[0] =(short)(temp & 0x000000FF);
	}
	//check for PUTSP
	else if(vector8 == 0x24){
		unsigned short address = state->regs[0];
		while(state->mem[address] != 0x0000){
			unsigned short word = state->mem[address];
			unsigned char first = (char)(word & 0x00FF);
			unsigned char second = (char)((word >> 8) & 0x00FF);
			if(second == 0x00){
				printf("%c",first);
			}
			else{
				printf("%c%c", first, second);
			}
		}
		printf("\n");
	}
	//check for HALT
	else if(vector8 == 0x25){
		//halt the machine
		state->halted = 1;
		//decrement the pc
		state->pc--;
		printf("The machine is now halted\n");
	}
	//basic trap
	else{
		state->regs[7] = state->pc;
		unsigned short address = (((unsigned short)(vector8)) & 0x00FF);
		state->pc = address;
	}
	
}

void setcc(lc3machine* state, unsigned short dr){
	if(state->regs[dr] < 0){
		state->cc = LC3_NEGATIVE;
		if(DEBUG)printf("CC is now n\n");
	}
	else if(state->regs[dr] == 0){
		state->cc = LC3_ZERO;
		if(DEBUG)printf("CC is now z\n");
	}
	else{
		state->cc = LC3_POSITIVE;
		if(DEBUG)printf("CC is now p\n");
	}
}

//executing functions

//this function doesn't do anything, it's just here to make execution easy
void junk(lc3machine* state, unsigned short instruction){
	printf("x%4x is not a valid instruction", instruction);
}

void lc3_br(lc3machine* state, unsigned short instruction){
	unsigned char nzp = (unsigned char)((instruction >> 9) & 0x07);
	if((state->cc & nzp) != 0){
		//at least one of the conditions were met
		short PCoffset9 = instruction & 0x01FF;//isolate PCoffset9
		//sign extend PCoffset9
		if((PCoffset9 >> 8) & 1){
			PCoffset9 |= 0xFE00;
		}
		if(DEBUG)printf("PC = x%04x\n", state->pc);
		state->pc += PCoffset9;
		if(DEBUG)printf("PC = x%04x\n", state->pc);
	}
}

void lc3_add(lc3machine* state, unsigned short instruction){
	//test to see if imm5 or register based
	if((instruction >> 5) & 1){
		//imm5 based
		short imm5 = (instruction & 0x001F);
		
		if(DEBUG)printf("imm5: x%04x\n", imm5);
		//sign extend imm5
		if((imm5 >> 4) & 1){
			imm5 |= 0xFFE0;
		}
		if(DEBUG)printf("imm5: x%04x\n", imm5);
		unsigned short sr1 = ((instruction >> 6) & 0x0007);
		unsigned short dr = ((instruction >> 9) & 0x0007);
		short sum = imm5 + state->regs[sr1];
		if(DEBUG)printf("R%d = %d, adding %d\n", dr, state->regs[dr], imm5);
		state->regs[dr] = sum;
		if(DEBUG)printf("R%d now equals %d\n", dr, state->regs[dr]);
		//modify the condition code appropriately
		setcc(state, dr);
	}
	else{
		//register based
		unsigned short sr2 = instruction & 0x0007;
		unsigned short sr1 = (instruction >> 6) & 0x0007;
		unsigned short dr = (instruction >> 9) & 0x0007;
		short sum = state->regs[sr1] + state->regs[sr2];
		if(DEBUG)printf("R%d = %d, adding %d\n", dr, state->regs[dr], sum);
		state->regs[dr] = sum;
		if(DEBUG)printf("R%d now equals %d\n", dr, state->regs[dr]);
		//modify the condition code appropriately
		setcc(state, dr);
	}
}

void lc3_ld(lc3machine* state, unsigned short instruction){
	//get the destination register
	unsigned short dr = (instruction >> 9) & 0x0007;
	short PCoffset9 = (instruction & 0x01FF);//isolate PCoffset9
	//sign extend PCoffset9
	if((PCoffset9 >> 8) & 1){
		PCoffset9 |= 0xFE00;
	}
	//set dr to mem[PC + PCoffset9]
	if(DEBUG)printf("R%d = %d, going to be set to %d\n", dr, state->regs[dr], state->mem[state->pc + 			PCoffset9]); 
	state->regs[dr] = state->mem[state->pc + PCoffset9];
	if(DEBUG)printf("R%d set to %d\n", dr, state->regs[dr]); 
	//set the condition code
	setcc(state, dr);
}

void lc3_st(lc3machine* state, unsigned short instruction){
	//calculate PCoffset9 and source register
	short PCoffset9 = (instruction & 0x01FF);//isolate PCoffset9
	//sign extend PCoffset9
	if((PCoffset9 >> 8) & 1){
		PCoffset9 |= 0xFE00;
	}
	unsigned short sr = ((instruction >> 9) & 0x0007);
	//calculate memory address to store to
	unsigned short address = state->pc + PCoffset9;
	//store the value
	if(DEBUG)printf("Storing %d at address x%04x\n", state->regs[sr], address);
	state->mem[address] = state->regs[sr];
	if(DEBUG)printf("Address x%04x is now %d\n", address, state->mem[address]);
}

void lc3_jsr(lc3machine* state, unsigned short instruction){
	//save off the PC
	state->regs[7] = state->pc;
	//test to see if offset or register based
	if((instruction >> 11) & 1){
		//offset based
		short PCoffset11 = (instruction & 0x07FF);//isolate PCoffset11
		//sign extend PCoffset11
		if((PCoffset11 >> 10) & 1){
			PCoffset11 |= 0xF800;
		}
		if(DEBUG)printf("PC = x%04x, going to add %d\n", state->pc, PCoffset11);
		state->pc += PCoffset11;
		if(DEBUG)printf("PC set to x%04x\n", state->pc);
	}
	else{
		//register based
		//get the base register
		unsigned short baseR = (instruction >> 6) & 0x0007;
		//set the pc to the value stored in the base register
		if(DEBUG)printf("PC = x%04x, going to set to %d\n", state->pc, state->regs[baseR]);
		state->pc = state->regs[baseR];
		if(DEBUG)printf("PC set to x%04x\n", state->pc);
	}
}

void lc3_and(lc3machine* state, unsigned short instruction){
	//test to see if imm5 or register based
	if((instruction >> 5) & 1){
		//imm5 based
		signed short imm5 = (instruction & 0x001F);//isolate imm5
		if(DEBUG)printf("imm5: x%04x\n", imm5);
		//sign extend PCoffset9
		if((imm5 >> 4) & 1){
			imm5 |= 0xFFE0;
		}
		if(DEBUG)printf("imm5: x%04x\n", imm5);
		unsigned short sr1 = ((instruction >> 6) & 0x0007);
		unsigned short dr = ((instruction >> 9) & 0x0007);
		short sum = imm5 & state->regs[sr1];
		if(DEBUG)printf("R%d to be set to R%d(x%04x) & x%04x = x%04x\n", dr, sr1, state->regs[sr1], imm5, 					sum); 
		state->regs[dr] = sum;
		if(DEBUG)printf("R%d set to x%04x\n", dr, state->regs[dr]);
		setcc(state, dr);
	}
	else{
		//register based
		unsigned short sr2 = instruction & 0x0007;
		unsigned short sr1 = (instruction >> 6) & 0x0007;
		unsigned short dr = (instruction >> 9) & 0x0007;
		short sum = state->regs[sr1] & state->regs[sr2];
		if(DEBUG)printf("R%d to be set to R%d(x%04x) & R%d(x%04x) = x%04x\n", dr, sr1, state->regs[sr1], sr2, state->regs[sr2], sum); 
		state->regs[dr] = sum;
		if(DEBUG)printf("R%d set to x%04x\n", dr, state->regs[dr]);
		setcc(state, dr);
	}
}

void lc3_ldr(lc3machine* state, unsigned short instruction){
	//calculate offset6, base and destination register
	unsigned short dr = (instruction >> 9) & 0x0007;
	unsigned short baseR = (instruction >> 6) & 0x0007;
	short PCoffset6 = (instruction & 0x003F);//isolate PCoffset6
	//sign extend PCoffset6
	if((PCoffset6 >> 5) & 1){
		PCoffset6 |= 0xFFC0;
	}
	//calculate address to load from
	unsigned short address = state->regs[baseR] + PCoffset6;
	//assign value stored in address to dr
	if(DEBUG)printf("R%d to be set to address x%04x = %d\n", dr, address, state->mem[address]);
	state->regs[dr] = state->mem[address];
	if(DEBUG)printf("R%d set to %d\n", dr, state->regs[dr]);
	//modify the cc appropriately
	setcc(state, dr);
}

void lc3_str(lc3machine* state, unsigned short instruction){
	//calculate offset6, baseR, and sr
	short PCoffset6 = (instruction & 0x003F);//isolate PCoffset6
	//sign extend PCoffset6
	if((PCoffset6 >> 5) & 1){
		PCoffset6 |= 0xFFC0;
	}
	unsigned short baseR = (instruction >> 6) & 0x0007;
	unsigned short sr = (instruction >> 9) & 0x0007;
	//calculate address to store to
	unsigned short address = state->regs[baseR] + PCoffset6;
	//store the value located in sr into address
	if(DEBUG)printf("Storing value %d to address x%04x\n", state->regs[sr], address);
	state->mem[address] = state->regs[sr];
	if(DEBUG)printf("Address x%04x now equals %d\n", address, state->mem[address]);
}

void lc3_not(lc3machine* state, unsigned short instruction){
	//get the source and destination register
	unsigned short sr = (instruction >> 6) & 0x0007;
	unsigned short dr = (instruction >> 9) & 0x0007;
	//do the operation
	if(DEBUG)printf("R%d to be set to ~R%d = x%04x \n", dr, sr, state->regs[sr]);
	state->regs[dr] = ~(state->regs[sr]);
	if(DEBUG)printf("R%d set to x%04x\n", dr, state->regs[dr]);
	//set the cc to the appropriate value
	setcc(state, dr);
}

void lc3_ldi(lc3machine* state, unsigned short instruction){
	//get the destination register
	unsigned short dr = (instruction >> 9) & 0x0007;
	short PCoffset9 = (instruction & 0x01FF);//isolate PCoffset9
	//sign extend PCoffset9
	if((PCoffset9 >> 8) & 1){
		PCoffset9 |= 0xFE00;
	}
	//calculate first and second addresses
	unsigned short address1 = state->pc + PCoffset9;
	unsigned short address2 = state->mem[address1];
	//assign a value to dr
	if(DEBUG)printf("R%d to be set to mem[mem[x%04x]] = %d\n", dr, address1, state->mem[address2]);
	state->regs[dr] = state->mem[address2];
	if(DEBUG)printf("R%d set to %d\n", dr, state->regs[dr]);
	//change the cc
	setcc(state, dr);
	
}

void lc3_sti(lc3machine* state, unsigned short instruction){
	//calculate PCoffset9 and source register
	short PCoffset9 = (instruction & 0x01FF);//isolate PCoffset9
	//sign extend PCoffset9
	if((PCoffset9 >> 8) & 1){
		PCoffset9 |= 0xFE00;
	}
	unsigned short sr = (instruction >> 9) & 0x0007;
	//calculate first memory address
	unsigned short address1 = state->pc + PCoffset9;
	//calculate second memory address
	unsigned short address2 = state->mem[address1];
	//finally, store the value in sr into address2
	if(DEBUG)printf("Storing value %d to mem[mem[x%04x]]\n", state->regs[sr], address1);
	state->mem[address2] = state->regs[sr];
	if(DEBUG)printf("mem[mem[x%04x]] set to %d\n", address1, state->mem[address2]);
}

void lc3_jmp(lc3machine* state, unsigned short instruction){
	//get the base register
	unsigned short baseR = (instruction >> 6) & 0x0007;
	//set the pc to the value stored in the base register
	if(DEBUG)printf("PC = x%04x, to be set to x%04x\n", state->pc, state->regs[baseR]);
	state->pc = state->regs[baseR];
	if(DEBUG)printf("PC set to x%04x\n", state->pc);
}

void lc3_lea(lc3machine* state, unsigned short instruction){
	//calculate offset and destination register
	short PCoffset9 = (instruction & 0x01FF);//isolate PCoffset9
	//sign extend PCoffset9
	if((PCoffset9 >> 8) & 1){
		PCoffset9 |= 0xFE00;
	}
	unsigned short dr = (instruction >> 9) & 0x0007;
	//load the new address into dr
	state->regs[dr] = state->pc + PCoffset9;
	//set the condition code to the appropriate value
	setcc(state, dr);
}

void trap(lc3machine* state, unsigned short instruction){
	unsigned char vector8 = (unsigned char)(instruction & 0x00FF);
	lc3_trap(state, vector8);
}

//disassembling functions
void lc3_disassemble_br(unsigned short instruction){
	unsigned char nzp = (unsigned char)((instruction >> 9) & 0x07);
	short PCoffset9 = instruction & 0x01FF;//isolate PCoffset9
	//sign extend PCoffset9
	if((PCoffset9 >> 8) & 1){
		PCoffset9 |= 0xFE00;
	}
	char* n = "";
	char* z = "";
	char* p = "";
	
	//check if p bit is set
	if(nzp & 1){
		p = "P";
	}
	
	nzp = nzp >> 1;
	//check to see if z bit is set
	if(nzp & 1){
		z = "Z";
	}
	
	nzp = nzp >> 1;
	//check to see if n bit is set
	if(nzp & 1){
		n = "N";
	}
	
	printf("BR%s%s%s %d\n",n,z,p,PCoffset9);
	
}

void lc3_disassemble_add(unsigned short instruction){
	//test to see if imm5 or register based
	if((instruction >> 5) & 1){
		//imm5 based
		short imm5 = (instruction & 0x001F);//isolate imm5
		//sign extend imm5
		if((imm5 >> 4) & 1){
			imm5 |= 0xFFE0;
		}
		unsigned short sr1 = ((instruction >> 6) & 0x0007);
		unsigned short dr = ((instruction >> 9) & 0x0007);

		printf("ADD R%d, R%d, %d\n", dr,sr1, imm5);
	}
	else{
		//register based
		unsigned short sr2 = instruction & 0x0007;
		unsigned short sr1 = (instruction >> 6) & 0x0007;
		unsigned short dr = (instruction >> 9) & 0x0007;
		printf("ADD R%d, R%d, R%d\n", dr,sr1, sr2);
	}
}

void lc3_disassemble_ld(unsigned short instruction){
	//get the destination register
	unsigned short dr = (instruction >> 9) & 0x0007;
	short PCoffset9 = (instruction & 0x01FF);//isolate PCoffset9
	//sign extend PCoffset9
	if((PCoffset9 >> 8) & 1){
		PCoffset9 |= 0xFE00;
	}
	printf("LD R%d, %d\n", dr, PCoffset9);
}

void lc3_disassemble_st(unsigned short instruction){
	//calculate PCoffset9 and source register
	short PCoffset9 = (instruction & 0x01FF);//isolate PCoffset9
	//sign extend PCoffset9
	if((PCoffset9 >> 8) & 1){
		PCoffset9 |= 0xFE00;
	}
	unsigned short sr = ((instruction >> 9) & 0x0007);
	printf("ST R%d, %d\n", sr, PCoffset9);
}

void lc3_disassemble_jsr(unsigned short instruction){
	//test to see if offset or register based
	if((instruction >> 11) & 1){
		//offset based
		short PCoffset11 = (instruction & 0x07FF);//isolate PCoffset11
		//sign extend PCoffset11
		if((PCoffset11 >> 10) & 1){
			PCoffset11 |= 0xF800;
		}
		printf("JSR %d\n", PCoffset11);
	}
	else{
		//register based
		//get the base register
		unsigned short baseR = (instruction >> 6) & 0x0007;
		printf("JSRR R%d\n", baseR);
		
	}
}

void lc3_disassemble_and(unsigned short instruction){
	//test to see if imm5 or register based
	if(instruction & 0x0020){
		//imm5 based
		short imm5 = (instruction & 0x001F);
		//sign extend imm5
		if((imm5 >> 4) & 1){
			imm5 |= 0xFFE0;
		}
		unsigned short sr1 = ((instruction >> 6) & 0x0007);
		unsigned short dr = ((instruction >> 9) & 0x0007);
		printf("AND R%d, R%d, %d\n", dr, sr1, imm5);
	}
	else{
		//register based
		unsigned short sr2 = instruction & 0x0007;
		unsigned short sr1 = (instruction >> 6) & 0x0007;
		unsigned short dr = (instruction >> 9) & 0x0007;
		printf("AND R%d, R%d, R%d\n", dr, sr1, sr2);

	}
}

void lc3_disassemble_ldr(unsigned short instruction){
	//calculate offset6, base and destination register
	unsigned short dr = (instruction >> 9) & 0x0007;
	unsigned short baseR = (instruction >> 6) & 0x0007;
	short PCoffset6 = (instruction & 0x003F);//isolate PCoffset6
	//sign extend PCoffset6
	if((PCoffset6 >> 5) & 1){
		PCoffset6 |= 0xFFC0;
	}
	printf("LDR R%d, R%d, %d\n", dr, baseR, PCoffset6);
}

void lc3_disassemble_str(unsigned short instruction){
	//calculate offset6, baseR, and sr
	short PCoffset6 = (instruction & 0x003F);//isolate PCoffset6
	//sign extend PCoffset6
	if((PCoffset6 >> 5) & 1){
		PCoffset6 |= 0xFFC0;
	}
	unsigned short baseR = (instruction >> 6) & 0x0007;
	unsigned short sr = (instruction >> 9) & 0x0007;
	printf("STR R%d, R%d, %d\n", sr, baseR, PCoffset6);
}

void lc3_disassemble_junk(unsigned short instruction){
	printf("%4x is not a real instruction", instruction);
}

void lc3_disassemble_not(unsigned short instruction){
	//get the source and destination register
	unsigned short sr = (instruction >> 6) & 0x0007;
	unsigned short dr = (instruction >> 9) & 0x0007;
	printf("NOT R%d, R%d\n", dr, sr);	
}
void lc3_disassemble_ldi(unsigned short instruction){
	//get the destination register
	unsigned short dr = (instruction >> 9) & 0x0007;
	short PCoffset9 = (instruction & 0x01FF);//isolate PCoffset9
	//sign extend PCoffset9
	if((PCoffset9 >> 8) & 1){
		PCoffset9 |= 0xFE00;
	}
	printf("LDI R%d, %d\n", dr, PCoffset9);
}

void lc3_disassemble_sti(unsigned short instruction){
	//calculate PCoffset9 and source register
	short PCoffset9 = (instruction & 0x01FF);//isolate PCoffset9
	//sign extend PCoffset9
	if((PCoffset9 >> 8) & 1){
		PCoffset9 |= 0xFE00;
	}
	unsigned short sr = (instruction >> 9) & 0x0007;
	printf("STI R%d, %d\n", sr, PCoffset9);
}

void lc3_disassemble_jmp(unsigned short instruction){
	//get the base register
	unsigned short baseR = (instruction >> 6) & 0x0007;
	printf("JMP R%d\n", baseR);
}

void lc3_disassemble_lea(unsigned short instruction){
	//calculate offset and destination register
	short PCoffset9 = (instruction & 0x01FF);//isolate PCoffset9
	//sign extend PCoffset9
	if((PCoffset9 >> 8) & 1){
		PCoffset9 |= 0xFE00;
	}
	unsigned short dr = (instruction >> 9) & 0x0007;
	printf("LEA R%d, %d\n", dr, PCoffset9);
}

void lc3_disassemble_trap(unsigned short instruction){
	//pull the trapvector off of the instruction
	unsigned char vector8 = (unsigned char)(instruction & 0x00FF);
	printf("TRAP x%2x\n", vector8);
}
