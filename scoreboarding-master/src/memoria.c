#include <stdio.h>
#include <stdlib.h>
#include "memoria.h"
#include "tradutor.h"
#include "registradores.h"
#include "barramento.h"

int *memoria;
int tam;
int qtdeInsts;



void inicializaMemoria(int m){
    if(m<=399){
        printf("Tamanho de memória insuficiente. O tamanho da memória deve ser mair do que 100. \n");
    }
    else{
        memoria = (int*)malloc(sizeof(int) * m );
    }
    tam = m;
}


void printMemoria(){
    printf("Conteúdo da memória:\n");
    for (int i = 0; i < tam; i += 4) {
        int palavra = 0;

        // Combina os quatro elementos do vetor memoria[i] até memoria[i+3] para construir a palavra de 32 bits
        for (int j = 0; j < 4; j++) {
            palavra = (palavra << 8) | memoria[i + j];
        }

        // Exibe a palavra de 32 bits
        printf("Endereço[%d]: %d\n", i / 4, palavra);
    }
}

int pegaMemoria(int indice){
    int palavra;
    for(int i = indice; i<indice+4; i++){
        palavra = (palavra << 8) | memoria[indice + i];
    }
    barramento = palavra;
    int valor = pegaBarramento();
    return valor;
}

int pegaMemoriaLw(int fonte1, int fonte2){
    int palavra;
    for(int i = 0; i<4; i++){
        palavra = (palavra << 8) | memoria[fonte1 + fonte2 + i];
    }
    barramento = palavra;
    int valor = pegaBarramento();
    return valor;
}

void colocaMemoria(int instrucao, int indice){
    if(memoria==NULL){
        printf("Erro.");
    }
    else{
        memoria[indice]= (instrucao >> 24) & 0xFF;
        indice++;
        memoria[indice]= (instrucao >> 16) & 0xFF;
        indice++;
        memoria[indice]= (instrucao >> 8) & 0xFF;
        indice++;
        memoria[indice]= instrucao & 0xFF;
        indice++;
    }
}

void insereMemoria(int instrucao){
    if(memoria==NULL){
        printf("Erro.");
    }
    else{
        memoria[pc]= (instrucao >> 24) & 0xFF;
        pc++;
        memoria[pc]= (instrucao >> 16) & 0xFF;
        pc++;
        memoria[pc]= (instrucao >> 8) & 0xFF;
        pc++;
        memoria[pc]= instrucao & 0xFF;
        pc++;
    }
}
