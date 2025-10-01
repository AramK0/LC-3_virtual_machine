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

int index = 0;
char *start;
int begin;
int end;
    FILE *file = fopen("try.s", "rb");
    FILE *f2 = fopen("yes.obj", "wb");

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

    /* for(buffer[begin]; begin == 34; begin++){
        printf(" %c \n", buffer[begin]);
     } */

     for(buffer[begin]; begin <= end; begin++){
        str3[f++] = buffer[begin];
        str3[e++] = (str3[e++] << 8) | (str3[e++] >> 8);

     }

     str3[f] = '\0';
   
    
    token = strtok(buffer, ",\t\n");
    for(int e = 0; token; e++){
        tokenizer[i++] = token;
        token = strtok(NULL, " \n");
        
        
    }
   

    uint16_t bin[str_size];
   


    for(int g = 0; g <= str_size; g++){
        bin[g] = str3[g];
        bin[g] = (bin[g] << 8) | (bin[g] >> 8);

    }

       
 
/*for(int e = 0; e < i; e++){
    printf("%s\n", tokenizer[e]);
}*/

            int cnt = 0;

 
    uint16_t instr;

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
             
        else if(strcmp(tokenizer[j], "ADD") == 0){
            cnt ++;
            uint16_t add = 0;
            uint16_t dr;
            uint16_t sr1;
            uint16_t sr2;
            uint16_t mask;  
            //mask = (1 << 12);

            add |= (0b0001 << 12);

            if(strcmp(tokenizer[j + 1], "R0,") == 0){ 
                // mask = 0000000000000000
                mask = (1 << 3) - 1; // 000000000000111  we only mask three bits 
                mask = (mask << 9);// 000111000000000
                //mask = ~mask;// 111000111111111
                //add &= mask; // we clear bits 9-11
                add |= (0b000 << 9); // 0001 000 000 000 000 | 0001 111 000 000 000


            }
            else if(strcmp(tokenizer[j+1], "R1,") == 0){
                add &= ~(0b111 << 9);
                mask = (1 << 3) - 1;
                mask = mask << 9;
                add |= (0b001 << 9);
            }
            else if(strcmp(tokenizer[j+1], "R2,") == 0){
                add &= ~(0b111 << 9);
                add |= (0b010 << 9);
            }
            else if(strcmp(tokenizer[j+1], "R3,") == 0){
                add &= ~(0b111 << 9);
                add |= (0b011 << 9);
            }
            else if(strcmp(tokenizer[j+1], "R4,") == 0){
                add &= ~(0b111 << 9);
                add |= (0b100 << 9);

            }
            else if(strcmp(tokenizer[j+1], "R5,") == 0){
                add &= ~(0b111 << 9);
                add |= (0b101 << 9);

            }
            else if(strcmp(tokenizer[j+1], "R6,") == 0){
                add &= ~(0b111 << 9);
                add |= (0b110 << 9);

            }
            else if(strcmp(tokenizer[j+1], "R7,") == 0){
                add &= ~(0b111 << 9);
                add |= (0b111 << 9);

            }


            if(strcmp(tokenizer[j + 2], "R0,") == 0){
                add &= ~(0b111 << 6);
                add |= (0b000 << 6);
            }
            else if(strcmp(tokenizer[j+2], "R1,") == 0){
                add &= ~(0b111 << 6);
                add |= (0b001 << 6);

            }
            else if(strcmp(tokenizer[j+2], "R2,") == 0){
                add &= ~(0b111 << 6);
                add |= (0b010 << 6);

            }
            else if(strcmp(tokenizer[j+2], "R3,") == 0){
                add &= ~(0b111 << 6);
                add |= (0b011 << 6);

            }
            else if(strcmp(tokenizer[j+2], "R4,") == 0){
                add &= ~(0b111 << 6);
                add |= (0b100 << 6);

            }
            else if(strcmp(tokenizer[j+2], "R5,") == 0){
                add &= ~(0b111 << 6);
                add |= (0b101 << 6);
            }
            else if(strcmp(tokenizer[j+2], "R6,") == 0){
                add &= ~(0b111 << 6);
                add |= (0b110 << 6);
            }
            else if(strcmp(tokenizer[j+2], "R7,") == 0){
                add &= ~(0b111 << 6);
                add |= (0b111 << 6);
            }

            if(strcmp(tokenizer[j + 3], "R0") == 0){
                add &= ~(0b111 << 0);
                add |= (0b000 << 0);
            }
            else if(strcmp(tokenizer[j+3], "R1") == 0){
                add &= ~(0b111 << 0);
                add |= (0b001 << 0);

            }
            else if(strcmp(tokenizer[j+3], "R2") == 0){
                add &= ~(0b111 << 0);
                add |= (0b010 << 0);

            }
            else if(strcmp(tokenizer[j+3], "R3") == 0){
                add &= ~(0b111 << 0);
                add |= (0b011 << 0);

            }
            else if(strcmp(tokenizer[j+3], "R4") == 0){
                add &= ~(0b111 << 0);
                add |= (0b100 << 0);

            }
            else if(strcmp(tokenizer[j+3], "R5") == 0){
                add &= ~(0b111 << 0);
                add |= (0b101 << 0);

            }
            else if(strcmp(tokenizer[j+3], "R6") == 0){
                add &= ~(0b111 << 0);
                add |= (0b110 << 0);

            }
            else if(strcmp(tokenizer[j+3], "R7") == 0){
                add &= ~(0b111 << 0);
                add |= (0b111 << 0);

            }
            else if(strcmp(tokenizer[j+3], "#0") == 0){
                add &= ~(0b11111 << 0);
                add |= (1 << 5);
                add |= (0b00000 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#1") == 0){
                add &= ~(0b11111 << 0);
                add |= (1 << 5);
                add |= (0b00001 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#2") == 0){
                add &= ~(0b11111 << 0);
                add |= (1 << 5);
                add |= (0b00010 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#3") == 0){
                add &= ~(0b11111 << 0);
                add |= (1 << 5);
                add |= (0b00011 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#4") == 0){
                add &= ~(0b11111 << 0);
                add |= (1 << 5);
                add |= (0b00100 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#5") == 0){
                add &= ~(0b11111 << 0);
                add |= (1 << 5);
                add |= (0b00101 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#6") == 0){
                add &= ~(0b11111 << 0);
                add |= (1 << 5);
                add |= (0b00110 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#7") == 0){
                add &= ~(0b11111 << 0);
                add |= (1 << 5);
                add |= (0b00111 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#8") == 0){
                add &= ~(0b11111 << 0);
                add |= (1 << 5);
                add |= (0b01000 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#9") == 0){
                add &= ~(0b11111 << 0);
                add |= (1 << 5);
                add |= (0b01001 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#9") == 0){
                add |= (1 << 5);
                add |= (0b01001 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#10") == 0){
                add |= (1 << 5);
                add |= (0b01010 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#11") == 0){
                add |= (1 << 5);
                add |= (0b01011 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#12") == 0){
                add |= (1 << 5);
                add |= (0b01100 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#13") == 0){
                add |= (1 << 5);
                add |= (0b01101 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#14") == 0){
                add |= (1 << 5);
                add |= (0b01110 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#15") == 0){
                add |= (1 << 5);
                add |= (0b01111 << 0);
            }
           
            
            //printf("%d\n", instr);
        
            add = (add << 8) | (add >> 8);
            fwrite(&add, sizeof(uint16_t), 1, f2);

            
        }
        else if(strcmp(tokenizer[j], "AND") == 0){
            uint16_t and = 0;
            uint16_t dr;
            uint16_t sr1;
            uint16_t sr2;
            uint16_t mask;  
            //mask = (1 << 12);

            and = (0b0101 << 12);

         printf("%s\n", tokenizer[j + 3]);

            if(strcmp(tokenizer[j + 1], "R0,") == 0){ 
                // mask = 0000000000000000
                mask = (1 << 3) - 1; // 000000000000111  we only mask three bits 
                mask = (mask << 9);// 000111000000000
                //mask = ~mask;// 111000111111111
                //add &= mask; // we clear bits 9-11
                and |= (0b000 << 9); // 0001 000 000 000 000 | 0001 111 000 000 000


            }
            else if(strcmp(tokenizer[j+1], "R1,") == 0){
                mask = (1 << 3) - 1;
                mask = mask << 9;
                and |= (0b001 << 9);
            }
            else if(strcmp(tokenizer[j+1], "R2,") == 0){
                and |= (0b010 << 9);
            }
            else if(strcmp(tokenizer[j+1], "R3,") == 0){
                and |= (0b011 << 9);
            }
            else if(strcmp(tokenizer[j+1], "R4,") == 0){
                and |= (0b100 << 9);

            }
            else if(strcmp(tokenizer[j+1], "R5,") == 0){
                and |= (0b101 << 9);

            }
            else if(strcmp(tokenizer[j+1], "R6,") == 0){
                and |= (0b110 << 9);

            }
            else if(strcmp(tokenizer[j+1], "R7,") == 0){
                and |= (0b111 << 9);

            }


            if(strcmp(tokenizer[j + 2], "R0,") == 0){
                and |= (0b000 << 6);
            }
            else if(strcmp(tokenizer[j+2], "R1,") == 0){
                and |= (0b001 << 6);

            }
            else if(strcmp(tokenizer[j+2], "R2,") == 0){
                and |= (0b010 << 6);

            }
            else if(strcmp(tokenizer[j+2], "R3,") == 0){
                and |= (0b011 << 6);

            }
            else if(strcmp(tokenizer[j+2], "R4,") == 0){
                and |= (0b100 << 6);

            }
            else if(strcmp(tokenizer[j+2], "R5,") == 0){
                and |= (0b101 << 6);
            }
            else if(strcmp(tokenizer[j+2], "R6,") == 0){
                and |= (0b110 << 6);
            }
            else if(strcmp(tokenizer[j+2], "R7,") == 0){
                and |= (0b111 << 6);
            }

            if(strcmp(tokenizer[j + 3], "R0") == 0){
                and |= (0b000 << 0);
            }
            else if(strcmp(tokenizer[j+3], "R1") == 0){
                and |= (0b001 << 0);

            }
            else if(strcmp(tokenizer[j+3], "R2") == 0){
                and |= (0b010 << 0);

            }
            else if(strcmp(tokenizer[j+3], "R3") == 0){
                and |= (0b011 << 0);

            }
            else if(strcmp(tokenizer[j+3], "R4") == 0){
                and |= (0b100 << 0);

            }
            else if(strcmp(tokenizer[j+3], "R5") == 0){
                and |= (0b101 << 0);

            }
            else if(strcmp(tokenizer[j+3], "R6") == 0){
                and |= (0b110 << 0);

            }
            else if(strcmp(tokenizer[j+3], "R7") == 0){
                and |= (0b111 << 0);

            }
            else if(strcmp(tokenizer[j+3], "#0") == 0){
                and &= ~(0b11111 << 0);
                and |= (1 << 5);
                and |= (0b00000 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#1") == 0){
                and &= ~(0b11111 << 0);
                and |= (1 << 5);
                and |= (0b00001 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#2") == 0){
                and &= ~(0b11111 << 0);
                and |= (1 << 5);
                and |= (0b00010 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#3") == 0){
                and &= ~(0b11111 << 0);
                and |= (1 << 5);
                and |= (0b00011 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#4") == 0){
                and |= (1 << 5);
                and |= (0b00100 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#5") == 0){
                and |= (1 << 5);
                and |= (0b00101 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#6") == 0){
                and |= (1 << 5);
                and |= (0b00110 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#7") == 0){
                and |= (1 << 5);
                and |= (0b00111 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#8") == 0){
                and |= (1 << 5);
                and |= (0b01000 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#9") == 0){
                and |= (1 << 5);
                and |= (0b01001 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#10") == 0){
                and |= (1 << 5);
                and |= (0b01010 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#11") == 0){
                and |= (1 << 5);
                and |= (0b01011 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#12") == 0){
                and |= (1 << 5);
                and |= (0b01100 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#13") == 0){
                and |= (1 << 5);
                and |= (0b01101 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#14") == 0){
                and |= (1 << 5);
                and |= (0b01110 << 0);
            }
            else if(strcmp(tokenizer[j+3], "#15") == 0){
                and |= (1 << 5);
                and |= (0b01111 << 0);
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
            

            

            

            

        }
        
    }
    
    
    //fwrite(&bin, 2, str_size + 1, f2);

    

   




    fclose(file);
    fclose(f2);
    free(tokenizer);


   

    return 0;
}
