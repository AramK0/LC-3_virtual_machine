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

int parse_register(char *token){
    if(token[0] == 'R' && token[1] >= '0' && token[1] < '7'){
        return token[0] - '0';
    }
    return -1;
}
int parse_immediate(char *token){
    if(token[0] == '#'){
        return atoi(token + 1);

    }
    return -999;
}

int main(){

    char **tokenizer = malloc(sizeof(char *) * TOKEN_SIZE);
    char *token;
    char buffer[1024];
    int i = 0;
    uint16_t orig;
    uint16_t ADD;
    uint16_t HALT;
    uint16_t r0;
    uint16_t r1;

    char path[128];
    printf("Please enter the path to the assembly file you want to assemble: ");
    fgets(path, sizeof(path), stdin);
    path[strcspn(path, "\n")] = '\0';

    int index = 0;
    char *start;
    int begin;
    int end;
    FILE *file = fopen(path, "r");
    if(!file){
        fprintf(stderr, "Invalid filepath or does not exist\n");
        exit(EXIT_FAILURE);
    }
    FILE *f2 = fopen("assembled_file.obj", "wb");
    if(!f2){
        fprintf(stderr, "Couldnt create or write to assembled_file.obl\n");
        exit(EXIT_FAILURE);
    }

    size_t read = fread(buffer, 1, sizeof(buffer) - 1, file);
    buffer[read] = '\0';
   
    

     for(index = 0; index <= read ; index++){
        if(buffer[index] == 34){
            begin = index;
            begin++;
            break;
        }
     }
     index = index + 1;
     for(index; index <= read ; index++){
        if(buffer[index] == 34){
            end = index;
            end--;
            break;
        }
     }
 


     
     int f = 0, e = 0;
     int str_size = end - begin;
     char str3[str_size];


 
     // get the quoted str into buffer then swap the bits
     for(buffer[begin]; begin <= end; begin++){
        str3[f++] = buffer[begin];
        //str3[e++] = (str3[e++] << 8) | (str3[e++] >> 8); This is undefined behavouir dont modify a variable more than once in same expression

     }

     str3[f] = '\0';
          
   
    
    token = strtok(buffer, ",\t\n");
    for(int e = 0; token; e++){
        tokenizer[i++] = token;
        token = strtok(NULL, " \n");
        if(!tokenizer){
            fprintf(stderr, "pointer failed at tokenize block\n");
            exit(EXIT_FAILURE);
        }
        
    }

    tokenizer[i] = NULL;
  
   

    uint16_t bin[str_size];
   


    for(int g = 0; g <= str_size; g++){

        bin[g] = str3[g];
        bin[g] = (bin[g] << 8) | (bin[g] >> 8);

    }

    int cnt = 0;

 
    uint16_t instr;

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
             
        else if(strcmp(tokenizer[j], "ADD") == 0){
            cnt ++;
            uint16_t add = 0x1000;
            // parse destination register
            int dr = parse_register(tokenizer[j + 1]);
            if(dr >= 0){
                add |= (dr << 9);
            }
            
            int sr1 = parse_register(tokenizer[j + 2]);
            if(sr1 >= 0){
                add |= (sr1 << 6);
            }

            char *third_operand = tokenizer[j + 3];
            if(third_operand[0] == 'R'){
                int sr2 = parse_register(third_operand);
                add |= sr2;
            }
            else if(third_operand[0] == '#'){
                int imm5 = parse_immediate(third_operand);
                add |= (1 << 5); // set 5 bits to indicate immediate mode
                add |= (imm5 & 0x1F);
            }
            add = (add << 8) | (add >> 8);
            fwrite(&add, sizeof(uint16_t), 1, f2);

            
        }
        else if(strcmp(tokenizer[j], "AND") == 0){
            uint16_t and = 0x5000;

            int dr = parse_register(tokenizer[j+1]);
            if(dr >= 0){
                and |= (dr << 9);
            }

            int sr1 = parse_register(tokenizer[j + 2]);
            if(sr1 >= 0){
                and |= (sr1 << 6);
            }

            char *third_operand = tokenizer[j+3];
            if(third_operand[0] == 'R'){
                int sr2 = parse_register(third_operand);
                and |= sr2;
            }
            else if(third_operand[0] == '#'){
                int imm5 = parse_immediate(third_operand);
                and |= 1 << 5;
                and |= (imm5 & 0x1F);

            }
            


            //printf("%d\n", instr);
            and = (and << 8) | (and >> 8);
            fwrite(&and, sizeof(uint16_t), 1, f2);
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
           fwrite(&bin, 2, str_size + 1, f2);

            
        }
        
    }
    

    printf("Successfully assembled '%s'\n", path);


    fclose(file);
    fclose(f2);
    free(tokenizer);


   

    return 0;
}
