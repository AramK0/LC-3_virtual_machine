#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


// we define our memory size based on the lc-3 model , this can store a total of 128KB, 65,536 memory locations (2^16)
// each location stored 16 bits = 2 bytes 
#define MEMORY_MAX (1 << 16)
// we will story this memory in an unsigned 16 bit integer array , because memory addresses are naturally unsigned there are no negative mem addresses
uint16_t memory[MEMORY_MAX];


// The LC-3 has 10 total registers , each of which is 16 bits, 8 general-purpose registers (R0-R7) to perform any program calculation
// R_PC Program counter register , an unsigned integer which is the address of the next instruction in memeory to execute
// Condition flag register tells us information about previouse calculation

enum{
    R_R0 = 0,
    R_R1,
    R_R2,
    R_R3,
    R_R4,
    R_R5,
    R_R6,
    R_R7,
    R_PC, /* program counter */
    R_COND,
    R_COUNT
};


// we will store our registers in an array just like the memory
uint16_t reg[R_COUNT]; // reg[10]

// Condition flags , they provide information about the most recently executed calculation, the lc-3 only has 3 condition flags
enum{
    FL_POS = 1 << 0, // P 001
    FL_ZRO = 1 << 1, // Z 010
    FL_NEG = 1 << 2, // N 100
};



// Instruction set, has Opcode: indicates the kind of task to perform and parameters which provide input to the task being performed
// Only 16 opcodes in the lc-3 architecture. each instruction is 16 bits with the left 4 bits storing the op code . The rest save the parameters

enum{
    OP_BR = 0, // Branch
    OP_ADD, //add  1 = 0001
    OP_LD, // load
    OP_ST, //store
    OP_JSR, // jump register
    OP_AND, //bitwise AND
    OP_LDR, // load register
    OP_STR, // store register
    OP_RTI, // unused
    OP_NOT, // bitwise NOT
    OP_LDI, // load indirect 1010 = 10
    OP_STI, // store indirect
    OP_JMP, // jump
    OP_RES, // reserved (unused)
    OP_LEA, // load effective addresss
    OP_TRAP // execute trap
};

// Trap routines

enum{
    TRAP_GETC = 0x20, 
    TRAP_OUT = 0x21,
    TRAP_PUTS = 0x22,
    TRAP_IN = 0x23,
    TRAP_PUTSP = 0x24,
    TRAP_HALT = 0x25
};

uint16_t sign_extend(uint16_t x, int bit_count){
    // we check if the LMB is 1 ( meaning if the number is negative) eg/ 5 bit num: x = 0b11010 -6 
    // bit_count = 5 -1 = 4
    // x >> 4 = 0b00001 and & 1 = 1 
    if((x >> (bit_count -1)) & 1){
        x |= (0xFFFF << bit_count); // 1111 1111 1111 1111 << 4 is : 1111 1111 1110 0000 x = 0000 0001 1010
        // result = 1111 1111 1111 1010 -6 in 16-bit
    }
    // for positive nums it alr gets filled with zeros 

    return x; 
}

void update_flag(uint16_t r){
    if(reg[r] == 0){
        reg[R_COND] = FL_ZRO;
    }
    else if(reg[r] >> 15){ // if the left most bit is 1 then it is negative
        reg[R_COND] = FL_NEG;

    }
    else{
        reg[R_COND] = FL_POS;
    }
}



int main(int argc, const char *argv[]){

    if(argc < 2){
        // show usage string
        printf("lc3 [image-file1]...\n");
        exit(2); // code 2 is misue of shell comms (invalid or missing args, etc.)
    }

    for(int j = 1; j < argc; ++j){
        if(!read_image(argv[j])){ // argv is an array of string pointers
            printf("Failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }


    // only one condtion flag should be up at a time so we set the Zro flag
    reg[R_COND] = FL_ZRO;

    enum{ PC_START = 0x3000}; // the memory location where the pc will start and have it to the counter register
    reg[R_PC] = PC_START;

    int running = 1;
    while(running){
        // step 1: fetch the instruction from memory at the address of the pc register
        uint16_t instr = mem_read(reg[PC_START]++); 
        // get the top 4 bits for the type of instr
        uint16_t op = instr >> 12;

        switch(op){
            case OP_ADD:
                // destination register DR (bits 11-9)
                // we get the INDEX to register 0
                uint16_t r0 = (instr >> 9) & 0x7; // result: r0 = 000
                // bits (8-6) we move to 2:0 and mask it out with 0x7 which is 111 in binary
                uint16_t r1 = (instr >> 6) & 0x7; // result: r1 = 001

                uint16_t imm_flag = (instr >> 5) & 0x1; // we check to see if bit 5 is 0 or 1

                if(imm_flag){
                    uint16_t last_five = (instr & 0x1F);
                    uint16_t imm5 = sign_extend(last_five, 5); // we mask only the last 5 bits
                    reg[r0] = reg[r1] + imm5;
                }
                else{
                    // bits 2-0
                    uint16_t r2 = (instr & 0x7);  // only mask out last three bits 
                    reg[r0] = reg[r1] + reg[r2];
                }
                
                update_flag(r0);


                break;
            case OP_AND:
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;
                
                if(imm_flag){
                    uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                    reg[r0] = reg[r1] * imm5;
                }
                else{
                    uint16_t r2 = (instr & 0x7);
                    reg[r0] = reg[r1] * reg[r2];
                }

                update_flag(r0);


                break;
            case OP_BR:
                // 
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);    // The signed offset to jump to if the condition is true 
                // we check bits 11-10-9 for the flag if 11 is set then 11:N 10:Z 9:P 
                // the instr decides which flag can trigger a branch 
                uint16_t cond_flag = (instr >> 9) & 0x7;
                if(cond_flag & reg[R_COND]){
                    // jump to that pcoffset if any condition was true 
                    reg[R_PC] += pc_offset;
                }


                break;
            case OP_JMP:

                uint16_t r1 = (instr >> 6) & 0x7;
                reg[R_PC] = reg[r1];    // Also does RET when the reg is r7 as r7 is used for return addresses so we return to where we came from


                break;
            case OP_JSR:
                
                // its basically two options of jumpting to a function(subroutine) address by either jumping to the register 
                // address of a function or jumpting to that function address directly
                reg[R_R7] = reg[R_PC]; // we save the return address in reg7 to return to after the subroutine(function)
                uint16_t bit_el = (instr >> 11) & 0x1;
                

                if(bit_el == 0){
                    uint16_t r1 = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1]; //JSRR jump to subroutine register 
                }
                else{
                    uint16_t pc_offset = (instr & 0x7FF);
                    reg[R_PC] += sign_extend(pc_offset, 11); // JSR Jump To Sobroutine 
                }

                break;
            case OP_LD: // when you know the exact location of the data
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend (instr & 0x1FF, 9);
                
                reg[r0] = mem_read(reg[R_PC] + pc_offset);

                update_flag(r0);


                break;
            case OP_LDI: // when working with pointers
                // desination register
                uint16_t r0 = (instr >> 9) & 0x7;
                // Program-Counter offset 9 pcoffset is a signed offset value that tells the cpu how far forward or backward to move in memory
                // the CPU takes the current program-counter PC, adds the PCoffset , and the result is the target memory address
                // Target address = PC + PCoffset
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9); // mask out first 9-bits: see LDI instr

                // add pc_offset to the current PC, look at mem location to get the final address
                reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
                update_flag(r0);

                break;
            case OP_LDR: // LDR R4, R2, #−5 ; R4 ← mem[R2 − 5]

                uint16_t DR = (instr >> 9) & 0x7;
                uint16_t base_r = (instr >> 6) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x3F, 6);

                reg[DR] = mem_read(reg[base_r] + pc_offset);

                update_flag(DR);

                break;
            case OP_LEA: // The Load-Effective-Address does NOT read memory to obtain the information to load into DR.
                        // the address itself is loaded into DR 
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

                reg[r0] = reg[R_PC] + pc_offset;
                
                update_flag(r0);

                break;
            case OP_NOT:
                    
                uint16_t r0 = (instr >> 9) & 0x7;
                uint16_t r1 = (instr >> 6) & 0x7;

                reg[r0] = ~reg[r1];

                update_flag(r0);

                break;
            case OP_ST:
                uint16_t SR = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

                mem_write(reg[R_PC] + pc_offset, reg[SR]);
            
                break;
            case OP_STI:
                uint16_t SR = (instr >> 9) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                mem_write(mem_read(reg[R_PC] + pc_offset), reg[SR]);

                break;
            case OP_STR:
                uint16_t SR = (instr >> 9) & 0x7;
                uint16_t BaseR = (instr >> 6) & 0x7;
                uint16_t pc_offset = sign_extend(instr & 0x3F, 6);
                
                mem_write(reg[BaseR] + pc_offset, reg[SR]);

                break;
            case OP_TRAP:
                reg[R_R7] = reg[R_PC];
                switch(instr & 0xFF){
                    case TRAP_GETC:
                    
                        break;
                    case TRAP_OUT:
                        break;
                    case TRAP_PUTS:
                        char *c = memory + reg[R_R0]; // we give it the string address first in memory
                        while(*c){
                            putc((char)*c, stdout); // read singl character , cast to 8-bit first 
                            ++c; // move the pointer to the next address
                        }
                        fflush(stdout);

                        break;
                    case TRAP_IN:
                        break;
                    case TRAP_PUTSP:
                        break;
                    case TRAP_HALT:
                        break;
                }    


                break;
            case OP_RTI:
            case OP_RES:
            default:
                // bad opcode
                break;
            
        }

    }





    return 0;
}