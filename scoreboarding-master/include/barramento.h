#ifndef BARRAMENTO_H_
#define BARRAMENTO_H_

extern int barramento;
extern int *barramentoResultados;
extern int largura_escrita;
void colocaBarramento(int valor);
void colocaPalavraBarramento();
int pegaBarramento();
void colocaBarramentoResultados(int valor);
int pegaBarramentoResultados();
void inicializaBarramentoResultados(int largura);
void limpaBarramentoResultados();
void printBarramentoResultados();
#endif