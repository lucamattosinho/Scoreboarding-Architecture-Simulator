#ifndef REGISTRADORES_H
#define REGISTRADORES_H

#include <stdio.h>
#include <stdlib.h>

extern int bancoRegs[32];
extern int pc;
extern int pc2;
extern int ir;

void escreverRegistrador();
int lerRegistrador();
void armazenarValorRegistrador(const char *linha, int tamanho);
void printRegistradores();

#endif 