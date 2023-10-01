#ifndef UNIDADE_FUNCIONAL_H
#define UNIDADE_FUNCIONAL_H

#include <stdbool.h>
#include "processor.h"

typedef enum { ADD, MUL, INT } tipoUF;

typedef struct UF {
    tipoUF tipo;
    int instrucao;
    int busy;
    int operacao;
    int fi;
    int fj;
    int fk;
    int valorfj;
    int valorfk;
    struct UF* qj;
    struct UF* qk;
    int rj;
    int rk;
    int qtde_ciclos;
} UF;

typedef struct {
    UF* ufAdd;
    UF* ufMul;
    UF* ufInt;
    int qtdeADD, qtdeMUL, qtdeINT;
} conjuntoUFS;

//Esse vetor serve para mostrar quais registradores de destino vão ser escritos ao final da execução
extern UF* vetorResultados[32]; 
extern conjuntoUFS unidadesFuncionais;

void inicializaUFs(int add, int mul, int inteiro);
int getUFdisponivel(int tipo);
int getTipoUF(int instrucao);

#endif

