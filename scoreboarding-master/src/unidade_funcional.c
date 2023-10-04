#include <stdio.h>
#include <stdlib.h>
#include "unidade_funcional.h"
#include "processor.h"
#include "main.h"
#include <stdbool.h>

conjuntoUFS unidadesFuncionais;
UF *vetorResultados[32];

// Retorna a string de acordo com o tipo da unidade funcional
const char* tipoToString(int tipo) {
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

// Inicialização das unidades funcionais
void inicializaUFs(int add, int mul, int inter){
    unidadesFuncionais.ufAdd = (UF*)malloc(sizeof(UF)*add);
    unidadesFuncionais.ufInt = (UF*)malloc(sizeof(UF)*inter);
    unidadesFuncionais.ufMul = (UF*)malloc(sizeof(UF)*mul);
    unidadesFuncionais.qtdeADD = add;
    unidadesFuncionais.qtdeINT = inter;
    unidadesFuncionais.qtdeMUL = mul;
    
    for(int i=0; i<unidadesFuncionais.qtdeADD; i++){
        unidadesFuncionais.ufAdd[i].tipo=0;
    }
    for(int i=0; i<unidadesFuncionais.qtdeMUL; i++){
        unidadesFuncionais.ufMul[i].tipo=1;
    }
    for(int i=0; i<unidadesFuncionais.qtdeINT; i++){
        unidadesFuncionais.ufInt[i].tipo=2;
    } 
}

// Busca qual unidade funcional está disponível para a emissão.
// Caso nenhuma unidade esteja disponível, retorna -1.
int getUFdisponivel(int tipo){
    if(tipo==0){
        for(int i=0; i<unidadesFuncionais.qtdeADD; i++){
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

// Retorna o tipo da unidade funcional de uma instrução
// de acordo com seu binário.
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
    uf->instrucao=0;
    uf->busy = 0;
    uf->fi = 0;
    uf->fj = 0;
    uf->fk = 0;
    uf->valorfj = 0;
    uf->valorfk = 0;
    uf->operacao = 0;
    uf->qj = NULL;
    uf->qk = NULL;
    uf->rj = 0;
    uf->rk = 0;
}