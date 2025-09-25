#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>


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

// memory mapped registers, we can read or write to them by reading or writing to their memory locations
enum{
    MR_KBSR = 0xFE00,
    MR_KBDR = 0xFE02
};


struct termios original_tio;

void disable_input_buffering()
{
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

uint16_t check_key()
{
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    return select(1, &readfds, NULL, NULL, &timeout) != 0;
}


uint16_t swap16(uint16_t x){
    return (x << 8) | (x >> 8);
}

// we give it a file containing an array of instructions and data that was created from converting assembly into machine code
// we load this by copying the files contents into an address in memory 
void read_image_file(FILE *file){
    // The origin:first 16-bits tells us when in memory to place the image
    uint16_t origin;    
    fread(&origin, sizeof(origin), 1, file); // fread transfers data from an already opened file into the program memory
    origin = swap16(origin);

    // we know maximum file size so we only need one fread
    uint16_t max_read = MEMORY_MAX - origin; // 65,536 - 12,288 = 53,248 we can read 53,248 16-bit words from the file before we hit the end
    // of available memory
    // this pointer points to the start of memory where we want to start loading the programs
    uint16_t *p = memory + origin; // memory[0] + 12,288 = memory[12,288] meanning 0x300
    // reads up to 53,248(max_read) 16-bit words from the binary file
    // and loads them into memory starting at address 0x3000(*p)
    // read tells us how many words were actually read 
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    // swap to little indian
    while(read-- > 0){
        *p = swap16(*p);
        ++p;
    }

}



int read_image(const char *image_path){
    FILE *file = fopen(image_path, "rb"); // r: read b: open the file in binary mode
    if(!file){
        fprintf(stderr, "File doesnt exit\n");
        return 0;
    }
    read_image_file(file);
    fclose(file);
    return 1;
}

void mem_write(uint16_t address, uint16_t val){
    memory[address] = val;
}

uint16_t mem_read(uint16_t address){
    if(address == MR_KBSR){ // checks if we are asking the keyboard for data
        if(check_key()){ // is there a key pressed
            memory[MR_KBSR] = (1 << 15); // The MSB is used as a status flag 
            memory[MR_KBDR] = getchar(); // store the pressed key in kbdr
        }
        else{
            memory[MR_KBSR] = 0;
        }
    }
    return memory[address];
}

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

void handle_interrupt(int signal)
{
    restore_input_buffering();
    printf("\n");
    exit(-2);
}

int main(){

    printf("**********************************\n");
    printf("Welcome to LC-3 VM.\n");

    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

/*    if(argc < 2){
        // show usage string
        printf("lc3 [image-file1]...\n");
        exit(2); // code 2 is misue of shell comms (invalid or missing args, etc.)
    }

    for(int j = 1; j < argc; ++j){
        if(!read_image(argv[j])){ // argv is an array of string pointers
            printf("Failed to load image: %s\n", argv[j]);
            exit(1);
        }
    } */

    
    int running = 1;
    int choice = 0;
    char path[1024];

    reg[R_COND] = FL_ZRO;

    enum{ PC_START = 0x3000}; // the memory location where the pc will start and have it to the counter register
    reg[R_PC] = PC_START;

    while(1){
        printf("LC-3->Please choose one of the following arguements: 1-Give an assembly file 2-quit: ");
        restore_input_buffering();
        scanf("%d", &choice);
        getchar();

        if(choice == 2){
            printf("LC-3->Goodbye.\n");
            return 0;
        }
        else if(choice == 1){

            printf("Enter the path to the assembly file you wish to run: ");
            fflush(stdout);
            fgets(path, sizeof(path), stdin);
            path[strcspn(path, "\n")] = '\0';
            fflush(stdout);
            char *p = path;
            
            fflush(stdout);
            
            
            int running = 1;
            while(running){

                if(read_image(p) == 0){
                running = 0;
            }
                
                // step 1: fetch the instruction from memory at the address of the pc register
                uint16_t instr = mem_read(reg[R_PC]++); 
                // get the top 4 bits for the type of instr
                uint16_t op = instr >> 12;

                switch(op){
                    case OP_ADD:{
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
                    }
                    case OP_AND:{
                        uint16_t r0 = (instr >> 9) & 0x7;
                        uint16_t r1 = (instr >> 6) & 0x7;
                        
                        uint16_t imm_flag = (instr >> 5) & 0x1;

                        if(imm_flag){
                            uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                            reg[r0] = reg[r1] & imm5;
                        }
                        else{
                            uint16_t r2 = (instr & 0x7);
                            reg[r0] = reg[r1] & reg[r2];
                        }

                        update_flag(r0);


                        break;
                    }
                    case OP_BR:{
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
                    }
                    case OP_JMP:{

                        uint16_t r1 = (instr >> 6) & 0x7;
                        reg[R_PC] = reg[r1];    // Also does RET when the reg is r7 as r7 is used for return addresses so we return to where we came from


                        break;
                    }
                    case OP_JSR:{
                        
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
                    }
                    case OP_LD:{ // when you know the exact location of the data
                        uint16_t r0 = (instr >> 9) & 0x7;
                        uint16_t pc_offset = sign_extend (instr & 0x1FF, 9);
                        
                        reg[r0] = mem_read(reg[R_PC] + pc_offset);

                        update_flag(r0);


                        break;
                    }
                    case OP_LDI:{ // when working with pointers
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
                    }
                    case OP_LDR:{ // LDR R4, R2, #−5 ; R4 ← mem[R2 − 5]

                        uint16_t DR = (instr >> 9) & 0x7;
                        uint16_t base_r = (instr >> 6) & 0x7;
                        uint16_t pc_offset = sign_extend(instr & 0x3F, 6);

                        reg[DR] = mem_read(reg[base_r] + pc_offset);

                        update_flag(DR);

                        break;
                    }
                    case OP_LEA:{ // The Load-Effective-Address does NOT read memory to obtain the information to load into DR.
                                // the address itself is loaded into DR 
                        uint16_t r0 = (instr >> 9) & 0x7;
                        uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

                        reg[r0] = reg[R_PC] + pc_offset;
                        
                        update_flag(r0);

                        break;
                    }
                    case OP_NOT:{
                            
                        uint16_t r0 = (instr >> 9) & 0x7;
                        uint16_t r1 = (instr >> 6) & 0x7;

                        reg[r0] = ~reg[r1];

                        update_flag(r0);

                        break;
                    }
                    case OP_ST:{
                        uint16_t SR = (instr >> 9) & 0x7;
                        uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);

                        mem_write(reg[R_PC] + pc_offset, reg[SR]);
                    
                        break;
                    }
                    case OP_STI:{
                        uint16_t SR = (instr >> 9) & 0x7;
                        uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                        mem_write(mem_read(reg[R_PC] + pc_offset), reg[SR]);

                        break;
                    }
                    case OP_STR:{
                        uint16_t SR = (instr >> 9) & 0x7;
                        uint16_t BaseR = (instr >> 6) & 0x7;
                        uint16_t pc_offset = sign_extend(instr & 0x3F, 6);
                        
                        mem_write(reg[BaseR] + pc_offset, reg[SR]);

                        break;
                    }
                    case OP_TRAP:
                        reg[R_R7] = reg[R_PC]; // save the return address in R7

                        switch(instr & 0xFF){
                            case TRAP_GETC:{
                                reg[R_R0] = (uint16_t)getchar(); // getchar returns a 32-bit integer value so we make it 16-bit as our registers are
                                update_flag(R_R0);

                                break;
                            }
                            case TRAP_OUT:{
                                char ch = reg[R_R0];
                                putc(ch, stdout);
                            
                                break;
                            }
                            case TRAP_PUTS:{
                                uint16_t *c = memory + reg[R_R0]; // we give it the string address first in memory
                                while(*c){
                                    putc((char)*c, stdout); // read singl character , cast to 8-bit first 
                                    ++c; // move the pointer to the next address
                                }
                                fflush(stdout);

                                break;
                            }
                            case TRAP_IN:{
                                printf("Enter a single character: ");
                                char c = getchar(); // 32-bit val
                                putc(c, stdout);
                                fflush(stdout);
                                reg[R_R0] = (uint16_t)c; 
                                update_flag(R_R0);
                                break;
                            }
                            case TRAP_PUTSP:{
                                uint16_t *c = memory + reg[R_R0];
                                while(*c){
                                    char char1 = (*c) & 0xFF;
                                    putc(char1, stdout);
                                    char char2 = (*c >> 8);
                                    putc (char2, stdout);
                                    if(char2 == 1) putc(char2, stdout);
                                    ++c;
                                }
                                fflush(stdout);

                                break;
                            }
                            case TRAP_HALT:
                                puts("HALT there!");
                                fflush(stdout);
                                running = 0;
                        }    


                        break;
                    case OP_RTI:
                    case OP_RES:
                    default:
                        
                        break;
                    
                }

            }

                }
            }

            
    restore_input_buffering();

    return 0;
}