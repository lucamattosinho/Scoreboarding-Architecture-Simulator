#ifndef TRADUTOR_H
#define TRADUTOR_H

int getCodigoOpcode(const char opcode[4]);
int instrucaoParaBinario(char *buffer);
void printBinario(int num);
char* instrucaoToString(int inst);
#endif
