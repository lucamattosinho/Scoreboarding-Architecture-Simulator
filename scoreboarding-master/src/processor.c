#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "processor.h"
#include "registradores.h"
#include "unidade_funcional.h"
#include "scoreboarding.h"
#include "main.h"
#include "tradutor.h"

// Contador de instruções emitidas
int instrucoesEmitidas = 0;

// Contador de instruções efetivadas
int instrucoesEfetivadas = 0;

// Contador de instruções buscadas
int instsBuscadas;

// Contador de loops que ocorreram no programa
int qtdeloops;

// Vetor utilizado para evitar que
// uma instrução que possua dependência
// verdadeira com outra possa ler operandos
// logo no mesmo ciclo que a outra escreve,
// ou seja, para evitar que haja Forwarding
int *vetorForwarding;

// Inicializando o vetor.
void inicializaVetorForwarding(){
    vetorForwarding = (int*)malloc(sizeof(int)*(qtdeInsts));
}

// Função que aumenta o tamanho do espaço alocado
// para o vetor conforme ele vai crescendo com
// o programa.
void aumentaVetorForwarding(){

    int *newVetorForwarding = (int*)malloc(sizeof(int)*(instsBuscadas));

    if (newVetorForwarding == NULL) {   
        printf("Erro na alocação de memória.\n");
        free(newVetorForwarding); // Liberar memória alocada para o vetor original
    }
    for (int i = 0; i < instsBuscadas; i++) {
        newVetorForwarding[i] = vetorForwarding[i];
    }
    
    free(vetorForwarding);
    vetorForwarding = (int*)malloc(sizeof(int)*(instsBuscadas+1));
    for (int i = 0; i < instsBuscadas; i++) {
        vetorForwarding[i] = newVetorForwarding[i];
    }
    
    free(newVetorForwarding);
}

// Função da Busca do Pipeline
int buscaInstrucao(){
    // BUSCA SE NÃO ESTIVER COM STALL
    if(!stalled){
        instsBuscadas++;
        colocaPalavraBarramento();
        ir = pegaBarramento();

        // Aqui, caso a quantidade de instruções buscadas
        // exceda a quantidade de instruções que existem no programa
        // (caracterizando loop), a tabela de Status das Instruções
        // e o vetor anti-forwarding aumentam de tamanho
        if(instsBuscadas>qtdeInsts){
            aumentaStatusInstrucoes();
            aumentaVetorForwarding();
        }

        // Inicializando a instrução buscada na tabela
        // Status das Instruções
        statusI[instsBuscadas-1].instrucao = ir;
        statusI[instsBuscadas-1].emissao = 0;
        statusI[instsBuscadas-1].leitura_op = 0;
        statusI[instsBuscadas-1].execucaofim = 0;
        statusI[instsBuscadas-1].escrita = 0;

        statusI[instsBuscadas-1].busca = clocki;
        
        // Caso a instrução buscada seja um salto incondicional,
        // já será resolvida na busca.
        if(getOpcode(ir)==13){
            if(getImm(ir)%4!=0 || getImm(ir)<100){
                pc = pc+4;
            }
            else{
                pc = getImm(ir);
            }
        }
        else if(getOpcode(ir)>=9 && getOpcode(ir)<=12){
            stalled=1;
            //STALL CASO SALTO CONDICIONAL
        }
        else if(getOpcode(ir)==16){
            stalled = 1;
            //STALL CASO EXIT
        }
        else{
            pc = pc+4;
        }
    }
    else if(ir==1073741824){
        // Retorno 0 para a função indica
        // que o exit já foi encontrado, ou seja,
        // o programa já buscou todas as instruções
        // que precisava.
        return 0;
    }
    else if(stalled==2){
        stalled=0;
    }
    else{
        //printf("\nESTA EM STALL");
    }
    return 1;
}

// Função para obter o Opcode de uma instrução
int getOpcode(int instrucao){
    int codOpcode = (instrucao >> 26) & 0x3F;
    return codOpcode;
} 

// Função para obter o Destino de uma instrução
int getDestino(int instrucao){
    int destino;
    if(getOpcode(instrucao)==0 || getOpcode(instrucao)==2 || getOpcode(instrucao)==4 || getOpcode(instrucao)==5 || getOpcode(instrucao)==6 || getOpcode(instrucao)==7 || getOpcode(instrucao)==8){
        destino = (instrucao >> 11) & 0x1F;
    }
    else if(getOpcode(instrucao)==1 || getOpcode(instrucao)==3 || getOpcode(instrucao)==14){
        destino = (instrucao >> 16) & 0x1F;
    }
    else if(getOpcode(instrucao)==15){
        destino = instrucao & 0xFFFF;
        int bit_15 = (instrucao >> 15) & 1;
        if (bit_15 != 0) {
            destino = -(destino); // Estende sinal para valores negativos
        }
    }
    else if(getOpcode(instrucao)==9 || getOpcode(instrucao)==10 || getOpcode(instrucao)==11 || getOpcode(instrucao)==12 || getOpcode(instrucao)==13){
        destino = (instrucao >> 0) & 0x7FFF;
        int bit_15 = (instrucao >> 15) & 1;
        if (bit_15 != 0) {
            destino = -(destino); // Estende sinal para valores negativos
        }
    }
    return destino;    
} 

// Função que retorna a Fonte1 de uma instrução
int getFonte1(int instrucao){
    int fonte1;
    if(getOpcode(instrucao)!=13 && getOpcode(instrucao)!=14 && getOpcode(instrucao)!=15 && getOpcode(instrucao)!=16)
        fonte1 = (instrucao >> 21) & 0x1F;
    else if(getOpcode(instrucao)==14){
        fonte1 = instrucao & 0xFFFF;
    }
    else if(getOpcode(instrucao)==15){
        fonte1 = (instrucao >> 21) & 0x1F;
    }
    return fonte1;    
} 

// Função que retorna a Fonte1 de uma instrução
int getFonte2(int instrucao){
    int fonte2;
    if(getOpcode(instrucao)!=1 && getOpcode(instrucao)!=3 && getOpcode(instrucao)!=8 && getOpcode(instrucao)!=13 && getOpcode(instrucao)!=14 && getOpcode(instrucao)!=15)
        fonte2 = (instrucao >> 16) & 0x1F;
    else if(getOpcode(instrucao)==14){
        fonte2 = (instrucao >> 21) & 0x1F;
    }
    else if(getOpcode(instrucao)==15){
        fonte2 = (instrucao >> 16) & 0x1F;
    }
    else if(getOpcode(instrucao)==1 || getOpcode(instrucao)==3){
        fonte2 = (instrucao >> 0) & 0xFFFF;
    }
    //printf("\nFONTE2: %d", fonte2);
    return fonte2;    
}

// Função que retorna o imm de uma instrução
int getImm(int instrucao){
    int imm;
    imm = instrucao & 0x1FFFFFF;
    return imm;
}

// Função que retorna a quantidade de ciclos de uma instrução
// de acordo com seu Opcode
int getCiclos(int opcode){
    int ciclos;
    if(opcode==0){
        ciclos = ciclosParaExecutar[0];
    }
    else if(opcode==1){
        ciclos = ciclosParaExecutar[1];
    }
    else if(opcode==2){
        ciclos = ciclosParaExecutar[2];
    }
    else if(opcode==3){
        ciclos = ciclosParaExecutar[3];
    }
    else if(opcode==4){
        ciclos = ciclosParaExecutar[4];
    }
    else if(opcode==5){
        ciclos = ciclosParaExecutar[5];
    }
    else if(opcode==6){
        ciclos = ciclosParaExecutar[6];
    }
    else if(opcode==7){
        ciclos = ciclosParaExecutar[7];
    }
    else if(opcode==8){
        ciclos = ciclosParaExecutar[8];
    }
    else if(opcode==9){
        ciclos = ciclosParaExecutar[9];
    }
    else if(opcode==10){
        ciclos = ciclosParaExecutar[10];
    }
    else if(opcode==11){
        ciclos = ciclosParaExecutar[11];
    }
    else if(opcode==12){
        ciclos = ciclosParaExecutar[12];
    }
    else if(opcode==13){
        ciclos = ciclosParaExecutar[13];
    }
    else if(opcode==14){
        ciclos = ciclosParaExecutar[14];
    }
    else if(opcode==15){
        ciclos = ciclosParaExecutar[15];
    }
    else{
        return -1;
    }
    return ciclos;
}

// Executa a instrução de fato após os ciclos para
// executar chegarem a 0.
int executaInstrucao(int destino, int fonte1, int fonte2, int opcode){
    int resultado=0;
    if(opcode==0){
        resultado = destino << 26;
        int valor = fonte1 + fonte2;
        if(valor!= abs(valor)){
        	int sinal = 1;
        	sinal = sinal << 25;
        	resultado = resultado | sinal;
		}	
        valor = abs(valor) << 0;
        resultado = resultado | valor;
    }
    else if(opcode==1){
        resultado = destino << 26;
        int valor = fonte1 + fonte2;
        if(valor!= abs(valor)){
        	int sinal = 1;
        	sinal = sinal << 25;
        	resultado = resultado | sinal;
		}	
        valor = abs(valor) << 0;
        resultado = resultado | valor;
    }
    else if(opcode==2){
       resultado = destino << 26;
        int valor = fonte1 - fonte2;
        if(valor!= abs(valor)){
        	int sinal = 1;
        	sinal = sinal << 25;
        	resultado = resultado | sinal;
		}	
        valor = abs(valor) << 0;
        resultado = resultado | valor;
    }
    else if(opcode==3){
        resultado = destino << 26;
        int valor = fonte1 - fonte2;
        if(valor!= abs(valor)){
        	int sinal = 1;
        	sinal = sinal << 25;
        	resultado = resultado | sinal;
		}	
        valor = abs(valor) << 0;
        resultado = resultado | valor;
    }
    else if(opcode==4){
        resultado = destino << 26;
        int valor = fonte1 * fonte2;
        if(valor!= abs(valor)){
        	int sinal = 1;
        	sinal = sinal << 25;
        	resultado = resultado | sinal;
		}	
        valor = abs(valor) << 0;
        resultado = resultado | valor;
    }
    else if(opcode==5){
        resultado = destino << 26;
        int valor;
        if(fonte2==0){
            printf("\nNÃO É POSSÍVEL FAZER DIVISÃO POR ZERO.");
            resultado = 0;
        }
        else{
            valor = fonte1 / fonte2;
            if(valor!= abs(valor)){
                int sinal = 1;
                sinal = sinal << 25;
                resultado = resultado | sinal;
            }	
            valor = abs(valor) << 0;
            resultado = resultado | valor;
        }
    }
    else if(opcode==6){
        resultado = destino << 26;
        int valor = fonte1 & fonte2;
        if(valor!= abs(valor)){
        	int sinal = 1;
        	sinal = sinal << 25;
        	resultado = resultado | sinal;
		}	
        valor = abs(valor) << 0;
        resultado = resultado | valor;
    }
    else if(opcode==7){
        resultado = destino << 26;
        int valor = fonte1 | fonte2;
        if(valor!= abs(valor)){
        	int sinal = 1;
        	sinal = sinal << 25;
        	resultado = resultado | sinal;
		}	
        valor = abs(valor) << 0;
        resultado = resultado | valor;
    }
    else if(opcode==8){
        resultado = destino << 26;
        int valor = ~fonte1;
        if(valor!= abs(valor)){
        	int sinal = 1;
        	sinal = sinal << 25;
        	resultado = resultado | sinal;
		}	
        valor = abs(valor) << 0;
        resultado = resultado | valor;
    }
    else if(opcode==9){
        resultado = 32 << 26;
        int valor=0;
        if(fonte1<fonte2){
            valor = pc + 4 + destino;
        }
        else{
            valor = pc + 4;
        }

        if(valor%4!=0 || valor<100){
            valor=pc+4;
        }

        resultado = resultado | valor; 
    }
    else if(opcode==10){
        resultado = 32 << 26;
        int valor=0;
        if(fonte1>fonte2){
            valor = pc + 4 + destino;
        }
        else{
            valor = pc + 4;
        }
        if(valor%4!=0 || valor<100){
            valor=pc+4;
        }
        resultado = resultado | valor; 
    }
    else if(opcode==11){
        resultado = 32 << 26;
        int valor=0;
        if(fonte1==fonte2){
            valor = pc + 4 + destino;
        }
        else{
            valor = pc + 4;
        }

        if(valor%4!=0 || valor<100){
            valor=pc+4;
        }
        resultado = resultado | valor; 
    }
    else if(opcode==12){
        resultado = 32 << 26;
        int valor=0;
        if(fonte1!=fonte2){
            valor = pc + 4 + destino;
            if(valor!= abs(valor)){
                int sinal = 1 << 25;
                resultado = resultado | sinal;
            }	 
        }
        else{
            valor = pc + 4;
        }

        if(valor%4!=0 || valor<100){
            valor=pc+4;
        }
        resultado = resultado | valor; 
    }
    else if(opcode==14){
        destino = destino << 26;
        resultado = resultado | destino;
        int valor = pegaMemoriaLw(fonte1, fonte2);
        if(valor!= abs(valor)){
            int sinal = 1;
            sinal = sinal << 25;
            resultado = resultado | sinal;
        }	
        valor = abs(valor) << 0;
        resultado = resultado | valor;
    }
    else if(opcode==15){
        resultado = 33 << 26;
        int valor = fonte2;
        int endereco = fonte1 + destino;
        endereco = endereco << 16;
        resultado = resultado | endereco;

        if(valor!= abs(valor)){
            int sinal = 1;
            sinal = sinal << 15;
            resultado = resultado | sinal;
        }	
        valor = abs(valor) << 0;
        resultado = resultado | valor;
    }
    return resultado;
}

// Escreve no destino o valor obtido pela execução de uma instrução
// Funciona da seguinte forma:
// bits_destino representa qual será o destino do resultado.
// De 0 a 31, o resultado será enviado para o registrador[bits_destino]
// Em 32, o resultado será enviado para o PC.
// Em 33, o resultado será enviado para a memória.
// Para este caso, o endereço da memória será armazenado dos bits 16 a 25,
// e o valor a ser escrito estará no restante dos bits.
void escreveNoDestino(int resultado){

    int bits_destino = (resultado >> 26) & 0x3F;
    int resto = resultado & 0x1FFFFFF;
    int bit_25 = (resultado >> 25) & 1;

    if (bits_destino>=0 && bits_destino<=31){
        if(bit_25 == 1){
            if(bits_destino == 0 && resto != 0){
                printf("\nO registrador r0 não pode receber valor diferente de 0!");
            }
            else{
                bancoRegs[bits_destino]=-resto;
            }
        }
        else{
            if(bits_destino == 0 && resto != 0){
                printf("\nO registrador r0 não pode receber valor diferente de 0!");
            }
            else{
                bancoRegs[bits_destino]=resto;
            }
        }
    }
    else if(bits_destino==32){
        pc=resto;
        stalled=2;
    }
    else if(bits_destino==33){
        
        int endereco = (resultado >> 16) & 0x3FF;
        
        int valor = resultado & 0x7FFF;  
        int bit_15 = (resultado >> 15) & 1;

        if(endereco%4!=0 && endereco>399){
            printf("ENDEREÇO DE MEMORIA INVALIDO.\n");
        }
        else{
            if(bit_15 == 1){
                colocaMemoria(-valor, endereco);
            }
            else{
                colocaMemoria(valor, endereco);
            }
        }
        
    }


}


// Realiza a emissão de uma instrução seguindo o algoritmo do Bookkeeping.
void emiteInstrucao(){
    if(ir!=0 && ir!=1073741824){
        int destino = getDestino(ir);
        int tipoUF_inst = getTipoUF(ir);
        int disponivel = getUFdisponivel(tipoUF_inst);
  
        if(getOpcode(ir)==13){
            // Instruções de salto incondicional
            // não entram na emissão do pipeline
        }
        else if(vetorResultados[destino] == NULL && disponivel!=-1 && (getOpcode(ir)<9 || getOpcode(ir)>13)){
            // Emissão para uma unidade funcional do tipo ADD:
            // 
            if(tipoUF_inst==0){
                int indice_instrucao = getIndiceInstrucao(ir);
                if(clocki-statusI[indice_instrucao].busca==1 || vetorForwarding[indice_instrucao]==1){
                    unidadesFuncionais.ufAdd[disponivel].instrucao=ir;
                    unidadesFuncionais.ufAdd[disponivel].busy = 1;
                    unidadesFuncionais.ufAdd[disponivel].fi = destino;
                    unidadesFuncionais.ufAdd[disponivel].fj = getFonte1(ir);
                    unidadesFuncionais.ufAdd[disponivel].fk = getFonte2(ir);
                    unidadesFuncionais.ufAdd[disponivel].operacao = getOpcode(ir);
                    if(unidadesFuncionais.ufAdd[disponivel].operacao==1 || unidadesFuncionais.ufAdd[disponivel].operacao==3){
                        unidadesFuncionais.ufAdd[disponivel].qk = NULL;
                    }
                    else{
                        unidadesFuncionais.ufAdd[disponivel].qk = vetorResultados[getFonte2(ir)];

                    }
                    if(vetorResultados[getFonte1(ir)]==0){
                        unidadesFuncionais.ufAdd[disponivel].qj = NULL;
                    }
                    else{
                        unidadesFuncionais.ufAdd[disponivel].qj = vetorResultados[getFonte1(ir)];
                    }
                    //unidadesFuncionais.ufAdd[disponivel].qj = vetorResultados[getFonte1(ir)];
                    unidadesFuncionais.ufAdd[disponivel].rj = (unidadesFuncionais.ufAdd[disponivel].qj == NULL);
                    unidadesFuncionais.ufAdd[disponivel].rk = (unidadesFuncionais.ufAdd[disponivel].qk == NULL);
                    unidadesFuncionais.ufAdd[disponivel].qtde_ciclos = -1;

                    vetorResultados[destino] = &unidadesFuncionais.ufAdd[disponivel];
                    

                    //Não teve RAW e pode ler nesse ciclo
                    statusI[indice_instrucao].emissao = clocki;
                    vetorForwarding[indice_instrucao]=0;
                    stalled=0;
                    instrucoesEmitidas++;
                }
                else{
                    vetorForwarding[indice_instrucao]=1;
                }
            }
            else if(tipoUF_inst==1){
                int indice_instrucao = getIndiceInstrucao(ir);
            
                if(clocki-statusI[indice_instrucao].busca==1 || vetorForwarding[indice_instrucao]==1){
                    unidadesFuncionais.ufMul[disponivel].instrucao=ir;
                    unidadesFuncionais.ufMul[disponivel].busy = 1;
                    unidadesFuncionais.ufMul[disponivel].fi = destino;
                    unidadesFuncionais.ufMul[disponivel].fj = getFonte1(ir);
                    unidadesFuncionais.ufMul[disponivel].fk = getFonte2(ir);
                    unidadesFuncionais.ufMul[disponivel].operacao = getOpcode(ir);
                    if(vetorResultados[getFonte1(ir)]==0){
                        unidadesFuncionais.ufMul[disponivel].qj = NULL;
                    }
                    else{
                        unidadesFuncionais.ufMul[disponivel].qj = vetorResultados[getFonte1(ir)];
                    }
                    if(vetorResultados[getFonte2(ir)]==0){
                        unidadesFuncionais.ufMul[disponivel].qk = NULL;
                    }
                    else{
                        unidadesFuncionais.ufMul[disponivel].qk = vetorResultados[getFonte2(ir)];
                    }
                    unidadesFuncionais.ufMul[disponivel].rj = (unidadesFuncionais.ufMul[disponivel].qj == NULL);
                    unidadesFuncionais.ufMul[disponivel].rk = (unidadesFuncionais.ufMul[disponivel].qk == NULL);
                    unidadesFuncionais.ufMul[disponivel].qtde_ciclos = -1;
                    vetorResultados[destino] = &unidadesFuncionais.ufMul[disponivel];
                    //Não teve RAW e pode ler nesse ciclo
                    statusI[indice_instrucao].emissao = clocki;
                    vetorForwarding[indice_instrucao]=0;
                    stalled=0;
                    instrucoesEmitidas++;
                }
                else{
                    vetorForwarding[indice_instrucao]=1;
                }
            }
            else if(tipoUF_inst==2){
                int indice_instrucao = getIndiceInstrucao(ir);
                
                if(clocki-statusI[indice_instrucao].busca==1 || vetorForwarding[indice_instrucao]==1){
                    unidadesFuncionais.ufInt[disponivel].instrucao=ir;
                    unidadesFuncionais.ufInt[disponivel].busy = 1;
                    unidadesFuncionais.ufInt[disponivel].fi = destino;
                    unidadesFuncionais.ufInt[disponivel].fj = getFonte1(ir);
                    unidadesFuncionais.ufInt[disponivel].fk = getFonte2(ir);
                    unidadesFuncionais.ufInt[disponivel].operacao = getOpcode(ir);
                    if(unidadesFuncionais.ufInt[disponivel].operacao!=15){
                        vetorResultados[destino] = &unidadesFuncionais.ufInt[disponivel];
                        if(unidadesFuncionais.ufInt[disponivel].operacao!=14){
                            if(vetorResultados[getFonte1(ir)]==0){
                                unidadesFuncionais.ufInt[disponivel].qj = NULL;
                            }
                            else{
                                unidadesFuncionais.ufInt[disponivel].qj = vetorResultados[getFonte1(ir)];
                            }
                        }
                        else{
                            unidadesFuncionais.ufInt[disponivel].qj = NULL;
                        }
                        if(vetorResultados[getFonte2(ir)]==0){
                            unidadesFuncionais.ufInt[disponivel].qk = NULL;
                        }
                        else{
                            unidadesFuncionais.ufInt[disponivel].qk = vetorResultados[getFonte2(ir)];
                        }
                    }
                    else{
                        if(destino<0 || destino>31){
                            vetorResultados[destino] = NULL;
                        }
                        if(vetorResultados[getFonte1(ir)]==0){
                            unidadesFuncionais.ufInt[disponivel].qj = NULL;
                        }
                        else{
                            unidadesFuncionais.ufInt[disponivel].qj = vetorResultados[getFonte1(ir)];
                        }
                        if(vetorResultados[getFonte2(ir)]==0){
                            unidadesFuncionais.ufInt[disponivel].qk = NULL;
                        }
                        else{
                            unidadesFuncionais.ufInt[disponivel].qk = vetorResultados[getFonte2(ir)];
                        }
                    }
                    unidadesFuncionais.ufInt[disponivel].rj = (unidadesFuncionais.ufInt[disponivel].qj == NULL);
                    unidadesFuncionais.ufInt[disponivel].rk = (unidadesFuncionais.ufInt[disponivel].qk == NULL);
                    unidadesFuncionais.ufInt[disponivel].qtde_ciclos = -1;
                    statusI[indice_instrucao].emissao = clocki;
                    vetorForwarding[indice_instrucao]=0;
                    stalled=0;
                    instrucoesEmitidas++;
                }
                else{
                    vetorForwarding[indice_instrucao]=1;
                }
            }
            //Esse caso acontece quando houve um stall porque uma instrução não conseguiu ser emitida.
            //Ou seja, é o caso em que houve WAW ou não UF livre. Mas como agora a instrução está sendo emitida, esse stall tem que ser removido.  
        }
        else if(disponivel!=-1 && getOpcode(ir)>=9 && getOpcode(ir)<=12){
            int indice_instrucao = getIndiceInstrucao(ir);
            if(statusI[indice_instrucao].emissao==0 && statusI[indice_instrucao].busca!=0){
                unidadesFuncionais.ufInt[disponivel].instrucao=ir;
                unidadesFuncionais.ufInt[disponivel].busy = 1;
                unidadesFuncionais.ufInt[disponivel].fi = destino;
                unidadesFuncionais.ufInt[disponivel].fj = getFonte1(ir);
                unidadesFuncionais.ufInt[disponivel].fk = getFonte2(ir);
                unidadesFuncionais.ufInt[disponivel].operacao = getOpcode(ir);
                if(vetorResultados[getFonte1(ir)]==0){
                    unidadesFuncionais.ufInt[disponivel].qj = NULL;
                }
                else{
                    unidadesFuncionais.ufInt[disponivel].qj = vetorResultados[getFonte1(ir)];
                }
                if(vetorResultados[getFonte2(ir)]==0){
                    unidadesFuncionais.ufInt[disponivel].qk = NULL; 
                }
                else{
                    unidadesFuncionais.ufInt[disponivel].qk = vetorResultados[getFonte2(ir)]; 
                }
                unidadesFuncionais.ufInt[disponivel].rj = (unidadesFuncionais.ufInt[disponivel].qj == NULL);
                unidadesFuncionais.ufInt[disponivel].rk = (unidadesFuncionais.ufInt[disponivel].qk == NULL);
                unidadesFuncionais.ufInt[disponivel].qtde_ciclos = -1;
                statusI[getIndiceInstrucao(ir)].emissao = clocki;
                instrucoesEmitidas++;
            }
        }
        else{
            stalled = 1;
        }
    }
}

// Realiza o estágio de Leitura de Operandos do Pipeline, seguindo também o Bookkeeping.
void leituraDeOperandos(){
    for(int i=0; i<unidadesFuncionais.qtdeADD; i++){
        if(unidadesFuncionais.ufAdd[i].rj == 1 && unidadesFuncionais.ufAdd[i].rk == 1){
            int indice_instrucao = getIndiceInstrucaoLO(unidadesFuncionais.ufAdd[i].instrucao);
            if(vetorForwarding[indice_instrucao]==0 || vetorForwarding[indice_instrucao]>1){
                //Não teve RAW e pode ler nesse ciclo
                unidadesFuncionais.ufAdd[i].rj = 0;
                unidadesFuncionais.ufAdd[i].rk = 0;
                unidadesFuncionais.ufAdd[i].valorfj = bancoRegs[unidadesFuncionais.ufAdd[i].fj];
                if(unidadesFuncionais.ufAdd[i].operacao == 1 || unidadesFuncionais.ufAdd[i].operacao == 3){
                    unidadesFuncionais.ufAdd[i].valorfk = unidadesFuncionais.ufAdd[i].fk;
                }
                else{
                    unidadesFuncionais.ufAdd[i].valorfk = bancoRegs[unidadesFuncionais.ufAdd[i].fk];
                }
                unidadesFuncionais.ufAdd[i].qtde_ciclos = getCiclos(unidadesFuncionais.ufAdd[i].operacao);
                statusI[indice_instrucao].leitura_op = clocki;
            }
            else{
                vetorForwarding[indice_instrucao]=0;
                //Teve RAW e pode ler só no próximo ciclo pq não tem forwarding. 
            }
        }
    }
    for(int i=0; i<unidadesFuncionais.qtdeMUL; i++){
        if(unidadesFuncionais.ufMul[i].rj == 1 && unidadesFuncionais.ufMul[i].rk == 1){
            int indice_instrucao = getIndiceInstrucaoLO(unidadesFuncionais.ufMul[i].instrucao);
            if(vetorForwarding[indice_instrucao]==0 || vetorForwarding[indice_instrucao]>1){
                //Não teve RAW e pode ler nesse ciclo
                unidadesFuncionais.ufMul[i].rj = 0;
                unidadesFuncionais.ufMul[i].rk = 0;
                unidadesFuncionais.ufMul[i].valorfj = bancoRegs[unidadesFuncionais.ufMul[i].fj];
                unidadesFuncionais.ufMul[i].valorfk = bancoRegs[unidadesFuncionais.ufMul[i].fk];
                unidadesFuncionais.ufMul[i].qtde_ciclos = getCiclos(unidadesFuncionais.ufMul[i].operacao);
                //int indice_instrucao = getIndiceInstrucaoLO(unidadesFuncionais.ufMul[i].instrucao);
                statusI[indice_instrucao].leitura_op = clocki;
            }
            else{
                vetorForwarding[indice_instrucao]=0;
                //Teve RAW e pode ler só no próximo ciclo pq não tem forwarding. 
            }
        }
    }
    for(int i=0; i<unidadesFuncionais.qtdeINT; i++){
        if(unidadesFuncionais.ufInt[i].rj == 1 && unidadesFuncionais.ufInt[i].rk == 1){
            int indice_instrucao = getIndiceInstrucaoLO(unidadesFuncionais.ufInt[i].instrucao);
            if(vetorForwarding[indice_instrucao]==0 || vetorForwarding[indice_instrucao]>1){
                //Não teve RAW e pode ler nesse ciclo
                unidadesFuncionais.ufInt[i].rj = 0;
                unidadesFuncionais.ufInt[i].rk = 0;
                if(unidadesFuncionais.ufInt[i].operacao==14){
                    unidadesFuncionais.ufInt[i].valorfj = unidadesFuncionais.ufInt[i].fj;
                }
                else{
                    unidadesFuncionais.ufInt[i].valorfj = bancoRegs[unidadesFuncionais.ufInt[i].fj];
                }
                unidadesFuncionais.ufInt[i].valorfk = bancoRegs[unidadesFuncionais.ufInt[i].fk];
                unidadesFuncionais.ufInt[i].qtde_ciclos = getCiclos(unidadesFuncionais.ufInt[i].operacao);
                statusI[indice_instrucao].leitura_op = clocki;
            }
            else{
                vetorForwarding[indice_instrucao]=0;
                //Teve RAW e pode ler só no próximo ciclo pq não tem forwarding. 
            }
        }
    }
}

// Fase de execução do Pipeline
void execucao(){
    for(int i=0; i<unidadesFuncionais.qtdeADD; i++){
        if(unidadesFuncionais.ufAdd[i].qtde_ciclos!=0 && unidadesFuncionais.ufAdd[i].qtde_ciclos!=-1){
            unidadesFuncionais.ufAdd[i].qtde_ciclos--;
            statusI[getIndiceInstrucaoEX(unidadesFuncionais.ufAdd[i].instrucao)].execucaofim=clocki;
        }
        if(unidadesFuncionais.ufAdd[i].qtde_ciclos==0 && (unidadesFuncionais.ufAdd[i].fi!=0)){
            int resultado = executaInstrucao(unidadesFuncionais.ufAdd[i].fi, unidadesFuncionais.ufAdd[i].valorfj, unidadesFuncionais.ufAdd[i].valorfk, unidadesFuncionais.ufAdd[i].operacao);
            colocaBarramentoResultados(resultado);
            printBarramentoResultados();
        }
    }
    for(int i=0; i<unidadesFuncionais.qtdeMUL; i++){
        if(unidadesFuncionais.ufMul[i].qtde_ciclos!=0 && unidadesFuncionais.ufMul[i].qtde_ciclos!=-1){
            unidadesFuncionais.ufMul[i].qtde_ciclos--;
            statusI[getIndiceInstrucaoEX(unidadesFuncionais.ufMul[i].instrucao)].execucaofim=clocki;
        }
        if(unidadesFuncionais.ufMul[i].qtde_ciclos==0 && (unidadesFuncionais.ufMul[i].fi!=0)){
            int resultado = executaInstrucao(unidadesFuncionais.ufMul[i].fi, unidadesFuncionais.ufMul[i].valorfj, unidadesFuncionais.ufMul[i].valorfk, unidadesFuncionais.ufMul[i].operacao);
            colocaBarramentoResultados(resultado);
            printBarramentoResultados();
        }
    }
    for(int i=0; i<unidadesFuncionais.qtdeINT; i++){
        if(unidadesFuncionais.ufInt[i].qtde_ciclos!=0 && unidadesFuncionais.ufInt[i].qtde_ciclos!=-1){
            unidadesFuncionais.ufInt[i].qtde_ciclos--;
            statusI[getIndiceInstrucaoEX(unidadesFuncionais.ufInt[i].instrucao)].execucaofim=clocki;
        }
        if(unidadesFuncionais.ufInt[i].qtde_ciclos==0 && (unidadesFuncionais.ufInt[i].fi!=0 || unidadesFuncionais.ufInt[i].operacao==15 ||
        (getOpcode(unidadesFuncionais.ufInt[i].instrucao)<=12 && getOpcode(unidadesFuncionais.ufInt[i].instrucao)>=9))){
            int resultado = executaInstrucao(unidadesFuncionais.ufInt[i].fi, unidadesFuncionais.ufInt[i].valorfj, unidadesFuncionais.ufInt[i].valorfk, unidadesFuncionais.ufInt[i].operacao);
            colocaBarramentoResultados(resultado);
            printBarramentoResultados();
        }
    }

}


// Fase de Escrita de Resultados do Pipeline, seguindo o Bookkeeping.
void escritaResultados(){
    int checkAddA=1, checkAddB=1, checkAddC=1;
    int checkIntA=1, checkIntB=1, checkIntC=1;
    int checkMulA=1, checkMulB=1, checkMulC=1;
    int linhaBarramento;
    for(int i=0; i<unidadesFuncionais.qtdeADD; i++){
        if(unidadesFuncionais.ufAdd[i].qtde_ciclos == 0){
            for(int j=0; j<unidadesFuncionais.qtdeMUL; j++){
                if((unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufMul[j].fj || unidadesFuncionais.ufMul[j].rj==0)
                && (unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufMul[j].fk || unidadesFuncionais.ufMul[j].rk==0)){
                }
                else{
                    checkAddA = 0;
                }
            }
            for(int j=0; j<unidadesFuncionais.qtdeINT; j++){
                if(((unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufInt[j].fj || unidadesFuncionais.ufInt[j].operacao==14) || unidadesFuncionais.ufInt[j].rj==0)
                && (unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufInt[j].fk || unidadesFuncionais.ufInt[j].rk==0)){
                }
                else{
                    checkAddB = 0;
                }
            }
            for(int j=0; j<unidadesFuncionais.qtdeADD; j++){
                if((unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufAdd[j].fj || unidadesFuncionais.ufAdd[j].rj==0)
                && ((unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufAdd[j].fk || unidadesFuncionais.ufAdd[j].operacao==1 || unidadesFuncionais.ufAdd[j].operacao==3) || unidadesFuncionais.ufAdd[j].rk==0)){
                }
                else{
                    checkAddC = 0;
                }
            }

            linhaBarramento = pegaBarramentoResultados(executaInstrucao(unidadesFuncionais.ufAdd[i].fi, unidadesFuncionais.ufAdd[i].valorfj, unidadesFuncionais.ufAdd[i].valorfk, unidadesFuncionais.ufAdd[i].operacao));

            if(checkAddA && checkAddB && checkAddC && unidadesFuncionais.ufAdd[i].qtde_ciclos == 0 && unidadesFuncionais.ufAdd[i].fi != 0 && linhaBarramento!=-1){
                for(int j=0; j<unidadesFuncionais.qtdeADD; j++){
                    if((unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufAdd[j].fj || unidadesFuncionais.ufAdd[j].rj==0)
                    && (unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufAdd[j].fk || unidadesFuncionais.ufAdd[j].rk==0)){
                        if(i!=j){
                            if(unidadesFuncionais.ufAdd[j].qj==&unidadesFuncionais.ufAdd[i]){
                                unidadesFuncionais.ufAdd[j].rj = 1;
                                unidadesFuncionais.ufAdd[j].qj = NULL;
                                vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufAdd[j].instrucao)]=1;
                            }
                            if(unidadesFuncionais.ufAdd[j].qk==&unidadesFuncionais.ufAdd[i]){
                                unidadesFuncionais.ufAdd[j].rk = 1;
                                unidadesFuncionais.ufAdd[j].qk = NULL;
                                vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufAdd[j].instrucao)]=1;
                            }
                        }
                    }
                }
                for(int j=0; j<unidadesFuncionais.qtdeMUL; j++){
                    if((unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufMul[j].fj || unidadesFuncionais.ufMul[j].rj==0)
                    && (unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufMul[j].fk || unidadesFuncionais.ufMul[j].rk==0)){
                        if(unidadesFuncionais.ufMul[j].qj==&unidadesFuncionais.ufAdd[i]){
                            unidadesFuncionais.ufMul[j].rj = 1;
                            unidadesFuncionais.ufMul[j].qj = NULL;
                            vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufMul[j].instrucao)]=1;
                        }
                        if(unidadesFuncionais.ufMul[j].qk==&unidadesFuncionais.ufAdd[i]){
                            unidadesFuncionais.ufMul[j].rk = 1;
                            unidadesFuncionais.ufMul[j].qk = NULL;
                            vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufMul[j].instrucao)]=1;
                        }
                    }
                }
                for(int j=0; j<unidadesFuncionais.qtdeINT; j++){
                    if((unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufInt[j].fj || unidadesFuncionais.ufInt[j].rj==0)
                    && (unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufInt[j].fk || unidadesFuncionais.ufInt[j].rk==0)){
                        if(unidadesFuncionais.ufInt[j].qj==&unidadesFuncionais.ufAdd[i]){
                            unidadesFuncionais.ufInt[j].rj = 1;
                            unidadesFuncionais.ufInt[j].qj = NULL;
                            if(unidadesFuncionais.ufInt[j].operacao==14 && unidadesFuncionais.ufInt[j].operacao!=15){
                                vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufInt[j].instrucao)]=0;
                            }
                            else{
                                vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufInt[j].instrucao)]=1;
                            }
                        }
                        if(unidadesFuncionais.ufInt[j].qk==&unidadesFuncionais.ufAdd[i]){
                            unidadesFuncionais.ufInt[j].rk = 1;
                            unidadesFuncionais.ufInt[j].qk = NULL;
                            vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufInt[j].instrucao)]=1;
                        }
                    }
                }
                vetorResultados[unidadesFuncionais.ufAdd[i].fi] = 0;
                escreveNoDestino(barramentoResultados[linhaBarramento]);
                statusI[getIndiceInstrucaoER(unidadesFuncionais.ufAdd[i].instrucao)].escrita=clocki;
                resetaUF(&unidadesFuncionais.ufAdd[i]);
                instrucoesEfetivadas++;
            }
        }
        checkAddA = 1;
        checkAddB = 1;
        checkAddC = 1;
    }
    for(int i=0; i<unidadesFuncionais.qtdeMUL; i++){
        if(unidadesFuncionais.ufMul[i].qtde_ciclos == 0){
            for(int j=0; j<unidadesFuncionais.qtdeADD; j++){
                if((unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufAdd[j].fj || unidadesFuncionais.ufAdd[j].rj==0)
                && ((unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufAdd[j].fk || unidadesFuncionais.ufAdd[j].operacao==1 || unidadesFuncionais.ufAdd[j].operacao==3) || unidadesFuncionais.ufAdd[j].rk==0)){
                }
                else{
                    checkMulA = 0;
                }
            }
            for(int j=0; j<unidadesFuncionais.qtdeINT; j++){
                if((unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufInt[j].fj || unidadesFuncionais.ufInt[j].rj==0)
                && (unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufInt[j].fk || unidadesFuncionais.ufInt[j].rk==0)){
                }
                else{
                    checkMulB = 0;
                }
            }
            for(int j=0; j<unidadesFuncionais.qtdeMUL; j++){
                if((unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufMul[j].fj || unidadesFuncionais.ufMul[j].rj==0)
                && (unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufMul[j].fk || unidadesFuncionais.ufMul[j].rk==0)){
                }
                else{
                    checkMulC = 0;
                }
            }
        }

        linhaBarramento = pegaBarramentoResultados(executaInstrucao(unidadesFuncionais.ufMul[i].fi, unidadesFuncionais.ufMul[i].valorfj, unidadesFuncionais.ufMul[i].valorfk, unidadesFuncionais.ufMul[i].operacao));

        if(checkMulA && checkMulB && checkMulC && unidadesFuncionais.ufMul[i].qtde_ciclos == 0 &&
        (unidadesFuncionais.ufMul[i].fi != 0) && linhaBarramento!=-1){
            for(int j=0; j<unidadesFuncionais.qtdeMUL; j++){
                if((unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufMul[j].fj || unidadesFuncionais.ufMul[j].rj==0)
                && (unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufMul[j].fk || unidadesFuncionais.ufMul[j].rk==0)){
                    if(i!=j){
                        if(unidadesFuncionais.ufMul[j].qj==&unidadesFuncionais.ufMul[i]){
                            unidadesFuncionais.ufMul[j].rj = 1;
                            unidadesFuncionais.ufMul[j].qj = NULL;
                            vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufMul[j].instrucao)]=1;
                        }
                        if(unidadesFuncionais.ufMul[j].qk==&unidadesFuncionais.ufMul[i]){
                            unidadesFuncionais.ufMul[j].rk = 1;
                            unidadesFuncionais.ufMul[j].qk = NULL;
                            vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufMul[j].instrucao)]=1;
                        }
                    }
                }
            }
            for(int j=0; j<unidadesFuncionais.qtdeADD; j++){
                if((unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufAdd[j].fj || unidadesFuncionais.ufAdd[j].rj==0)
                && (unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufAdd[j].fk || unidadesFuncionais.ufAdd[j].rk==0)){
                    if(unidadesFuncionais.ufAdd[j].qj==&unidadesFuncionais.ufMul[i]){
                        unidadesFuncionais.ufAdd[j].rj = 1;
                        unidadesFuncionais.ufAdd[j].qj = NULL;
                        vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufAdd[j].instrucao)]=1;
                    }
                    if(unidadesFuncionais.ufAdd[j].qk==&unidadesFuncionais.ufMul[i]){
                        unidadesFuncionais.ufAdd[j].rk = 1;
                        unidadesFuncionais.ufAdd[j].qk = NULL;
                        vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufAdd[j].instrucao)]=1;
                    }
                }
            }
            for(int j=0; j<unidadesFuncionais.qtdeINT; j++){
                if((unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufInt[j].fj || unidadesFuncionais.ufInt[j].rj==0)
                && (unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufInt[j].fk || unidadesFuncionais.ufInt[j].rk==0)){
                    if(unidadesFuncionais.ufInt[j].qj==&unidadesFuncionais.ufMul[i]){
                        unidadesFuncionais.ufInt[j].rj = 1;
                        unidadesFuncionais.ufInt[j].qj = NULL;
                        if(unidadesFuncionais.ufInt[j].operacao!=14 && unidadesFuncionais.ufInt[j].operacao!=15){
                            vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufInt[j].instrucao)]=1;
                        }
                        else{
                            vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufInt[j].instrucao)]=0;
                        }
                    }
                    if(unidadesFuncionais.ufInt[j].qk==&unidadesFuncionais.ufMul[i]){
                        unidadesFuncionais.ufInt[j].rk = 1;
                        unidadesFuncionais.ufInt[j].qk = NULL;
                        vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufInt[j].instrucao)]=1;
                    }
                }
            }
            vetorResultados[unidadesFuncionais.ufMul[i].fi] = 0;
            escreveNoDestino(barramentoResultados[linhaBarramento]);
            statusI[getIndiceInstrucaoER(unidadesFuncionais.ufMul[i].instrucao)].escrita=clocki;
            resetaUF(&unidadesFuncionais.ufMul[i]);
            instrucoesEfetivadas++;
        }
        checkMulA = 1;
        checkMulB = 1;
        checkMulC = 1;
    }
    for(int i=0; i<unidadesFuncionais.qtdeINT; i++){
        if(unidadesFuncionais.ufInt[i].qtde_ciclos==0){
            if((getOpcode(unidadesFuncionais.ufInt[i].instrucao)<9 || getOpcode(unidadesFuncionais.ufInt[i].instrucao)>13) && unidadesFuncionais.ufInt[i].operacao!=15){
                for(int j=0; j<unidadesFuncionais.qtdeADD; j++){
                    if(((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufAdd[j].fj || unidadesFuncionais.ufAdd[j].rj==0)
                    && ((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufAdd[j].fk || unidadesFuncionais.ufAdd[j].operacao==1 || unidadesFuncionais.ufAdd[j].operacao==3) || unidadesFuncionais.ufAdd[j].rk==0))
                    || unidadesFuncionais.ufInt[i].operacao==15){
                    }
                    else{
                        checkIntA = 0;
                    }
                }
                for(int j=0; j<unidadesFuncionais.qtdeMUL; j++){
                    if(((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufMul[j].fj || unidadesFuncionais.ufMul[j].rj==0)
                    && (unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufMul[j].fk || unidadesFuncionais.ufMul[j].rk==0))
                    || unidadesFuncionais.ufInt[i].operacao==15){
                    }
                    else{
                        checkIntB = 0;
                    }
                }
                for(int j=0; j<unidadesFuncionais.qtdeINT; j++){
                    if(((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufInt[j].fj || unidadesFuncionais.ufInt[j].rj==0)
                    && (unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufInt[j].fk || unidadesFuncionais.ufInt[j].rk==0))
                    || unidadesFuncionais.ufInt[i].operacao==15){
                    }
                    else{
                        checkIntC = 0;
                    }
                }
            }
        }
        linhaBarramento = pegaBarramentoResultados(executaInstrucao(unidadesFuncionais.ufInt[i].fi, unidadesFuncionais.ufInt[i].valorfj, unidadesFuncionais.ufInt[i].valorfk, unidadesFuncionais.ufInt[i].operacao));

        if(checkIntA && checkIntB && checkIntC && unidadesFuncionais.ufInt[i].qtde_ciclos == 0 && linhaBarramento!=-1 && (unidadesFuncionais.ufInt[i].fi != 0
        || unidadesFuncionais.ufInt[i].operacao==15 || (unidadesFuncionais.ufInt[i].operacao<=12 && unidadesFuncionais.ufInt[i].operacao>=9))){
            for(int j=0; j<unidadesFuncionais.qtdeINT; j++){
                if((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufInt[j].fj || unidadesFuncionais.ufInt[j].rj==0)
                    && (unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufInt[j].fk || unidadesFuncionais.ufInt[j].rk==0)){
                    if(i!=j){
                        if(unidadesFuncionais.ufInt[j].qj==&unidadesFuncionais.ufInt[i]){
                            unidadesFuncionais.ufInt[j].rj = 1;
                            unidadesFuncionais.ufInt[j].qj = NULL;
                            if(unidadesFuncionais.ufInt[j].operacao!=14 && unidadesFuncionais.ufInt[j].operacao!=15){
                                vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufInt[j].instrucao)]=1;
                            }
                            else{
                                vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufInt[j].instrucao)]=0;
                            }
                        }
                        if(unidadesFuncionais.ufInt[j].qk==&unidadesFuncionais.ufInt[i]){
                            unidadesFuncionais.ufInt[j].rk = 1;
                            unidadesFuncionais.ufInt[j].qk = NULL;
                            vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufInt[j].instrucao)]=1;

                        }     
                    }
                }
            }
            for(int j=0; j<unidadesFuncionais.qtdeADD; j++){
                if((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufAdd[j].fj || unidadesFuncionais.ufAdd[j].rj==0)
                    && (unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufAdd[j].fk || unidadesFuncionais.ufAdd[j].rk==0)){
                    if(unidadesFuncionais.ufAdd[j].qj==&unidadesFuncionais.ufInt[i]){
                        unidadesFuncionais.ufAdd[j].rj = 1;
                        unidadesFuncionais.ufAdd[j].qj = NULL;
                        vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufAdd[j].instrucao)]=1;

                    }
                    if(unidadesFuncionais.ufAdd[j].qk==&unidadesFuncionais.ufInt[i]){
                        unidadesFuncionais.ufAdd[j].rk = 1;
                        unidadesFuncionais.ufAdd[j].qk = NULL;
                        vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufAdd[j].instrucao)]=1;

                    }          
                }
            }
            for(int j=0; j<unidadesFuncionais.qtdeMUL; j++){
                if((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufMul[j].fj || unidadesFuncionais.ufMul[j].rj==0)
                    && (unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufMul[j].fk || unidadesFuncionais.ufMul[j].rk==0)){
                    if(unidadesFuncionais.ufMul[j].qj==&unidadesFuncionais.ufInt[i]){
                        unidadesFuncionais.ufMul[j].rj = 1;
                        unidadesFuncionais.ufMul[j].qj = NULL;
                        vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufMul[j].instrucao)]=1;

                    }
                    if(unidadesFuncionais.ufMul[j].qk==&unidadesFuncionais.ufInt[i]){
                        unidadesFuncionais.ufMul[j].rk = 1;
                        unidadesFuncionais.ufMul[j].qk = NULL;
                        vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufMul[j].instrucao)]=1;

                    }          
                }
            }
            
            if((unidadesFuncionais.ufInt[i].operacao<9 || unidadesFuncionais.ufInt[i].operacao>13) && unidadesFuncionais.ufInt[i].operacao!=15){
                vetorResultados[unidadesFuncionais.ufInt[i].fi] = NULL;
            }
            else{
                if(unidadesFuncionais.ufInt[i].fi!=abs(unidadesFuncionais.ufInt[i].fi))
                    qtdeloops++;
            }
            escreveNoDestino(barramentoResultados[linhaBarramento]);
            statusI[getIndiceInstrucaoER(unidadesFuncionais.ufInt[i].instrucao)].escrita=clocki;
            resetaUF(&unidadesFuncionais.ufInt[i]);
            instrucoesEfetivadas++;
        }
        checkIntA = 1;
        checkIntB = 1;
        checkIntC = 1;
    }
    limpaBarramentoResultados();
}
