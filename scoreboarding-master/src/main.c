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


// Função que separa os valores relacionados às quantidades de UFs e ciclos
int getValor(const char *linha) { 
    char *separador = strstr(linha, ":");
    if (separador != NULL) {
        int valor;
        sscanf(separador + 1, "%d", &valor);
        return valor;
    }
    return 0; // Caso não encontre o separador, retorna 0.
}

// Função que obtém os dados a serem inseridos nos 100 primeiros endereços do programa.
void getValoresDados(char *linha){
	int numero;
	char *ptr = linha;
	while (*ptr != '\0'){
        if(isdigit(*ptr) || (*ptr == '-' && isdigit(*(ptr + 1)))){
            // Se o caractere atual for um dígito, construa o número
            sscanf(ptr, "%d", &numero);
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
	enum categoria { NENHUMA, UF, INST, PL, DADOS };
    enum categoria categoria = NENHUMA;
	

	if (arquivo == NULL) {
	    printf("Erro ao abrir o arquivo.\n");
		return 0;
	}
	//printf("AAAAAAAAAA");
	while(fgets(buffer, sizeof(buffer), arquivo)){
		int contadorlinha = 0;
		if((strcmp(buffer, "\r\n")==0) || (strcmp(buffer, "\n")==0) || (strcmp(buffer, "\0")==0)){
			continue;
		}
		else{
			while (buffer[contadorlinha] == ' ' || buffer[contadorlinha] == '\t') {
				contadorlinha++;
			}
			if(buffer[contadorlinha] == '#'){ // Indica que a linha é um comentário

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
				}
				else if(strcmp(buffer, "*/\n") == 0){
					categoria = NENHUMA;
				}  
				else{
					if(categoria == PL){ 
						if((strcmp(buffer, "\r\n")==0) || (strcmp(buffer, "\n")==0) || (strcmp(buffer, "\0")==0)){
							continue;
						}
						else{
							qtdeInsts++;
						} 
					}
				}
			}
		}
	}

	categoria = NENHUMA;

	// A largura de escrita de resultados pode ser definida entre 1 e 8.
	if(largura<1 || largura>8){
		printf("\nSelecione uma largura de escrita entre 1 e 8.");
		return 0;
	}

	// O programa só funcionará se a memória suportar o tamanho do programa.
	if(memsize>=(qtdeInsts*4)+400){
		inicializaMemoria(memsize);
	}
	else{
		printf("\nA memória precisa ser grande o suficiente para suportar o programa de entrada.");
		printf("\nPara suportar este programa, o tamanho mínimo para a memória é de %d", (qtdeInsts*4)+400);
		return 0;
	}

	inicializaBarramentoResultados(largura);
	fseek(arquivo, 0, SEEK_SET);
	
	while(fgets(buffer, sizeof(buffer), arquivo)){
		int contadorlinha = 0;
		if((strcmp(buffer, "\r\n")==0) || (strcmp(buffer, "\n")==0) || (strcmp(buffer, "\0")==0)){
			continue;
		}
		else{
			while (buffer[contadorlinha] == ' ' || buffer[contadorlinha] == '\t') {
				contadorlinha++;
			}
			if(buffer[contadorlinha] == '#'){ // Indica que a linha é um comentário
				//printf("\nComentário");
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
								//QUANTIDADE DE CICLOS DA ADDI
								ciclosParaExecutar[1]=valor;
							}
						}
						else if (strstr(buffer, "add")){
							valor = getValor(buffer);
							if (categoria == UF){
								qtdeAdd = valor; //QUANTIDADE DE UF ADD
							}
							else if (categoria == INST){
								//QUANTIDADE DE CICLOS DA ADD
								ciclosParaExecutar[0]=valor;
							}
						} 
						else if (strstr(buffer, "mul")) {
							valor = getValor(buffer);
							if (categoria == UF){
								qtdeMul = valor; //QUANTIDADE DE UF MUL
							}
							else if (categoria == INST){
								//QUANTIDADE DE CICLOS DA MUL
								ciclosParaExecutar[4]=valor;
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
								//QUANTIDADE DE CICLOS DA DIV
								ciclosParaExecutar[5]=valor;
							}
						}
						else if (strstr(buffer, "subi")) {
							valor = getValor(buffer);
							if (categoria == INST){
								//QUANTIDADE DE CICLOS DA SUBI
								ciclosParaExecutar[3]=valor;
							}
						}
						else if (strstr(buffer, "sub")) {
							valor = getValor(buffer);
							if (categoria == INST){
								//QUANTIDADE DE CICLOS DA SUB
								ciclosParaExecutar[2]=valor;
							}
						}
						else if (strstr(buffer, "lw")) {
							valor = getValor(buffer);
							if (categoria == INST){
								//QUANTIDADE DE CICLOS DA LW
								ciclosParaExecutar[14]=valor;
							}
						}
						else if (strstr(buffer, "sw")) {
							valor = getValor(buffer);
							if (categoria == INST){
								//QUANTIDADE DE CICLOS DA SW
								ciclosParaExecutar[15]=valor;
							}
						}
						else if (strstr(buffer, "beq")) {
							valor = getValor(buffer);
							if (categoria == INST){
								//QUANTIDADE DE CICLOS DA BEQ
								ciclosParaExecutar[11]=valor;
							}
						}
						else if (strstr(buffer, "bne")) {
							valor = getValor(buffer);
							if (categoria == INST){
								//QUANTIDADE DE CICLOS DA BNE
								ciclosParaExecutar[12]=valor;
							}
						}
						else if (strstr(buffer, "blt")) {
							valor = getValor(buffer);
							if (categoria == INST){
								//QUANTIDADE DE CICLOS DA BLT
								ciclosParaExecutar[9]=valor;
							}
						}
						else if (strstr(buffer, "bgt")) {
							valor = getValor(buffer);
							if (categoria == INST){
								//QUANTIDADE DE CICLOS DA BGT
								ciclosParaExecutar[10]=valor;
							}
						}
						else if (strstr(buffer, "j")) {
							valor = getValor(buffer);
							if (categoria == INST){
								//QUANTIDADE DE CICLOS DA J
								ciclosParaExecutar[13]=valor;
							}
						}
						else if (strstr(buffer, "and")) {
							valor = getValor(buffer);
							if (categoria == INST){
								//QUANTIDADE DE CICLOS DA AND
								ciclosParaExecutar[6]=valor;
							}
						}
						else if (strstr(buffer, "or")) {
							valor = getValor(buffer);
							if (categoria == INST){
								//QUANTIDADE DE CICLOS DA OR
								ciclosParaExecutar[7]=valor;
							}
						}
						else if (strstr(buffer, "not")) {
							valor = getValor(buffer);
							if (categoria == INST){
								//QUANTIDADE DE CICLOS DA NOT
								ciclosParaExecutar[8]=valor;
							}
						}
					}
					else if (categoria == DADOS){ // Carrega o que tem abaixo de ".data" para a memória						
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

	return 1;
}	
