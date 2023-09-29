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

void escreverRegistrador(int indice, int valor){
    bancoRegs[indice] = valor;
}

int lerRegistrador(int indice){
    return bancoRegs[indice];
}

void armazenarValorRegistrador(const char *linha, int tamanho) {
    char *separador = strstr(linha, "=");
    if (separador != NULL) {
        // Extrai o número do registrador (X) da parte esquerda da linha.
        int registrador_numero;
        sscanf(linha, "r%d =", &registrador_numero);

        // Verifica se o número do registrador é válido (1 a tamanho).
        if (registrador_numero >= 1 && registrador_numero < tamanho) {
            // Extrai o valor (Y) da parte direita da linha.
            int valor;
            sscanf(separador + 1, "%d", &valor);

            // Armazena o valor no registrador correspondente.
            bancoRegs[registrador_numero] = valor;
        }
    }
}