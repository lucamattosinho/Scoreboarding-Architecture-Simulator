#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memoria.h"
#include "tradutor.h"
#include "processor.h"


// Função para devolver uma string do opcode
char* opcodeString(int opcode) {
    switch (opcode) {
        case 0:
            return "add";
        case 1:
            return "addi";
        case 2:
            return "sub";
        case 3:
            return "subi";
        case 4:
            return "mul";
        case 5:
            return "div";
        case 6:
            return "and";
        case 7:
            return "or";
        case 8:
            return "not";
        case 9:
            return "blt";
        case 10:
            return "bgt";
        case 11:
            return "beq";
        case 12:
            return "bne";
        case 13:
            return "j";
        case 14:
            return "lw";
        case 15:
            return "sw";
        case 16:
            return "exit";
        default:
            return "unknown";
    }
}

// Função que devolve o opcode de uma string
int getCodigoOpcode(const char opcode[4]) {

    if (strcmp(opcode, "add") == 0) {
        return 0;
    } else if (strcmp(opcode, "addi") == 0) {
        return 1;
    } else if (strcmp(opcode, "sub") == 0) {
        return 2;
    } else if (strcmp(opcode, "subi") == 0) {
        return 3;
    } else if (strcmp(opcode, "mul") == 0) {
        return 4;
    } else if (strcmp(opcode, "div") == 0) {
        return 5;
    } else if (strcmp(opcode, "and") == 0) {
        return 6;
    } else if (strcmp(opcode, "or") == 0) {
        return 7;
    } else if (strcmp(opcode, "not") == 0) {
        return 8;
    } else if (strcmp(opcode, "blt") == 0) {
        return 9;
    } else if (strcmp(opcode, "bgt") == 0) {
        return 10;
    } else if (strcmp(opcode, "beq") == 0) {
        return 11;
    } else if (strcmp(opcode, "bne") == 0) {
        return 12;
    } else if (strcmp(opcode, "j") == 0) {
        return 13;
    } else if (strcmp(opcode, "lw") == 0) {
        return 14;
    } else if (strcmp(opcode, "sw") == 0) {
        return 15;
    } else if (strcmp(opcode, "exit") == 0) {
        return 16;
    } else {
        return -1;
    }
}

// Função que retorna uma string de uma instrução baseando-se
// no binário que a representa
char* instrucaoToString(int inst){
    
    // Extrai os 6 bits mais significativos (destino)
    int opcode = (inst >> 26) & 0x3F;
    static char result[256];
    result[0] = '\0';

    // Concatena o opcode correspondente
    strcat(result, opcodeString(opcode));
    // Adiciona espaço após o opcode
    strcat(result, " ");

    // Lógica para adicionar registradores e valores imediatos conforme o tipo de instrução
    if (opcode < 9) {
        if (opcode == 8) {
            int rd = (inst >> 11) & 0x1F;
            int rs = (inst >> 21) & 0x1F;
            sprintf(result + strlen(result), "r%d, r%d", rd, rs);
        } else if (opcode != 1 && opcode != 3) {
            int rd = (inst >> 11) & 0x1F;
            int rs = (inst >> 21) & 0x1F;
            int rt = (inst >> 16) & 0x1F;
            sprintf(result + strlen(result), "r%d, r%d, r%d", rd, rs, rt);
        } else {
            int rt = (inst >> 16) & 0x1F;
            int rs = (inst >> 21) & 0x1F;
            int imm = inst & 0xFFFF;
            if ((imm & 0x8000) != 0) {
                imm |= 0xFFFF0000; // Estende sinal para valores negativos
            }
            sprintf(result + strlen(result), "r%d, r%d, %d", rt, rs, imm);
        }
    } else if (opcode == 13) {
        int address = inst & 0xFFFF;
        sprintf(result + strlen(result), "%d", address);

    }
    else {

        if(opcode==14 || opcode==15){
            int rs = (inst >> 21) & 0x1F;
            int rt = (inst >> 16) & 0x1F;
            int imm = inst & 0xFFFF;
            if ((imm & 0x8000) != 0) {
                imm |= 0xFFFF0000; // Estende sinal para valores negativos
            }
            sprintf(result + strlen(result), "r%d, %d(r%d)", rt, imm, rs);
                
        }
        else if(opcode==16){
            sprintf(result + strlen(result), " ");

        }
        else{
            int rs = (inst >> 21) & 0x1F;
            int rt = (inst >> 16) & 0x1F;
            int imm = inst & 0x7FFF;
            int bit_15 = (inst >> 15) & 1;
            if (bit_15 != 0) {
                imm = -(imm); // Estende sinal para valores negativos
            }
            sprintf(result + strlen(result), "r%d, r%d, %d", rs, rt, imm);

        }
      
    }

    return result;
}

// Tradução de string para binário. As instruções
// do programa a ser lido pelo simulador passarão
// por essa função para entrar na memória.
int instrucaoParaBinario(char *buffer){
    char* token;
    int inst=0;
    token = strtok(buffer, " \r\n");
    int codOpcode = getCodigoOpcode(token);
    int rs, rd, rt, imm, address;
    inst = codOpcode << 26;

    if(codOpcode<9){
        if (codOpcode == 8){
            char* arg1,*arg2;
            arg1 = strtok(NULL, "r ,");
            arg2 = strtok(NULL, "r \r\n");
            rd=atoi(arg1);
            rs=atoi(arg2);
            rs = rs << 21;
            inst = inst | rs;
            rd = rd << 11;
            inst = inst | rd;
        }
        else if(codOpcode!=1 && codOpcode !=3){
            char* arg1,*arg2,*arg3;
            arg1 = strtok(NULL, "r ,");
            arg2 = strtok(NULL, "r ,");
            arg3 = strtok(NULL, "r \r\n");
            rd=atoi(arg1);
            rs=atoi(arg2);
            rt=atoi(arg3);
            rs = rs << 21;
            inst = inst | rs;
            rt = rt << 16;
            inst = inst | rt;
            rd = rd << 11;
            inst = inst | rd;
        }
        else{
            char* arg1,*arg2,*arg3;
            arg1 = strtok(NULL, "r ,");
            arg2 = strtok(NULL, "r ,");
            arg3 = strtok(NULL, ", \r\n");
            rt=atoi(arg1);
            rs=atoi(arg2);
            imm=atoi(arg3);
            rs = rs << 21;
            inst = inst | rs;
            rt = rt << 16;
            inst = inst | rt;
            if(imm != abs(imm)){
                int sinal = 1;
                sinal = sinal << 15;
                inst = inst | sinal;
            }	
            imm = abs(imm) << 0;
            inst = inst | imm;
        }
    }
    else if(codOpcode==16 || codOpcode==13){
        if(codOpcode == 13){
            char* arg1;
            arg1 = strtok(NULL, " \r\n");
            address = atoi(arg1);
            address = address << 0;
            inst = inst | address;
        }
    }
    else{
        char* arg1,*arg2,*arg3;
        if(codOpcode==14 || codOpcode==15){
            arg1 = strtok(NULL, "r ,");
            arg2 = strtok(NULL, " (");
            arg3 = strtok(NULL, "r )\r\n");
            rs=atoi(arg3);
            rt=atoi(arg1);
            imm=atoi(arg2);
        }
        else{
            arg1 = strtok(NULL, "r ,");
            arg2 = strtok(NULL, "r ,");
            arg3 = strtok(NULL, ", \r\n");
            rs=atoi(arg1);
            rt=atoi(arg2);
            imm=atoi(arg3);
        }

        rs = rs << 21;
        inst = inst | rs;
        rt = rt << 16;
        inst = inst | rt;
        if(imm != abs(imm)){
        	int sinal = 1;
        	sinal = sinal << 15;
        	inst = inst | sinal;
		}	
        imm = abs(imm) << 0;
        inst = inst | imm;
    }
 
    return inst;
}

void printBinario(int num) {
    int bits = sizeof(num) * 8;

    for (int i = bits - 1; i >= 26; i--) {
        int bit = (num >> i) & 1;
        printf("%u", bit);
    }
    printf("  ");

    for (int i = 25; i >= 21; i--) {
        int bit = (num >> i) & 1;
        printf("%u", bit);
    }
    printf("  ");

    for (int i = 20; i >= 16 ; i--) {
        int bit = (num >> i) & 1;
        printf("%u", bit);
    }
    printf("  ");

    for (int i = 15; i >= 11 ; i--) {
        int bit = (num >> i) & 1;
        printf("%u", bit);
    }
    printf("  ");

    for (int i = 10; i >= 0 ; i--) {
        int bit = (num >> i) & 1;
        printf("%u", bit);
    }

    printf("\n");
}
