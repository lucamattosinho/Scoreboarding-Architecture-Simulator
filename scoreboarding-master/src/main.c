#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tradutor.h"
#include "memoria.h"
#include "registradores.h"
#include "main.h"
#include "barramento.h"
#include "unidade_funcional.h"
#include "scoreboarding.h"

FILE *arq_saida;
int ciclosParaExecutar[16];
int clocki;
int stalled;
int qtdeAdd, qtdeInt, qtdeMul;
int addCiclos, mulCiclos, lwCiclos, subCiclos, divCiclos, swCiclos, bgtCiclos, jCiclos;
int addiCiclos, subiCiclos, andCiclos, orCiclos, notCiclos, bltCiclos, beqCiclos, bneCiclos;

int getValor(const char *linha) { // Função que separa os valores relacionados às quantidades de UFs e ciclos
    char *separador = strstr(linha, ":");
    if (separador != NULL) {
        int valor;
        sscanf(separador + 1, "%d", &valor);
        return valor;
    }
    return 0; // Caso não encontre o separador, retorna 0.
}

void getValoresDados(char *linha){
	int numero;
	char *ptr = linha;
	printf("\nLinha %s", linha);
	while (*ptr != '\0'){
        if(isdigit(*ptr) || (*ptr == '-' && isdigit(*(ptr + 1)))){
            // Se o caractere atual for um dígito, construa o número
            sscanf(ptr, "%d", &numero);
            printf("Número lido: %d\n", numero);
			insereMemoria(numero);
            // Encontre o próximo espaço em branco ou o final da linha
            while (isdigit(*ptr) || (*ptr == '-' && isdigit(*(ptr + 1))))
                ptr++;
            } 
		else{
            // Se não for um dígito, vá para o próximo caractere
            ptr++;
        }
    }
}

int leituraArquivo(char * file, int memsize, char* output, int largura){
	FILE *arquivo;
	char buffer[256];
	// Abre o arquivo em modo de leitura
	arquivo = fopen(file, "r");
	// Direciona o arquivo de saída para o diretório "saidas"
	char* dir_saida = "saidas/";
	char caminhoCompleto[256];
	snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s%s", dir_saida, output);
	// Se o usuário passou uma saída, o resultado da simulação será direcionado será direcionada pra ela.
	if(output){
		arq_saida = freopen(caminhoCompleto, "w", stdout);
	}
    inicializaMemoria(memsize);
	inicializaBarramentoResultados(largura);
	if(largura<1 || largura>8){
		printf("\nSelecione uma largura de escrita entre 1 e 8.");
		return 0;
	}
	if (arquivo == NULL) {
	    printf("Erro ao abrir o arquivo.\n");
		return 0;
	}
	enum categoria { NENHUMA, UF, INST, PL, DADOS };
    enum categoria categoria = NENHUMA;
    
	while(fgets(buffer, sizeof(buffer), arquivo)){
		int contadorlinha = 0;
		//printf("\n%s", buffer);
		if((strcmp(buffer, "\r\n")==0) || (strcmp(buffer, "\n")==0) || (strcmp(buffer, "\0")==0)){
			continue;
		}
		else{
			while (buffer[contadorlinha] == ' ' || buffer[contadorlinha] == '\t') {
				contadorlinha++;
			}
			if(buffer[contadorlinha] == '#'){ // Indica que a linha é um comentário
				printf("\nComentário");
			}
			else{ // Utilizamos flags para indicar do que se trata a linha que será lida
				if (strcmp(buffer, "UF\n") == 0) { // Verifica se a linha contém a palavra-chave "UF"
					categoria = UF;
				}
				else if (strcmp(buffer, "INST\n") == 0) { // Verifica se a linha contém a palavra-chave "INST"
					categoria = INST;
				}
				else if(strcmp(buffer, ". data\n") == 0 || strcmp(buffer, ".data\n") == 0){ // Verifica se a linha contém ".data"
					categoria = DADOS;
				}
				else if(strcmp(buffer, ". text\n") == 0 || strcmp(buffer, ".text\n") == 0){ // Verifica se a linha contém ".text"
					categoria = PL;
					pc = 400;
				}
				else if(strcmp(buffer, "*/\n") == 0){
					categoria = NENHUMA;
				}  
				else{
					int valor = 0;
					if (categoria == INST || categoria == UF){
						if (strstr(buffer, "addi")){
							valor = getValor(buffer);
							if (categoria == INST){
								addiCiclos = valor; //QUANTIDADE DE CICLOS DA ADDI
								ciclosParaExecutar[1]=addiCiclos;
							}
						}
						else if (strstr(buffer, "add")){
							valor = getValor(buffer);
							if (categoria == UF){
								qtdeAdd = valor; //QUANTIDADE DE UF ADD
							}
							else if (categoria == INST){
								addCiclos = valor; //QUANTIDADE DE CICLOS DA ADD
								ciclosParaExecutar[0]=addCiclos;
							}
						} 
						else if (strstr(buffer, "mul")) {
							valor = getValor(buffer);
							if (categoria == UF){
								qtdeMul = valor; //QUANTIDADE DE UF MUL
							}
							else if (categoria == INST){
								mulCiclos = valor; //QUANTIDADE DE CICLOS DA MUL
								ciclosParaExecutar[4]=mulCiclos;
							}
						} 
						else if (strstr(buffer, "inteiro")) {
							valor = getValor(buffer);
							if (categoria == UF){
								qtdeInt = valor; //QUANTIDADE DE UF INTEGER
							}
						}
						else if (strstr(buffer, "div")) {
							valor = getValor(buffer);
							if (categoria == INST){
								divCiclos = valor; //QUANTIDADE DE CICLOS DA DIV
								ciclosParaExecutar[5]=mulCiclos;
							}
						}
						else if (strstr(buffer, "subi")) {
							valor = getValor(buffer);
							if (categoria == INST){
								subiCiclos = valor; //QUANTIDADE DE CICLOS DA SUBI
								ciclosParaExecutar[3]=subiCiclos;
							}
						}
						else if (strstr(buffer, "sub")) {
							valor = getValor(buffer);
							if (categoria == INST){
								subCiclos = valor; //QUANTIDADE DE CICLOS DA SUB
								ciclosParaExecutar[2]=subCiclos;
							}
						}
						else if (strstr(buffer, "lw")) {
							valor = getValor(buffer);
							if (categoria == INST){
								lwCiclos = valor; //QUANTIDADE DE CICLOS DA LW
								ciclosParaExecutar[14]=lwCiclos;
							}
						}
						else if (strstr(buffer, "sw")) {
							valor = getValor(buffer);
							if (categoria == INST){
								swCiclos = valor; //QUANTIDADE DE CICLOS DA SW
								ciclosParaExecutar[15]=swCiclos;
							}
						}
						else if (strstr(buffer, "beq")) {
							valor = getValor(buffer);
							if (categoria == INST){
								beqCiclos = valor; //QUANTIDADE DE CICLOS DA BEQ
								ciclosParaExecutar[11]=beqCiclos;
							}
						}
						else if (strstr(buffer, "bne")) {
							valor = getValor(buffer);
							if (categoria == INST){
								bneCiclos = valor; //QUANTIDADE DE CICLOS DA BNE
								ciclosParaExecutar[12]=bneCiclos;
							}
						}
						else if (strstr(buffer, "blt")) {
							valor = getValor(buffer);
							if (categoria == INST){
								bltCiclos = valor; //QUANTIDADE DE CICLOS DA BLT
								ciclosParaExecutar[9]=bltCiclos;
							}
						}
						else if (strstr(buffer, "bgt")) {
							valor = getValor(buffer);
							if (categoria == INST){
								bgtCiclos = valor; //QUANTIDADE DE CICLOS DA BGT
								ciclosParaExecutar[10]=bgtCiclos;
							}
						}
						else if (strstr(buffer, "j")) {
							valor = getValor(buffer);
							if (categoria == INST){
								jCiclos = valor; //QUANTIDADE DE CICLOS DA J
								ciclosParaExecutar[13]=jCiclos;
							}
						}
						else if (strstr(buffer, "and")) {
							valor = getValor(buffer);
							if (categoria == INST){
								andCiclos = valor; //QUANTIDADE DE CICLOS DA AND
								ciclosParaExecutar[6]=andCiclos;
							}
						}
						else if (strstr(buffer, "or")) {
							valor = getValor(buffer);
							if (categoria == INST){
								orCiclos = valor; //QUANTIDADE DE CICLOS DA OR
								ciclosParaExecutar[7]=orCiclos;
							}
						}
						else if (strstr(buffer, "not")) {
							valor = getValor(buffer);
							if (categoria == INST){
								notCiclos = valor; //QUANTIDADE DE CICLOS DA NOT
								ciclosParaExecutar[8]=notCiclos;
							}
						}
					}
					else if (categoria == DADOS){ // Carrega o que tem abaixo de ".data" para a memória						
						//dado = atoi(buffer);
						getValoresDados(buffer);
					}
					else{
						int inst;
						if(categoria == PL){ // Carrega pra memória as instruções do programa (abaixo de ".text")
							if((strcmp(buffer, "\r\n")==0) || (strcmp(buffer, "\n")==0) || (strcmp(buffer, "\0")==0)){
								continue;
							}
							else{
								inst = instrucaoParaBinario(buffer);
								insereMemoria(inst);
								qtdeInsts++;
							} 
						}
					}
				}
			}
		} 
	}
	inicializaUFs(qtdeAdd, qtdeMul, qtdeInt); //inicializa as Unidades Funcionais dos 3 tipos
	inicializaStatusInstrucoes(); //inicializa a tabela de status das instrucoes
	inicializaVetorForwarding(); //inicializa o vetor que será utilizado para simular a falta da retroalimentação do scoreboarding
	fclose(arquivo); // Fecha o arquivo de entrada
    printMemoria();
	if(memsize<101){
		printf("\nERRO: Não há espaço na memória para todas as instruções do programa.\n");
		return 0;
	}
	// Imprime os valores lidos
	printf("\n\nUFs - add: %d, mul: %d, int: %d\n\n", qtdeAdd, qtdeMul, qtdeInt);
	printf("Ciclos de clock necessarios\npara completar a execucao:\nadd: %d, mul: %d, lw: %d\n", addCiclos, mulCiclos, lwCiclos);
    printf("div: %d, and: %d, addi: %d\nsubi: %d, or: %d, not: %d\n", divCiclos, andCiclos, addiCiclos, subiCiclos, orCiclos, notCiclos);
    printf("bgt: %d, blt: %d, beq: %d\nbne: %d, j: %d, sw: %d, sub: %d\n", bgtCiclos, bltCiclos, beqCiclos, bneCiclos, jCiclos, swCiclos, subCiclos);
	//inicializaPipeline();
	printRegistradores();

	/*if(output!=NULL){
		printf("SAIDA: %s", output);
		//printf("Escrevendo os resultados no arquivo %s.txt\n", output);
		arq_saida = freopen(output,"w", stdout);
		//stdout=arq_saida;
	}*/
	
return 1;
}	
