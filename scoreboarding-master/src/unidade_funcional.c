#include <stdio.h>
#include <stdlib.h>
#include "unidade_funcional.h"
#include "processor.h"
#include "main.h"
#include <stdbool.h>

conjuntoUFS unidadesFuncionais;
UF *vetorResultados[32];

/* void printStatusReg(){
    for(int i=0; i<32; i++){
        printf()
    }
} */

const char* tipoToString(tipoUF tipo) {
    switch (tipo) {
        case ADD:
            return "ADD";
            break;
        case MUL:
            return "MUL";
            break;
        case INT:
            return "INT";
            break;
        default:
            return "-";
            break;
    }
}
/* 
void printConjuntoUFS(conjuntoUFS* conjunto) {
    printf("UF Add:\n");
    printUF(conjunto->ufAdd);
    
    printf("UF Mul:\n");
    printUF(conjunto->ufMul);
    
    printf("UF Int:\n");
    printUF(conjunto->ufInt);
} */


void printUFS(UF* ufs, int qtde){

    printf("UF  Tipo  Busy   Operacao   Fi      Fj      Fk       Qj       Qk     Rj     Rk\n");
    for (int i = 0; i < qtde; i++) {
        printf("%-4d %-4s %-7s %-7u %-7u %-7u %-7u %-6p %-6p %-6s %-7s\n",
               i, tipoToString(ufs[i].tipo), ufs[i].busy ? "true" : "false", ufs[i].operacao,
               ufs[i].fi, ufs[i].fj, ufs[i].fk, ufs[i].qj, ufs[i].qk,
               ufs[i].rj ? "true" : "false", ufs[i].rk ? "true" : "false");
    }
}


void inicializaUFs(int add, int mul, int inter){
    unidadesFuncionais.ufAdd = (UF*)malloc(sizeof(UF)*add);
    unidadesFuncionais.ufInt = (UF*)malloc(sizeof(UF)*inter);
    unidadesFuncionais.ufMul = (UF*)malloc(sizeof(UF)*mul);
    unidadesFuncionais.qtdeADD = add;
    unidadesFuncionais.qtdeINT = inter;
    unidadesFuncionais.qtdeMUL = mul;
    
    for(int i=0; i<unidadesFuncionais.qtdeADD; i++){
        /* unidadesFuncionais.ufAdd[i].qtde_ciclos = -1;
        unidadesFuncionais.ufAdd[i].fi = -1;
        unidadesFuncionais.ufAdd[i].fj = -1;
        unidadesFuncionais.ufAdd[i].fk = -1; */
        unidadesFuncionais.ufAdd[i].tipo=0;
    }
    for(int i=0; i<unidadesFuncionais.qtdeMUL; i++){
        /* unidadesFuncionais.ufMul[i].qtde_ciclos = -1;
        unidadesFuncionais.ufMul[i].fi = -1;
        unidadesFuncionais.ufMul[i].fj = -1;
        unidadesFuncionais.ufMul[i].fk = -1; */
        unidadesFuncionais.ufMul[i].tipo=1;
    }
    for(int i=0; i<unidadesFuncionais.qtdeINT; i++){
        /* unidadesFuncionais.ufInt[i].qtde_ciclos = -1;
        unidadesFuncionais.ufInt[i].fi = -1;
        unidadesFuncionais.ufInt[i].fj = -1;
        unidadesFuncionais.ufInt[i].fk = -1; */
        unidadesFuncionais.ufInt[i].tipo=2;
    } 
}

int getUFdisponivel(int tipo){
    if(tipo==0){
        for(int i=0; i<unidadesFuncionais.qtdeADD; i++){
            //printf("\nBUSY: %d ", unidadesFuncionais.ufAdd[i].busy);
            if(unidadesFuncionais.ufAdd[i].busy==0){
                return i;
            }
        }
    }
    else if(tipo==1){
        for(int i=0; i<unidadesFuncionais.qtdeMUL; i++){
            if(unidadesFuncionais.ufMul[i].busy==0){
                return i;
            }
        }
    }
    else if(tipo==2){
        for(int i=0; i<unidadesFuncionais.qtdeINT; i++){
            if(unidadesFuncionais.ufInt[i].busy==0){
                return i;
            }
        }
    }

    return -1;
}

int getTipoUF(int instrucao){
    int tp=0;
    if(getOpcode(instrucao)<4){
        tp = 0;
    }
    else if(getOpcode(instrucao)==4 || getOpcode(instrucao)==5){
        tp = 1;
    }
    else if(getOpcode(instrucao)>5 && getOpcode(instrucao)<16){
        tp = 2;
    }
    return tp;
} 

void resetaUF(UF* uf){
    vetorResultados[uf->fi]=NULL;
    uf->instrucao=0;
    uf->busy = 0;
    uf->fi = 0;
    uf->fj = 0;
    uf->fk = 0;
    uf->operacao = -1;
    uf->qj = NULL;
    uf->qk = NULL;
    uf->rj = (uf->qj == NULL);
    uf->rk = (uf->qk == NULL);
    uf->qtde_ciclos = 0;
}
