#ifndef _INSTRUCTION_H_
#define _INSTRUCTION_H_

// Function pointer for "special" instructions
class cpu_core;
typedef void (*operation)(cpu_core *cpu);
void sysc_op(cpu_core *cpu);

// Information about each instruction in the cpu, including function pointer and a
// character description for debugging purposes.
typedef struct _instruction {
	char name[9];
	operation special_case; // if nonnull handled by some magical process.
	const bool register_write;

	const byte alu_operation;  // an integer which determines which operation the ALU should perform
	const byte alu_source;     // second arg source
	                           // 0: source from register.
	                           // 1: source from immediate.
	                           // 2: source from sum of immidate and register (addrcalc)

	const bool branch;
	const byte mem_write;
	const byte mem_read;
	const bool mem_to_register;
	const byte exe_cycles;
} instruction;

// opcode is the instruction's position in the array.
const instruction instructions[] =
  {{ "    nop", NULL,     false, 0, 0, false, 0, 0, false, 1 }     // 00
  ,{ "   addi", NULL,     true,  1, 1, false, 0, 0, false, 2 }     // 01
  ,{ "   beqz", NULL,     false, 0, 0, true,  0, 0, false, 1 }     // 02
  ,{ "    bge", NULL,     false, 0, 0, true,  0, 0, false, 1 }     // 03
  ,{ "    bne", NULL,     false, 0, 0, true,  0, 0, false, 1 }     // 04
  ,{ "     la", NULL,     true,  0, 1, false, 0, 0, false, 1 }     // 05
  ,{ "     lb", NULL,     true,  0, 2, false, 0, 1, true , 1 }     // 06
  ,{ "     li", NULL,     true,  0, 1, false, 0, 0, false, 1 }     // 07
  ,{ "   subi", NULL,     true,  2, 1, false, 0, 0, false, 2 }     // 08
  ,{ "    add", NULL,     true,  1, 0, false, 0, 0, false, 2 }     // 09
  ,{ "syscall", &sysc_op, false, 0, 0, false, 0, 0, false, 5 }     // 10
  ,{ "     sb", NULL,     false, 0, 2, false, 1, 0, false, 1 } 	   // 11
  ,{ "     ld", NULL,      true, 0, 2, false, 0, 4, true , 1 }     // 12
  ,{ "     sd", NULL,     false, 0, 2, false, 4, 0, false, 1 }     // 13
  ,{ "  multi", NULL,      true, 3, 1, false, 0, 0, false, 3 }     // 14
  ,{ "   mult", NULL,      true, 3, 0, false, 0, 0, false, 3 }     // 15  
}; 

#endif /* _INSTRUCTION_H_ */
