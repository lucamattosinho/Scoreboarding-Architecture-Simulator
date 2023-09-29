#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "barramento.h"
#include "unidade_funcional.h"
#include "scoreboarding.h"
#include "registradores.h"
#include "memoria.h"

extern int instsBuscadas;
extern int instrucoesEmitidas;
extern int instrucoesEfetivadas;
extern int *vetorForwarding;
int getOpcode(int instrucao);
int getRegistradorDestino(int instrucao);
int getRegistradorFonte1(int instrucao);
int getRegistradorFonte2(int instrucao);
int getImm(int instrucao);
void emiteInstrucao();
int buscaInstrucao();
void leituraDeOperandos();
void execucao();
int executaInstrucao(int destino, int fonte1, int fonte2, int opcode);
void escritaResultados();
void inicializaVetorForwarding();

#endif 
