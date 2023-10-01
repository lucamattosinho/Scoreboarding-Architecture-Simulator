#include <stdio.h>
#include <stdlib.h>
#include "barramento.h"
#include "memoria.h"
#include "registradores.h"

int barramento;
int largura_escrita;
int *barramentoResultados;

void inicializaBarramentoResultados(int largura){
    if(largura<1){
        printf("Largura de escrita deve ser maior do que 0. \n");
    }
    else{
        barramentoResultados = (int*)malloc(sizeof(int)*largura);
        largura_escrita=largura;
    }
}

void limpaBarramentoResultados(){
    for(int i=0; i<largura_escrita; i++){
        barramentoResultados[i]=0;
    }
}

void colocaBarramento(int valor){
    barramento=valor;
}

void colocaPalavraBarramento(){
    __uint32_t palavra;
    for(int i = 0; i<4; i++){
        palavra = (palavra << 8) | memoria[pc + i];
    }
    barramento = palavra;
}

int pegaBarramento(){
    return barramento;
}

void colocaBarramentoResultados(int valor){
    if(barramentoResultados==NULL){
        printf("ERRO\n");
    }
    else{

        for(int i=0; i<largura_escrita; i++){
            if(barramentoResultados[i]==0 && pegaBarramentoResultados(valor)==-1){
                barramentoResultados[i]=valor;
            }
        }   
    }
   
}

int pegaBarramentoResultados(int resultado){
    for(int i=0; i<largura_escrita; i++){
        if(barramentoResultados[i]==resultado){
            return i;
        }
    }
    return -1;
}

void printBarramentoResultados(){
    for(int i=0; i<largura_escrita; i++){
        printf("Linha %d: %d\n",i,barramentoResultados[i]);
    }
}