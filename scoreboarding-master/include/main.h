#ifndef MAIN_H
#define MAIN_H
extern FILE *arq_saida;
extern int ciclosParaExecutar[16];
extern int clocki;
extern int stalled;
extern int bufferEscrita[32];
extern int qtdeAdd, qtdeInt, qtdeMul;
int getValor(const char *linha);
int leituraArquivo(char * file, int memsize, char * output, int largura);
#endif