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



int main(){

    char **tokenizer = malloc(sizeof(char *) * TOKEN_SIZE);
    char **string = malloc(TOKEN_SIZE * sizeof(char *));
    char *token;
    char buffer[1024];
    int i = 0;
    uint16_t orig;
    uint16_t ADD;
    uint16_t HALT;
    uint16_t r0;
    uint16_t r1;
    


    FILE *file = fopen("test.s", "r");
    FILE *f2 = fopen("yes.obj", "wb");

    size_t read = fread(buffer, 1, sizeof(buffer) - 1, file);
    buffer[read] = '\0';

    token = strtok(buffer, ",\t\n");
    for(int e = 0; token; e++){
        tokenizer[i++] = token;
        token = strtok(NULL, " ,\t\n");
        if(strcmp(tokenizer[e], ".STRINGZ") == 0){
            token = strtok(NULL, "\n");
            string[0] = token;
        }
        
    }
    int size = sizeof(string[0]);
    char text[size];

    

    strcpy(text, string[0]);

   printf("%s\n", text);
 
    

    tokenizer[i] = NULL;
    for(int j = 0; j < i; j++){
        if(strcmp(tokenizer[j], ".ORIG x3000") == 0){
        orig = 0x3000;
        orig = (orig << 8) | (orig >> 8);
        fwrite(&orig, sizeof(uint16_t), 1, f2);

        }
        else if(strcmp(tokenizer[j], "LEA") == 0){
            uint16_t LOAD_E = 0xE002;
            //uint16_t pc_offset = malloc(sizeof(uint16_t));
            

            
            LOAD_E = (LOAD_E << 8) | (LOAD_E >> 8);
            fwrite(&LOAD_E, sizeof(uint16_t), 1, f2);

            


        }
        else if(strcmp(tokenizer[j], ".STRINGZ") == 0){
            //printf("%s\n", tokenizer[j+1]);
        }
        else if(strcmp(tokenizer[j], "ADD") == 0){
            ADD = 0x1007;
            ADD = (ADD << 8) | (ADD >> 8);
            fwrite(&ADD, sizeof(uint16_t), 1, f2);
        }
        else if(strcmp(tokenizer[j], "PUTS") == 0){
            uint16_t PUTS = 0xF022;
            PUTS = (PUTS << 8) | (PUTS >> 8);
            fwrite(&PUTS, sizeof(uint16_t), 1, f2);
            
        }
        else if(strcmp(tokenizer[j], "HALT") == 0){
            
            HALT = 0xF025;
            HALT = (HALT << 8) | (HALT >> 8);
            fwrite(&HALT, sizeof(uint16_t), 1, f2);

            fwrite(&text, sizeof(text), 1, f2);
            fwrite(&text, sizeof(text), 1, f2);
            

        }
        
    }
    
    


    

   




    fclose(file);
    fclose(f2);
    free(tokenizer);


   

    return 0;
}