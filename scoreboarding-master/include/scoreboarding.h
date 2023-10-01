#ifndef SCOREBOARDING_H
#define SCOREBOARDING_H

void statusRegistradores();
void statusUFs();

typedef struct{
    int instrucao;
    int busca;
    int emissao;
    int leitura_op;
    int execucaoinicio;
    int execucaofim;
    int escrita;
}statusInstrucoes;

extern statusInstrucoes *statusI;

const char* tipoToString(int tipo);
void aumentaStatusInstrucoes();
void printStatusInstrucoes();
void inicializaStatusInstrucoes();
int getIndiceInstrucao(int instrucao);
int getIndiceInstrucaoLO(int instrucao);
int getIndiceInstrucaoEX(int instrucao);
int getIndiceInstrucaoER(int instrucao);
void printStatusReg();

#endif