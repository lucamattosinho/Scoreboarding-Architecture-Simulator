#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int bancoRegs[32];
int pc=0;
int pc2=0;
int ir;

void printRegistradores(){
    printf("\n\nREGISTRADORES:\n");
	for(int j=0; j<32; j++){
        if(bancoRegs[j]<0){
            printf("r%d = %d\t", j, bancoRegs[j]);
        }
        else{
		    printf("r%d = %d\t\t", j, bancoRegs[j]);
        }
        if(j==7 || j==15 || j==23){
            printf("\n");
        }
	}
    printf("\n");
}
