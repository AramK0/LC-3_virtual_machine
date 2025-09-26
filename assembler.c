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

#define TOKEN_SIZE 1024


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




int main(){

    char **tokenizer = malloc(sizeof(char *) * TOKEN_SIZE);
    char *token;
    char buffer[1024];
    int i = 0;
    uint16_t orig;
    uint16_t ADD;
    uint16_t HALT;
    

    FILE *file = fopen("test.s", "r");

    size_t read = fread(buffer, 1, sizeof(buffer) - 1, file);
    buffer[read] = '\0';

    token = strtok(buffer, ",\t\n");
    while(token){
        tokenizer[i++] = token;
        token = strtok(NULL, " ,\t\n");
    }
    
    printf("%s\n", tokenizer[0]);
    printf("%s\n", tokenizer[1]);
    printf("%s\n", tokenizer[2]);
    printf("%s\n", tokenizer[3]);
    printf("%s\n", tokenizer[4]);
    printf("%s\n", tokenizer[5]);


    tokenizer[i] = NULL;
  
    if(strcmp(tokenizer[0], ".ORIG x3000") == 0){
        orig = 0x3000;
        orig = (orig << 8) | (orig >> 8);
    }
    if(strcmp(tokenizer[1], "ADD") == 0){
        
        ADD = 0x1007;
        ADD = (ADD << 8) | (ADD >> 8);
    }
    if(strcmp(tokenizer[5], "HALT") == 0){
        
        HALT = 0xF025;
        HALT = (HALT << 8) | (HALT >> 8);
    }


    

    FILE *f2 = fopen("yes.obj", "wb");
    fwrite(&orig, sizeof(uint16_t), 1, f2);
    fwrite(&ADD, sizeof(uint16_t), 1, f2);
    fwrite(&HALT, sizeof(uint16_t), 1, f2);





    fclose(file);
    free(tokenizer);


   

    return 0;
}