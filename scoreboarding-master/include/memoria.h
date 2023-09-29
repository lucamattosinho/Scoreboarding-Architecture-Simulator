#ifndef MEMORIA_H
#define MEMORIA_H

extern int qtdeInsts;
extern int tam;
extern int tam2;
extern int *memoria;
extern int *memodemo;
void inicializaMemoria(int m);
void colocaMemoria(int instrucao, int indice);
int pegaMemoria(int indice);
int pegaMemoriaLw(int fonte1, int fonte2);
void insereMemoria(int instrucao);
void printMemoria();

#endif
