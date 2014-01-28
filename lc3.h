// DO NOT CHANGE ANY PROTOTYPE NAMES
// OR ARGUMENTS PASSED IN TO ANY PROTOTYPE HERE.

// YOU MAY ADD ADDITIONAL FUNCTIONS BUT YOU MUST IMPLEMENT THE ONES BELOW
#ifndef LC3_H
#define LC3_H
#include <stdio.h>

#define DEBUG 0
/* Represents the state of the condition code register */
enum lc3_cc
{
	LC3_NEGATIVE = 4,
	LC3_ZERO = 2,
	LC3_POSITIVE = 1,
	INVALID = 0 /* Should never be set to this */
};

/**
 * A structure that represents the state of the LC3 machine.
 */
typedef struct 
{
	short mem[65536]; /* Memory */
	short regs[8]; /* The eight registers in the LC-3 */
	unsigned short pc; /* The pc register */
	unsigned char cc; /* The condition code register the value will be one of the enum values above */
	int halted; /* 0 if the machine is running 1 if the machine is halted */
} lc3machine;

/* lc3_init
   This function initializes the lc3machine to its initial state
   The pc should be set to 3000 (hex)
   and the conditional code register should be set to indicate zero (LC3_ZERO)
*/
void lc3_init(lc3machine* state);

/* lc3_load
   This function is responsible for reading a .obj file
   produced from as2obj the format is explained in the
   assignment */
void lc3_load(lc3machine* state, const char* program);

/* lc3_step_one
   This function just does one simulation step  */
void lc3_step_one(lc3machine* state);

/* lc3_run
   This function will run for the number of steps given or until you halt.
   @param num_steps number of steps to run (-1 for infinite)
*/
void lc3_run(lc3machine* state, int num_steps);

/* lc3_fetch
   Fetches an instruction from memory BE SURE to do everything that is needed
   to be done in the fetch stage
   @return the fetch'd instruction. */
unsigned short lc3_fetch(lc3machine* state);

/* lc3_execute
   Execution function. in charge of executing the instruction given
   @param instruction instruction to execute
*/
void lc3_execute(lc3machine* state, unsigned short instruction);


/* lc3_disassemble
   Disassembling function. in charge of disassembling the instruction given
   @param instruction instruction to disassemble. 	
*/
void lc3_disassemble(unsigned short instruction);

/* lc3_trap
   Trap execution function called from within lc3_execute.
*/
void lc3_trap(lc3machine* state, unsigned char vector8);

/*	lc3_continue
	runs the machine until the program halts
*/
void lc3_continue(lc3machine* state);

/*	lc3_br
	changes the value of the pc if certain conditions are met
*/
void lc3_br(lc3machine* state, unsigned short br_instruction);

/*	lc3_add
	adds two pieces of data together
*/
void lc3_add(lc3machine* state, unsigned short add_instruction);

/*	lc3_ld
	loads data from the lc3's memory
*/
void lc3_ld(lc3machine* state, unsigned short ld_instruction);

/*	lc3_st
	stores data in the lc3's memory
*/
void lc3_st(lc3machine* state, unsigned short st_instruction);

/*	lc3_jsr
	jumps to another spot in memory, and stores the return address in R7
*/
void lc3_jsr(lc3machine* state, unsigned short jsr_instruction);

/*	lc3_and
	performs a bitwise and
*/
void lc3_and(lc3machine* state, unsigned short and_instruction);

/*	lc3_ldr
	loads data from memory based off of a value stored in a register
*/
void lc3_ldr(lc3machine* state, unsigned short ldr_instruction);

/*	lc3_str
	stores data in memory based of a value stored in a register
*/
void lc3_str(lc3machine* state, unsigned short str_instruction);

/*	lc3_not
	performs a bitwise not
*/
void lc3_not(lc3machine* state, unsigned short not_instruction);

/*	lc3_ldi
	loads data from memory based off of the instruction
*/
void lc3_ldi(lc3machine* state, unsigned short ldi_instruction);

/*	lc3_sti
	stores data in memory based off of the instruction
*/
void lc3_sti(lc3machine* state, unsigned short sti_instruction);

/*	lc3_jmp
	sets the pc to the value contained in a register
*/
void lc3_jmp(lc3machine* state, unsigned short jmp_instruction);

/*	lc3_lea
	loads the pc with an address
*/
void lc3_lea(lc3machine* state, unsigned short lea_instruction);

//junk method, here for efficiency reasons
void junk(lc3machine* junk1, unsigned short junk2);

/*	trap
	gets the trapvector from the instruction and performs the trap
*/
void trap(lc3machine* state, unsigned short trap_instruction);

/*	setcc
	sets the condtion code to the appropriate value\
*/
void setcc(lc3machine* state, unsigned short dr);

//disassembling functions
void lc3_disassemble_br(unsigned short instruction);
void lc3_disassemble_add(unsigned short instruction);
void lc3_disassemble_ld(unsigned short instruction);
void lc3_disassemble_st(unsigned short instruction);
void lc3_disassemble_jsr(unsigned short instruction);
void lc3_disassemble_and(unsigned short instruction);
void lc3_disassemble_ldr(unsigned short instruction);
void lc3_disassemble_str(unsigned short instruction);
void lc3_disassemble_junk(unsigned short instruction);
void lc3_disassemble_not(unsigned short instruction);
void lc3_disassemble_ldi(unsigned short instruction);
void lc3_disassemble_sti(unsigned short instruction);
void lc3_disassemble_jmp(unsigned short instruction);
void lc3_disassemble_lea(unsigned short instruction);
void lc3_disassemble_trap(unsigned short instruction);
#endif //ifndef LC3_H
