#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "processor.h"
#include "scoreboarding.h"
#include "memoria.h"
#include "tradutor.h"
#include "registradores.h"
#include "unidade_funcional.h"

statusInstrucoes *statusI;


void inicializaStatusInstrucoes(){
    statusI = (statusInstrucoes*)malloc(sizeof(statusInstrucoes)*(qtdeInsts));
    int j=0;
    int i=400;
    for(j=0; j<qtdeInsts-2; j++){
        
        statusI[j].instrucao = pegaMemoria(i);

        i=i+4;
    } 
}

void aumentaStatusInstrucoes(){

    statusInstrucoes *newStatusI = (statusInstrucoes*)malloc(sizeof(statusInstrucoes)*(instsBuscadas+2));
    if (newStatusI == NULL) {   
        printf("Erro na alocação de memória.\n");
        free(newStatusI); // Liberar memória alocada para o vetor original
    }
    for (int i = 0; i < instsBuscadas+1; i++) {
        newStatusI[i] = statusI[i];
    }
    
    free(statusI);
    statusI = (statusInstrucoes*)malloc(sizeof(statusInstrucoes)*(instsBuscadas+2));
    for (int i = 0; i < instsBuscadas+1; i++) {
        statusI[i] = newStatusI[i];
    }
    
    free(newStatusI);
}


void statusRegistradores(){
    for(int i=0; i<32; i++){
        printf("\nr%d = %d", i, bancoRegs[i]);
    }
}


//tentar com um while pra pegar sempre a última
int getIndiceInstrucao(int instrucao){
    int res;
    for (int i=0; i<instsBuscadas+1; i++){
        if(statusI[i].instrucao==instrucao){// && statusI[i].escrita==0){
            res = i;
        }
    }
    return res;
}

int getIndiceInstrucaoLO(int instrucao){
    int res;
    for (int i=0; i<instsBuscadas+1; i++){
        if(statusI[i].instrucao==instrucao && statusI[i].emissao!=0){// && statusI[i].escrita==0){
            res = i;
        }
    }
    return res;
}

int getIndiceInstrucaoEX(int instrucao){
    int res;
    for (int i=0; i<instsBuscadas+1; i++){
        if(statusI[i].instrucao==instrucao && statusI[i].leitura_op!=0){// && statusI[i].escrita==0){
            res = i;
        }
    }
    return res;
}

int getIndiceInstrucaoER(int instrucao){
    int res = 0;
    for (int i=0; i<instsBuscadas+1; i++){
        if(statusI[i].instrucao==instrucao && statusI[i].execucaofim!=0){
            res = i;
        }
    }
    return res;
}

int encontraQ(UF* unidadefuncional){
    for(int i=0; i<unidadesFuncionais.qtdeADD; i++){
        if(&unidadesFuncionais.ufAdd[i] == unidadefuncional){
            return i;
        }
    }
    for(int i=0; i<unidadesFuncionais.qtdeMUL; i++){
        if(&unidadesFuncionais.ufMul[i] == unidadefuncional){
            return i;
        }
    }
    for(int i=0; i<unidadesFuncionais.qtdeINT; i++){
        if(&unidadesFuncionais.ufInt[i] == unidadefuncional){
            return i;
        }
    }
    return 0;
}

void printStatusInstrucoes() {
    printf("\n\n%-25s%-14s%-14s%-14s%-14s%-14s\n\n", "Instrucao", "Busca", "Emissao", "Leitura_OP", "Execucao", "Escrita");
    for (int i = 0; i < instsBuscadas; i++) {
        printf("%-25s%-14d%-14d%-14d%-14d%-14d\n",
               instrucaoToString(statusI[i].instrucao), statusI[i].busca, statusI[i].emissao,
               statusI[i].leitura_op, statusI[i].execucaofim, statusI[i].escrita);
    }
}

void statusUFs(){
    printf("\n| %-10s | %-5s | %-10s | %-5s | %-5s | %-5s | %-10s | %-10s | %-5s | %-5s | %-15s |\n", "Tipo", "Busy", "Operacao", "Fi", "Fj", "Fk", "Qj", "Qk", "Rj", "Rk", "Quantidade Ciclos");
    for(int i=0; i<unidadesFuncionais.qtdeADD; i++){
        printf("|------------|-------|------------|-------|-------|-------|------------|------------|-------|-------|-------------------|\n");
        char* qj;
        if(unidadesFuncionais.ufAdd[i].qj!=0){
            if(unidadesFuncionais.ufAdd[i].qj->tipo==0){
                qj = "ADD";
            }
            else if(unidadesFuncionais.ufAdd[i].qj->tipo==1){
                qj = "MUL";
            }
            else if(unidadesFuncionais.ufAdd[i].qj->tipo==2){
                qj = "INT";
            }
        }
        else{
            qj = "-";
        }
        char* qk;
        if(unidadesFuncionais.ufAdd[i].qk!=0){
            if(unidadesFuncionais.ufAdd[i].qk->tipo==0){
                qk = "ADD";
            }
            else if(unidadesFuncionais.ufAdd[i].qk->tipo==1){
                qk = "MUL";
            }
            else if(unidadesFuncionais.ufAdd[i].qk->tipo==2){
                qk = "INT";
            }
        }
        else{
            qk = "-";
        }
        printf("| %-10s | %-5d | %-10u | %-5u | %-5u | %-5u | %-5s%-5d | %-5s%-5d | %-5d | %-5d | %-17d |\n",
        "ADD",
        unidadesFuncionais.ufAdd[i].busy,
        unidadesFuncionais.ufAdd[i].operacao,
        unidadesFuncionais.ufAdd[i].fi,
        unidadesFuncionais.ufAdd[i].fj,
        unidadesFuncionais.ufAdd[i].fk,
        qj, encontraQ(unidadesFuncionais.ufAdd[i].qj),
        qk, encontraQ(unidadesFuncionais.ufAdd[i].qk),
        unidadesFuncionais.ufAdd[i].rj,
        unidadesFuncionais.ufAdd[i].rk,
        unidadesFuncionais.ufAdd[i].qtde_ciclos);
    }
    for(int i=0; i<unidadesFuncionais.qtdeMUL; i++){
        printf("|------------|-------|------------|-------|-------|-------|------------|------------|-------|-------|-------------------|\n");
        char* qj;
        if(unidadesFuncionais.ufMul[i].qj!=0){
            if(unidadesFuncionais.ufMul[i].qj->tipo==0){
                qj = "ADD";
            }
            else if(unidadesFuncionais.ufMul[i].qj->tipo==1){
                qj = "MUL";
            }
            else if(unidadesFuncionais.ufMul[i].qj->tipo==2){
                qj = "INT";
            }
        }
        else{
            qj = "-";
        }
        char* qk;
        if(unidadesFuncionais.ufMul[i].qk!=0){
            if(unidadesFuncionais.ufMul[i].qk->tipo==0){
                qk = "ADD";
            }
            else if(unidadesFuncionais.ufMul[i].qk->tipo==1){
                qk = "MUL";
            }
            else if(unidadesFuncionais.ufMul[i].qk->tipo==2){
                qk = "INT";
            }
        }
        else{
            qk = "-";
        }
        printf("| %-10s | %-5d | %-10u | %-5u | %-5u | %-5u | %-5s%-5d | %-5s%-5d | %-5d | %-5d | %-17d |\n",
        "MUL",
        unidadesFuncionais.ufMul[i].busy,
        unidadesFuncionais.ufMul[i].operacao,
        unidadesFuncionais.ufMul[i].fi,
        unidadesFuncionais.ufMul[i].fj,
        unidadesFuncionais.ufMul[i].fk,
        qj, encontraQ(unidadesFuncionais.ufMul[i].qj),
        qk, encontraQ(unidadesFuncionais.ufMul[i].qk),
        unidadesFuncionais.ufMul[i].rj,
        unidadesFuncionais.ufMul[i].rk,
        unidadesFuncionais.ufMul[i].qtde_ciclos);
    }
    for(int i=0; i<unidadesFuncionais.qtdeINT; i++){
        printf("|------------|-------|------------|-------|-------|-------|------------|------------|-------|-------|-------------------|\n");
        char* qj;
        if(unidadesFuncionais.ufInt[i].qj!=0){
            if(unidadesFuncionais.ufInt[i].qj->tipo==0){
                qj = "ADD";
            }
            else if(unidadesFuncionais.ufInt[i].qj->tipo==1){
                qj = "MUL";
            }
            else if(unidadesFuncionais.ufInt[i].qj->tipo==2){
                qj = "INT";
            }
        }
        else{
            qj = "-";
        }
        char* qk;
        if(unidadesFuncionais.ufInt[i].qk!=0){
            if(unidadesFuncionais.ufInt[i].qk->tipo==0){
                qk = "ADD";
            }
            else if(unidadesFuncionais.ufInt[i].qk->tipo==1){
                qk = "MUL";
            }
            else if(unidadesFuncionais.ufInt[i].qk->tipo==2){
                qk = "INT";
            }
        }
        else{
            qk = "-";
        }
        printf("| %-10s | %-5d | %-10u | %-5u | %-5u | %-5u | %-5s%-5d | %-5s%-5d | %-5d | %-5d | %-17d |\n",
        "INT",
        unidadesFuncionais.ufInt[i].busy,
        unidadesFuncionais.ufInt[i].operacao,
        unidadesFuncionais.ufInt[i].fi,
        unidadesFuncionais.ufInt[i].fj,
        unidadesFuncionais.ufInt[i].fk,
        qj, encontraQ(unidadesFuncionais.ufInt[i].qj),
        qk, encontraQ(unidadesFuncionais.ufInt[i].qk),
        unidadesFuncionais.ufInt[i].rj,
        unidadesFuncionais.ufInt[i].rk,
        unidadesFuncionais.ufInt[i].qtde_ciclos);
    }
}

void printStatusReg(){
    printf("\n\n");
    char* nomeUF;
    int j;

    printf("--------------------------------------------------------------------------------------------------------------------------------------------------\n");
    
    
    for (int i = 0; i<11; i++) {
        printf("|%-5sR%d%-5s","",i,"");

    }
    printf("|");
    printf("\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------\n");



    for(int i=0; i<11; i++){

        if(vetorResultados[i]!=NULL){
            if(vetorResultados[i]->tipo==0){
                nomeUF = "ADD";
                for(j=0; j<unidadesFuncionais.qtdeADD; j++){
                    if(unidadesFuncionais.ufAdd[j].fi==i){
                        printf("|%-4s%s%d%-4s","",nomeUF,j,"");

                    }
                }
            }
            else if(vetorResultados[i]->tipo==1){
                nomeUF = "MUL";
                for(j=0; j<unidadesFuncionais.qtdeMUL; j++){
                    if(unidadesFuncionais.ufMul[j].fi==i){
                        printf("|%-4s%s%d%-4s","",nomeUF,j,"");


                    }
                }
            }
            else if(vetorResultados[i]->tipo==2){
                nomeUF = "INT";
                for(j=0; j<unidadesFuncionais.qtdeINT; j++){
                    if(unidadesFuncionais.ufInt[j].fi==i){
                        printf("|%-4s%s%d%-4s","",nomeUF,j,"");

                    }
                }
            }
        }
        else{
            nomeUF = "-";
            printf("|%-5s%s%-6s","",nomeUF,"");
        }
        

    }
    printf("|");
    printf("\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------\n");

    
    for (int i =11; i<22; i++) {
        printf("|%-5sR%d%-4s","",i,"");

    }
    printf("|");
    printf("\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------\n");



    for(int i=11; i<22; i++){

        if(vetorResultados[i]!=NULL){
            if(vetorResultados[i]->tipo==0){
                nomeUF = "ADD";
                for(j=0; j<unidadesFuncionais.qtdeADD; j++){
                    if(unidadesFuncionais.ufAdd[j].fi==i){
                        printf("|%-4s%s%d%-4s","",nomeUF,j,"");

                    }
                }
            }
            else if(vetorResultados[i]->tipo==1){
                nomeUF = "MUL";
                for(j=0; j<unidadesFuncionais.qtdeMUL; j++){
                    if(unidadesFuncionais.ufMul[j].fi==i){
                        printf("|%-4s%s%d%-4s","",nomeUF,j,"");


                    }
                }
            }
            else if(vetorResultados[i]->tipo==2){
                nomeUF = "INT";
                for(j=0; j<unidadesFuncionais.qtdeINT; j++){
                    if(unidadesFuncionais.ufInt[j].fi==i){
                        printf("|%-4s%s%d%-4s","",nomeUF,j,"");

                    }
                }
            }
        }
        else{
            nomeUF = "-";
            printf("|%-5s%s%-6s","",nomeUF,"");
        }
        

    }

    printf("|");
    printf("\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------\n");

    
    for (int i =22; i<32; i++) {
        printf("|%-5sR%d%-4s","",i,"");

    }
    printf("|");
    printf("\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------\n");



    for(int i=22; i<32; i++){

        if(vetorResultados[i]!=NULL){
            if(vetorResultados[i]->tipo==0){
                nomeUF = "ADD";
                for(j=0; j<unidadesFuncionais.qtdeADD; j++){
                    if(unidadesFuncionais.ufAdd[j].fi==i){
                        printf("|%-4s%s%d%-4s","",nomeUF,j,"");

                    }
                }
            }
            else if(vetorResultados[i]->tipo==1){
                nomeUF = "MUL";
                for(j=0; j<unidadesFuncionais.qtdeMUL; j++){
                    if(unidadesFuncionais.ufMul[j].fi==i){
                        printf("|%-4s%s%d%-4s","",nomeUF,j,"");


                    }
                }
            }
            else if(vetorResultados[i]->tipo==2){
                nomeUF = "INT";
                for(j=0; j<unidadesFuncionais.qtdeINT; j++){
                    if(unidadesFuncionais.ufInt[j].fi==i){
                        printf("|%-4s%s%d%-4s","",nomeUF,j,"");

                    }
                }
            }
        }
        else{
            nomeUF = "-";
            printf("|%-5s%s%-6s","",nomeUF,"");
        }
        

    }
    printf("|\n");
    printf("--------------------------------------------------------------------------------------------------------------------------------------------------\n");


}































