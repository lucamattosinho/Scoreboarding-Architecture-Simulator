#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "processor.h"
#include "registradores.h"
#include "unidade_funcional.h"
#include "scoreboarding.h"
#include "main.h"
#include "tradutor.h"
int instrucoesEmitidas = 0;
int instrucoesEfetivadas = 0;
int *vetorForwarding;

void inicializaVetorForwarding(){
    vetorForwarding = (int*)malloc(sizeof(int)*(qtdeInsts));
}

void aumentaVetorForwarding(){

    int *newVetorForwarding = (int*)malloc(sizeof(int)*(instsBuscadas+1));

    if (newVetorForwarding == NULL) {   
        printf("Erro na alocação de memória.\n");
        free(newVetorForwarding); // Liberar memória alocada para o vetor original
    }
    for (int i = 0; i < instsBuscadas-1; i++) {
        newVetorForwarding[i] = vetorForwarding[i];
    }
    
    free(vetorForwarding);
    vetorForwarding = (int*)malloc(sizeof(int)*(instsBuscadas+1));
    for (int i = 0; i < instsBuscadas-1; i++) {
        vetorForwarding[i] = newVetorForwarding[i];
    }
    
    free(newVetorForwarding);
}

int instsBuscadas;
int qtdeloops;

int buscaInstrucao(){
    printf("\n-------------BUSCA--------------\n");
    if(!stalled){

        instsBuscadas++;
        colocaPalavraBarramento();
        ir = pegaBarramento();


        if(instsBuscadas>qtdeInsts-2){
            aumentaStatusInstrucoes();
            aumentaVetorForwarding();
        }
        statusI[instsBuscadas-1].instrucao = ir;
        //printf("LOOPS FEITOS: %d",qtdeloops);
        //printf("\nPOSICAO STATUSI %d", instsBuscadas-1);
        //if(statusI[((pc%100)/4)*(1+qtdeloops)].busca == 0){
        //    statusI[((pc%100)/4)*(1+qtdeloops)].busca = clocki;
        //}
        // if(statusI[instsBuscadas-1].escrita!=0){
        //     statusI[instsBuscadas-1].busca = 0;
        //     statusI[instsBuscadas-1].emissao = 0;
        //     statusI[instsBuscadas-1].leitura_op = 0;
        //     statusI[instsBuscadas-1].execucaofim = 0;
        //     statusI[instsBuscadas-1].escrita = 0;
        // }
        //statusI[instsBuscadas-1].busca = 0;
        statusI[instsBuscadas-1].emissao = 0;
        statusI[instsBuscadas-1].leitura_op = 0;
        statusI[instsBuscadas-1].execucaofim = 0;
        statusI[instsBuscadas-1].escrita = 0;
        //statusI[instsBuscadas-1].emissao = 0;
        //statusI[instsBuscadas-1].leitura_op = 0;
        //statusI[instsBuscadas-1].execucaofim = 0;
        //statusI[instsBuscadas-1].escrita = 0;
        statusI[instsBuscadas-1].busca = clocki;
        /*
        else if(statusI[((pc%100)/4)*(1+qtdeloops)].busca != 0 && statusI[((pc%100)/4)*(1+qtdeloops)].escrita!=0){
            statusI[((pc%100)/4)*(1+qtdeloops)].busca = clocki;
            statusI[((pc%100)/4)*(1+qtdeloops)].emissao = 0;
            statusI[((pc%100)/4)*(1+qtdeloops)].leitura_op = 0;
            statusI[((pc%100)/4)*(1+qtdeloops)].execucaofim = 0;
            statusI[((pc%100)/4)*(1+qtdeloops)].escrita = 0;
        }
        else if(statusI[((pc%100)/4)*(1+qtdeloops)].busca != 0 && statusI[((pc%100)/4)*(1+qtdeloops)].escrita==0){
            statusI[((pc%100)/4)*(1+qtdeloops)].busca = clocki;
        }*/


        //("\nOPCODE: %d", getOpcode(ir));
        printBinario(ir);
        if(getOpcode(ir)==13){
            if(getImm(ir)%4!=0 || getImm(ir)<100){
                //printf("SALTO INCONDICIONAL NÃO TOMADO: ENDEREÇO ERRADO PARA PC\n");
                pc = pc+4;
            }
            else{
                pc = getImm(ir);
            }
        }
        else if(getOpcode(ir)>=9 && getOpcode(ir)<=12){
            stalled=1;
            //printf("\nAPLICOU O STALL");
            //desvio condicional 

        }
        else if(getOpcode(ir)==16){
            stalled = 1;
            //printf("\nAPLICOU O STALL");
        }
        else{
            pc = pc+4;
            //printf("\nBUSCOU: %d", ir);
        }
    }
    else if(ir==1073741824){
        //printf("\nESTA EM STALL POR CAUSA DO EXIT");
        return 0;
    }
    else if(stalled==2){
        stalled=0;
    }
    else{
        //printf("\nESTA EM STALL");
    }
    return 1;
    //printf("\n---------------------\n");
}

int getOpcode(int instrucao){
    int codOpcode = (instrucao >> 26) & 0x3F;
    return codOpcode;
} 

//PARA AS INSTRUCOES DE LOAD E STORE:::: RT É DESTINO, IMM É FONTE1 E R2 É FONTE2

//PARA INSTRUÇÕES DE SALTO O DESTINO FOI UTILIZADO PARA ARMAZENAR O IMM
int getRegistradorDestino(int instrucao){
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
    //printf("\nDESTINO: %d", destino);
    return destino;    
} 

int getRegistradorFonte1(int instrucao){
    int fonte1;
    if(getOpcode(instrucao)!=13 && getOpcode(instrucao)!=14 && getOpcode(instrucao)!=15 && getOpcode(instrucao)!=16)
        fonte1 = (instrucao >> 21) & 0x1F;
    else if(getOpcode(instrucao)==14){
        fonte1 = instrucao & 0xFFFF;
    }
    else if(getOpcode(instrucao)==15){
        fonte1 = (instrucao >> 21) & 0x1F;
    }
    //printf("\nFONTE1: %d", fonte1);
    return fonte1;    
} 

int getRegistradorFonte2(int instrucao){
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

int getImm(int instrucao){
    int imm;
    imm = instrucao & 0x1FFFFFF;
    return imm;
}


//SÓ PRA RETORNAR A QUANTIDADE DE CICLOS QUE A INSTRUCAO VAI LEVAR PRA EXECUTAR
int getCiclos(int opcode){
    int ciclos;
    if(opcode==0){
        ciclos = addCiclos;
    }
    else if(opcode==1){
        ciclos = addiCiclos;
    }
    else if(opcode==2){
        ciclos = subCiclos;
    }
    else if(opcode==3){
        ciclos = subiCiclos;
    }
    else if(opcode==4){
        ciclos = mulCiclos;
    }
    else if(opcode==5){
        ciclos = divCiclos;
    }
    else if(opcode==6){
        ciclos = andCiclos;
    }
    else if(opcode==7){
        ciclos = orCiclos;
    }
    else if(opcode==8){
        ciclos = notCiclos;
    }
    else if(opcode==9){
        ciclos = bltCiclos;
    }
    else if(opcode==10){
        ciclos = bgtCiclos;
    }
    else if(opcode==11){
        ciclos = beqCiclos;
    }
    else if(opcode==12){
        ciclos = bneCiclos;
    }
    else if(opcode==13){
        ciclos = jCiclos;
    }
    else if(opcode==14){
        ciclos = lwCiclos;
    }
    else if(opcode==15){
        ciclos = swCiclos;
    }
    return ciclos;
}

//EXECUTA DE FATO AS OPERACOESSSSSSSS

//PROVAVEL QUE TENHA QUE ARREDONDAR OS VALORES QUANDO É DIV OU MUL PQ O ANDERSON DISSE QUE NAO VAMOS TRABALHAR COM FLOAT

//NAO SEI SE A OPERACAO NOT SÓ ALTERA O SINAL DO NUMERO OU DA TIPO UM COMPLEMENTO (???) DELE

//NOS SALTOS RS E RT SÃO FONTE1 E FONTE2 RESPECTIVAMENTE MAS AINDA PRECISO RESOLVER O IMM
//PQ NAO SEI ONDE ARMAZENO (PROVAVELMENTE NO DESTINO)

//FALTA GERENCIAR O STALL
int executaInstrucao(int destino, int fonte1, int fonte2, int opcode){
    int resultado=0;
    //printf("\nOPCODE: %d, DESTINO: %d, FONTE1: %d, FONTE2: %d", opcode, destino, fonte1, fonte2);
    if(opcode==0){
        //printf("\nESCREVEU r%d = r%d + r%d", destino, fonte1, fonte2);
        //bancoRegs[destino] = bancoRegs[fonte1] + bancoRegs[fonte2];
        resultado = destino << 26;
        int valor = fonte1 + fonte2;
        if(valor!= abs(valor)){
        	int sinal = 1;
        	sinal = sinal << 25;
        	resultado = resultado | sinal;
		}	
        valor = abs(valor) << 0;
        resultado = resultado | valor;
        //printf("\nRESULTADO Add= %d", resultado);
    }
    else if(opcode==1){
        //printf("\nESCREVEU r%d = r%d + %d", destino, fonte1, fonte2);
        //bancoRegs[destino] = bancoRegs[fonte1] + fonte2;
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
        //printf("\nESCREVEU r%d = r%d - r%d", destino, fonte1, fonte2);
       //bancoRegs[destino] = bancoRegs[fonte1] - bancoRegs[fonte2];
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
        //printf("\nESCREVEU r%d = r%d - %d", destino, fonte1, fonte2);
        //bancoRegs[destino] = bancoRegs[fonte1] - fonte2;
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
        //printf("\nESCREVEU r%d = r%d * r%d", destino, fonte1, fonte2);
        //bancoRegs[destino] = bancoRegs[fonte1] * bancoRegs[fonte2];
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
        //printf("\nESCREVEU r%d = r%d / r%d", destino, fonte1, fonte2);
        //bancoRegs[destino] = bancoRegs[fonte1] / bancoRegs[fonte2];
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
        //printf("\nESCREVEU r%d = %d", destino, fonte1 & fonte2);
        //bancoRegs[destino] = bancoRegs[fonte1] & bancoRegs[fonte2];
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
        //printf("\nESCREVEU r%d = r%d | r%d", destino, fonte1, fonte2);
        //bancoRegs[destino] = bancoRegs[fonte1] | bancoRegs[fonte2];
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
        //printf("\nESCREVEU r%d = -r%d", destino, fonte1);
        //bancoRegs[destino] = -1 * bancoRegs[fonte1];
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
            //pc = pc + 4 + destino;
            valor = pc + 4 + destino;
        }
        else{
            valor = pc + 4;
        }

        if(valor%4!=0 || valor<100){
            //printf("SALTO CONDICIONAL NÃO TOMADO: ENDEREÇO ERRADO PARA PC\n");
            valor=pc+4;
        }
        else{
            //printf("SALTO CONDICIONAL TOMADO: ENDEREÇO %d\n",valor);
        } 

        resultado = resultado | valor; 
    }
    else if(opcode==10){
        //printf("\nSALTO: if(%d > %d): pc = %d", bancoRegs[fonte1], bancoRegs[fonte2], pc+destino+4);
        resultado = 32 << 26;
        int valor=0;
        if(fonte1>fonte2){
            //printf("\n\n%d\n\n", destino);
            valor = pc + 4 + destino;
        }
        else{
            valor = pc + 4;
        }

        if(valor%4!=0 || valor<100){
            //printf("SALTO CONDICIONAL NÃO TOMADO: ENDEREÇO ERRADO PARA PC\n");
            valor=pc+4;
        }
        else{
            //printf("ENTROU AQUIIIIIIII\n");
            //printf("SALTO CONDICIONAL TOMADO: ENDEREÇO %d\n",valor);
        } 
        resultado = resultado | valor; 
    }
    else if(opcode==11){
        resultado = 32 << 26;
        int valor=0;
        if(fonte1==fonte2){
            //pc = pc + 4 + destino;
            valor = pc + 4 + destino;
        }
        else{
            valor = pc + 4;
        }

        if(valor%4!=0 || valor<100){
            //printf("SALTO CONDICIONAL NÃO TOMADO: ENDEREÇO ERRADO PARA PC\n");
            valor=pc+4;
        }
        else{
            //printf("SALTO CONDICIONAL TOMADO: ENDEREÇO %d\n",valor);
        } 
        resultado = resultado | valor; 
    }
    else if(opcode==12){
        resultado = 32 << 26;
        int valor=0;
        if(fonte1!=fonte2){
            //pc = pc + 4 + destino;
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
            //printf("SALTO CONDICIONAL NÃO TOMADO: ENDEREÇO ERRADO PARA PC\n");
            valor=pc+4;
        }
        else{
            //printf("SALTO CONDICIONAL TOMADO: ENDEREÇO %d\n",valor);
        } 
        resultado = resultado | valor; 
    }
    //SALTO INCONDICIONAL (OPCODE 13) TEM QUE SER VISTO NA BUSCAAAAAAAAAA
    else if(opcode==14){
        //printf("\nEXECUTOU r%d = memoria[%d + r%d]\n", destino, fonte1, fonte2);
        //bancoRegs[destino] = memoria[fonte1 + bancoRegs[fonte2]];
            destino = destino << 26;
            resultado = resultado | destino;
            int valor = pegaMemoriaLw(fonte1, fonte2);
            //int valor = memoria[fonte1 + fonte2];
            //printf("\nVALOR MEMORIA = %d", valor);
            //printf("ABS = %d", abs(valor));
            if(valor!= abs(valor)){
                //printf("AAAAAAAAAAAA");
                int sinal = 1;
                sinal = sinal << 25;
                resultado = resultado | sinal;
            }	
            valor = abs(valor) << 0;
            resultado = resultado | valor;
            //printf("\nRESULTADO lw = %d", resultado);
    }
    else if(opcode==15){
        //printf("\nEXECUTOu memoria[%d + r%d] = r%d\n", destino, fonte1, fonte2);
        //memoria[bancoRegs[fonte1] + fonte2] = bancoRegs[destino];
        resultado = 33 << 26;
        int valor = fonte2;
        //printf("\nValor: %d", valor);
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

void escreveNoDestino(int resultado){
    //int mascara_destino = 0xFC000000; // 11111100000000000000000000000000 em binário
    // Extrai os 6 bits mais significativos (destino)
    int bits_destino = (resultado >> 26) & 0x3F;
    // Limpa os 6 bits mais significativos do resultado (obtendo o "resto")
    int resto = resultado & 0x1FFFFFF;
    
    int bit_25 = (resultado >> 25) & 1;
    //printf("BIT 25: %d", bit_25);

    //printf("\n\nDESTINO = %d, RESTO = %d", bits_destino, resto);

    if (bits_destino>=0 && bits_destino<=31){
        // Create a bitmask to isolate the 25th bit
        if(bit_25 == 1){
            if(bits_destino == 0 && resto != 0){
                printf("\nO registrador r0 não pode receber valor diferente de 0!");
            }
            else{
                //printf("INSERINDO %d em REGISTRADOR %d\n",resto,bits_destino);
                bancoRegs[bits_destino]=-resto;
            }
        }
        else{
            if(bits_destino == 0 && resto != 0){
                printf("\nO registrador r0 não pode receber valor diferente de 0!");
            }
            else{
                //printf("INSERINDO %d em REGISTRADOR %d\n",resto,bits_destino);
                bancoRegs[bits_destino]=resto;
            }
        }
    }
    else if(bits_destino==32){
        //printf("INSERINDO em pc o valor %d\n",resto);
        //printf("SALTO FOI ESCRITO, RETIROU O STALL\n");
        pc=resto;
        stalled=2;
    }
    else if(bits_destino==33){
        // Extract bits 25 to 16 for the address
        int endereco = (resultado >> 16) & 0x3FF;  // 0x3FF represents 10 bits set to 1
        // Extract bits 15 to 0 for the value to be written
        int valor = resultado & 0x7FFF;  // 0xFFFF represents 16 bits set to 1
        int bit_15 = (resultado >> 15) & 1;
        //printf("INSERINDO em MEM[%d] o valor %d\n", endereco, valor);

        if(endereco%4!=0 && endereco>399){
            printf("ENDEREÇO DE MEMORIA INVALIDO.\n");
            //printf("NAO FOI POSSIVEL INSERIR %d em memoria[%d].\n",valor,endereco);
        }
        else{
            if(bit_15 == 1){
                //memoria[endereco]=-valor;
                colocaMemoria(-valor, endereco);
            }
            else{
                //memoria[endereco]=valor;
                colocaMemoria(valor, endereco);
            }
        }
        
    }


}

//DEPOIS DE JOGAR AS INSTRUCOES PRA UF EU COLOCO OS DADOS E FAÇO A QUANTIDADE DE CICLOS = -1 
//PRA SINALIZAR QUE A LEITURA DE OPERANDOS AINDA NAO FOI FEITA E A EXECUCAO AINDA NAO PODE COMECAR

//DUVIDAS:::: DEPOIS DOS RESULTADOS ESTAREM PRONTOS QJ E QK ANULA?; NAO SEI DIREITO AINDA COMO VAI FUNCIONAR O VETORRESULTADOS
void emiteInstrucao(){
    //printf("\n----------------EMISSAO--------------\n");
    if(ir!=0 && ir!=1073741824){
        int regDestino = getRegistradorDestino(ir);
        int tipoUF_inst = getTipoUF(ir);
        int disponivel = getUFdisponivel(tipoUF_inst);
        //printf("\nUF DISPONIVEL DO TIPO %d = %d", tipoUF_inst, disponivel);
        //printf("\nREG DESTINO = %d\n",regDestino);
        //printf("INSTRUCAO: ");
        printBinario(ir);
        if(getOpcode(ir)==13){
            //printf("\nDesvio incondicional");
        }
        else if(vetorResultados[regDestino] == NULL && disponivel!=-1 && (getOpcode(ir)<9 || getOpcode(ir)>13)){
            //printf("%p", vetorResultados[1]);
            if(tipoUF_inst==0){
                int indice_instrucao = getIndiceInstrucao(ir);
                if(clocki-statusI[indice_instrucao].busca==1 || vetorForwarding[indice_instrucao]==1){
                    unidadesFuncionais.ufAdd[disponivel].instrucao=ir;
                    unidadesFuncionais.ufAdd[disponivel].busy = 1;
                    unidadesFuncionais.ufAdd[disponivel].fi = regDestino;
                    unidadesFuncionais.ufAdd[disponivel].fj = getRegistradorFonte1(ir);
                    unidadesFuncionais.ufAdd[disponivel].fk = getRegistradorFonte2(ir);
                    unidadesFuncionais.ufAdd[disponivel].operacao = getOpcode(ir);
                    if(unidadesFuncionais.ufAdd[disponivel].operacao==1 || unidadesFuncionais.ufAdd[disponivel].operacao==3){
                        unidadesFuncionais.ufAdd[disponivel].qk = NULL;
                    }
                    else{
                        unidadesFuncionais.ufAdd[disponivel].qk = vetorResultados[getRegistradorFonte2(ir)];

                    }
                    if(vetorResultados[getRegistradorFonte1(ir)]==0){
                        unidadesFuncionais.ufAdd[disponivel].qj = NULL;
                    }
                    else{
                        unidadesFuncionais.ufAdd[disponivel].qj = vetorResultados[getRegistradorFonte1(ir)];
                    }
                    //unidadesFuncionais.ufAdd[disponivel].qj = vetorResultados[getRegistradorFonte1(ir)];
                    unidadesFuncionais.ufAdd[disponivel].rj = (unidadesFuncionais.ufAdd[disponivel].qj == NULL);
                    unidadesFuncionais.ufAdd[disponivel].rk = (unidadesFuncionais.ufAdd[disponivel].qk == NULL);
                    unidadesFuncionais.ufAdd[disponivel].qtde_ciclos = -1;
                    vetorResultados[regDestino] = &unidadesFuncionais.ufAdd[disponivel];

                    //Não teve RAW e pode ler nesse ciclo
                    statusI[indice_instrucao].emissao = clocki;
                    vetorForwarding[indice_instrucao]=0;
                    stalled=0;
                    instrucoesEmitidas++;
                    //printf("\nEMITIU %d", ir);
                }
                else{
                    //resetaUF(&unidadesFuncionais.ufAdd[disponivel]);
                    vetorForwarding[indice_instrucao]=1;
                }
                //printf("EMITIU ADD R%d %d %d\n\n",unidadesFuncionais.ufAdd[disponivel].fi,unidadesFuncionais.ufAdd[disponivel].fj,unidadesFuncionais.ufAdd[disponivel].fk);
            }
            else if(tipoUF_inst==1){
                int indice_instrucao = getIndiceInstrucao(ir);
                //printf("\n\nINDICE INST: %d", indice_instrucao);
                if(clocki-statusI[indice_instrucao].busca==1 || vetorForwarding[indice_instrucao]==1){
                    unidadesFuncionais.ufMul[disponivel].instrucao=ir;
                    unidadesFuncionais.ufMul[disponivel].busy = 1;
                    unidadesFuncionais.ufMul[disponivel].fi = regDestino;
                    unidadesFuncionais.ufMul[disponivel].fj = getRegistradorFonte1(ir);
                    unidadesFuncionais.ufMul[disponivel].fk = getRegistradorFonte2(ir);
                    unidadesFuncionais.ufMul[disponivel].operacao = getOpcode(ir);
                    if(vetorResultados[getRegistradorFonte1(ir)]==0){
                        unidadesFuncionais.ufMul[disponivel].qj = NULL;
                    }
                    else{
                        unidadesFuncionais.ufMul[disponivel].qj = vetorResultados[getRegistradorFonte1(ir)];
                    }
                    if(vetorResultados[getRegistradorFonte2(ir)]==0){
                        unidadesFuncionais.ufMul[disponivel].qk = NULL;
                    }
                    else{
                        unidadesFuncionais.ufMul[disponivel].qk = vetorResultados[getRegistradorFonte2(ir)];
                    }
                    unidadesFuncionais.ufMul[disponivel].rj = (unidadesFuncionais.ufMul[disponivel].qj == NULL);
                    unidadesFuncionais.ufMul[disponivel].rk = (unidadesFuncionais.ufMul[disponivel].qk == NULL);
                    unidadesFuncionais.ufMul[disponivel].qtde_ciclos = -1;
                    vetorResultados[regDestino] = &unidadesFuncionais.ufMul[disponivel];
                    //Não teve RAW e pode ler nesse ciclo
                    statusI[indice_instrucao].emissao = clocki;
                    vetorForwarding[indice_instrucao]=0;
                    stalled=0;
                    instrucoesEmitidas++;
                    //printf("\nEMITIU %d", ir);
                }
                else{
                    //resetaUF(&unidadesFuncionais.ufMul[disponivel]);
                    vetorForwarding[indice_instrucao]=1;
                }
            }
            else if(tipoUF_inst==2){
                int indice_instrucao = getIndiceInstrucao(ir);
                //printf("\nINDICE Q TA PEGANDO %d\n",indice_instrucao);
                if(clocki-statusI[indice_instrucao].busca==1 || vetorForwarding[indice_instrucao]==1){
                    unidadesFuncionais.ufInt[disponivel].instrucao=ir;
                    unidadesFuncionais.ufInt[disponivel].busy = 1;
                    unidadesFuncionais.ufInt[disponivel].fi = regDestino;
                    unidadesFuncionais.ufInt[disponivel].fj = getRegistradorFonte1(ir);
                    unidadesFuncionais.ufInt[disponivel].fk = getRegistradorFonte2(ir);
                    unidadesFuncionais.ufInt[disponivel].operacao = getOpcode(ir);
                    if(unidadesFuncionais.ufInt[disponivel].operacao!=15){
                        vetorResultados[regDestino] = &unidadesFuncionais.ufInt[disponivel];
                        if(unidadesFuncionais.ufInt[disponivel].operacao!=14){
                            if(vetorResultados[getRegistradorFonte1(ir)]==0){
                                unidadesFuncionais.ufInt[disponivel].qj = NULL;
                            }
                            else{
                                unidadesFuncionais.ufInt[disponivel].qj = vetorResultados[getRegistradorFonte1(ir)];
                            }
                        }
                        else{
                            unidadesFuncionais.ufInt[disponivel].qj = NULL;
                        }
                        if(vetorResultados[getRegistradorFonte2(ir)]==0){
                            unidadesFuncionais.ufInt[disponivel].qk = NULL;
                        }
                        else{
                            unidadesFuncionais.ufInt[disponivel].qk = vetorResultados[getRegistradorFonte2(ir)];
                        }
                    }
                    else{
                        vetorResultados[regDestino] = NULL;
                        if(vetorResultados[getRegistradorFonte1(ir)]==0){
                            unidadesFuncionais.ufInt[disponivel].qj = NULL;
                        }
                        else{
                            unidadesFuncionais.ufInt[disponivel].qj = vetorResultados[getRegistradorFonte1(ir)];
                        }
                        if(vetorResultados[getRegistradorFonte2(ir)]==0){
                            unidadesFuncionais.ufInt[disponivel].qk = NULL;
                        }
                        else{
                            unidadesFuncionais.ufInt[disponivel].qk = vetorResultados[getRegistradorFonte2(ir)];
                        }
                    }
                    unidadesFuncionais.ufInt[disponivel].rj = (unidadesFuncionais.ufInt[disponivel].qj == NULL);
                    unidadesFuncionais.ufInt[disponivel].rk = (unidadesFuncionais.ufInt[disponivel].qk == NULL);
                    unidadesFuncionais.ufInt[disponivel].qtde_ciclos = -1;
                    //printf("\n\nFi: %d, Fj: %d, Fk: %d\n\n", unidadesFuncionais.ufInt[disponivel].fi, unidadesFuncionais.ufInt[disponivel].fj, unidadesFuncionais.ufInt[disponivel].fk);
                    //printf("INSTRUSSAO %s\n",instrucaoToString(unidadesFuncionais.ufInt[disponivel].instrucao));
                    //printf("FI: %d", unidadesFuncionais.ufInt[disponivel].fi);
                    //vetorResultados[regDestino] = &unidadesFuncionais.ufInt[disponivel];
                    //Não teve RAW e pode ler nesse ciclo
                    //printf("\nENTROU AQUI AAA\n");
                    statusI[indice_instrucao].emissao = clocki;
                    vetorForwarding[indice_instrucao]=0;
                    stalled=0;
                    instrucoesEmitidas++;
                    //printf("\nEMITIU %d", ir);
                }
                else{
                    //resetaUF(&unidadesFuncionais.ufInt[disponivel]);
                    vetorForwarding[indice_instrucao]=1;
                }
            }
            //printf("\n%d\n", statusI[pc].emissao);
            //Esse caso acontece quando houve um stall porque uma instrução não conseguiu ser emitida.
            //Ou seja, é o caso em que houve WAW ou não UF livre. Mas como agora a instrução está sendo emitida, esse stall tem que ser removido.
            
        }
        else if(disponivel!=-1 && getOpcode(ir)>=9 && getOpcode(ir)<=12){
            int indice_instrucao = getIndiceInstrucao(ir);
            //printf("\n\nINDICE: %d", indice_instrucao);
            if(statusI[indice_instrucao].emissao==0 && statusI[indice_instrucao].busca!=0){
                unidadesFuncionais.ufInt[disponivel].instrucao=ir;
                unidadesFuncionais.ufInt[disponivel].busy = 1;
                unidadesFuncionais.ufInt[disponivel].fi = regDestino;
                unidadesFuncionais.ufInt[disponivel].fj = getRegistradorFonte1(ir);
                unidadesFuncionais.ufInt[disponivel].fk = getRegistradorFonte2(ir);
                unidadesFuncionais.ufInt[disponivel].operacao = getOpcode(ir);
                if(vetorResultados[getRegistradorFonte1(ir)]==0){
                    unidadesFuncionais.ufInt[disponivel].qj = NULL;
                }
                else{
                    unidadesFuncionais.ufInt[disponivel].qj = vetorResultados[getRegistradorFonte1(ir)];
                }
                if(vetorResultados[getRegistradorFonte2(ir)]==0){
                    unidadesFuncionais.ufInt[disponivel].qk = NULL; 
                }
                else{
                    unidadesFuncionais.ufInt[disponivel].qk = vetorResultados[getRegistradorFonte2(ir)]; 
                }
                //unidadesFuncionais.ufInt[disponivel].qj = vetorResultados[getRegistradorFonte1(ir)];
                //unidadesFuncionais.ufInt[disponivel].qk = vetorResultados[getRegistradorFonte2(ir)];
                unidadesFuncionais.ufInt[disponivel].rj = (unidadesFuncionais.ufInt[disponivel].qj == NULL);
                unidadesFuncionais.ufInt[disponivel].rk = (unidadesFuncionais.ufInt[disponivel].qk == NULL);
                unidadesFuncionais.ufInt[disponivel].qtde_ciclos = -1;
                //printf("\n\nFi: %d, Fj: %d, Fk: %d\n\n", unidadesFuncionais.ufInt[disponivel].fi, unidadesFuncionais.ufInt[disponivel].fj, unidadesFuncionais.ufInt[disponivel].fk);
                //printf("\nAAAAAAAAAAAAAA\n");
                //printf("FI: %d", unidadesFuncionais.ufInt[disponivel].fi);
                //printf("\nEMITIU %d", ir);
                statusI[getIndiceInstrucao(ir)].emissao = clocki;
                instrucoesEmitidas++;
            }
        }
        else{
            stalled = 1;
            //printf("\nNAO FOI POSSIVEL EMITIR A INSTRUCAO\n\n");
        }
    }
    else{
        //printf("\nNão houve emissão");
    }
    //printf("\n-----------------------------\n");
}

void leituraDeOperandos(){
    //("\n-------------------LEITURA OPS------------------\n");
    //printf("\nVetForw de I6: %d", vetorForwarding[6]);
    for(int i=0; i<unidadesFuncionais.qtdeADD; i++){
        //printf("\n\nADD rj: %d; rk: %d", unidadesFuncionais.ufAdd[i].rj, unidadesFuncionais.ufAdd[i].rk);
        if(unidadesFuncionais.ufAdd[i].rj == 1 && unidadesFuncionais.ufAdd[i].rk == 1){
            int indice_instrucao = getIndiceInstrucaoLO(unidadesFuncionais.ufAdd[i].instrucao);
            //printf("\n\nINDICE EH %d", indice_instrucao);
            if(vetorForwarding[indice_instrucao]==0){
                //Não teve RAW e pode ler nesse ciclo
                unidadesFuncionais.ufAdd[i].rj = 0;
                unidadesFuncionais.ufAdd[i].rk = 0;
                //printf("\nTEM QUE MOSTRAR");
                unidadesFuncionais.ufAdd[i].valorfj = bancoRegs[unidadesFuncionais.ufAdd[i].fj];
                if(unidadesFuncionais.ufAdd[i].operacao == 1 || unidadesFuncionais.ufAdd[i].operacao == 3){
                    unidadesFuncionais.ufAdd[i].valorfk = unidadesFuncionais.ufAdd[i].fk;
                }
                else{
                    unidadesFuncionais.ufAdd[i].valorfk = bancoRegs[unidadesFuncionais.ufAdd[i].fk];
                }
                
                unidadesFuncionais.ufAdd[i].qtde_ciclos = getCiclos(unidadesFuncionais.ufAdd[i].operacao);
                //printf("\nVIROU: rj: %d; rk: %d", unidadesFuncionais.ufAdd[i].rj, unidadesFuncionais.ufAdd[i].rk);
                //printf("\nINST= %d", unidadesFuncionais.ufAdd[i].instrucao);
                statusI[indice_instrucao].leitura_op = clocki;
                //vetorForwarding[indice_instrucao]=1;
            }
            else{
                vetorForwarding[indice_instrucao]=0;
                //Teve RAW e pode ler só no próximo ciclo pq não tem forwarding. 
            }

            //printf("LEU %d %d %d", unidadesFuncionais.ufAdd[i].fi, unidadesFuncionais.ufAdd[i].fj, unidadesFuncionais.ufAdd[i].fk);
        }
    }
    for(int i=0; i<unidadesFuncionais.qtdeMUL; i++){
        if(unidadesFuncionais.ufMul[i].rj == 1 && unidadesFuncionais.ufMul[i].rk == 1){
            int indice_instrucao = getIndiceInstrucaoLO(unidadesFuncionais.ufMul[i].instrucao);
            if(vetorForwarding[indice_instrucao]==0){
                //Não teve RAW e pode ler nesse ciclo
                unidadesFuncionais.ufMul[i].rj = 0;
                unidadesFuncionais.ufMul[i].rk = 0;
                unidadesFuncionais.ufMul[i].valorfj = bancoRegs[unidadesFuncionais.ufMul[i].fj];
                unidadesFuncionais.ufMul[i].valorfk = bancoRegs[unidadesFuncionais.ufMul[i].fk];
                unidadesFuncionais.ufMul[i].qtde_ciclos = getCiclos(unidadesFuncionais.ufMul[i].operacao);
                int indice_instrucao = getIndiceInstrucaoLO(unidadesFuncionais.ufMul[i].instrucao);
                statusI[indice_instrucao].leitura_op = clocki;
                //vetorForwarding[indice_instrucao]=1;
            }
            else{
                vetorForwarding[indice_instrucao]=0;
                //Teve RAW e pode ler só no próximo ciclo pq não tem forwarding. 
            }
            //printf("LEU %d %d %d", unidadesFuncionais.ufMul[i].fi, unidadesFuncionais.ufMul[i].fj, unidadesFuncionais.ufMul[i].fk);
        }
    }
    for(int i=0; i<unidadesFuncionais.qtdeINT; i++){
        //printf("\n\nrj: %d; rk: %d", unidadesFuncionais.ufInt[i].rj, unidadesFuncionais.ufInt[i].rk);
        if(unidadesFuncionais.ufInt[i].rj == 1 && unidadesFuncionais.ufInt[i].rk == 1){
            int indice_instrucao = getIndiceInstrucaoLO(unidadesFuncionais.ufInt[i].instrucao);
            //printf("\nVetor Forwarding: %d", vetorForwarding[indice_instrucao]);
            if(vetorForwarding[indice_instrucao]==0){
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
                //printf("\nVALORES SAO: %d E %d", unidadesFuncionais.ufInt[i].valorfj, unidadesFuncionais.ufInt[i].valorfk);
                unidadesFuncionais.ufInt[i].qtde_ciclos = getCiclos(unidadesFuncionais.ufInt[i].operacao);
                statusI[indice_instrucao].leitura_op = clocki;
                //vetorForwarding[indice_instrucao]=0;
            }
            else{
                vetorForwarding[indice_instrucao]=0;
                //Teve RAW e pode ler só no próximo ciclo pq não tem forwarding. 
            }
            //printf("LEU %d %d %d", unidadesFuncionais.ufInt[i].fi, unidadesFuncionais.ufInt[i].fj, unidadesFuncionais.ufInt[i].fk);
        }
    }
    //printf("\n-----------------------------\n");
}

//TALVEZ SEJA MELHOR SÓ SIMULAR A EXECUCAO E FAZER ELA DE FATO SÓ NA ESCRITA DE RESULTADOS
void execucao(){
    //printf("\n----------EXECUCAO-----------\n");
    for(int i=0; i<unidadesFuncionais.qtdeADD; i++){
        if(unidadesFuncionais.ufAdd[i].qtde_ciclos!=0 && unidadesFuncionais.ufAdd[i].qtde_ciclos!=-1){
            if(statusI[getIndiceInstrucao(unidadesFuncionais.ufAdd[i].instrucao)].execucaoinicio==0){
                statusI[getIndiceInstrucao(unidadesFuncionais.ufAdd[i].instrucao)].execucaoinicio=clocki;
            }
            unidadesFuncionais.ufAdd[i].qtde_ciclos--;
            statusI[getIndiceInstrucaoEX(unidadesFuncionais.ufAdd[i].instrucao)].execucaofim=clocki;
            //printf("\nADD%d FALTAM %d CICLOS\n", i, unidadesFuncionais.ufAdd[i].qtde_ciclos);
        }
        if(unidadesFuncionais.ufAdd[i].qtde_ciclos==0 && (unidadesFuncionais.ufAdd[i].fi!=0)){
            int resultado = executaInstrucao(unidadesFuncionais.ufAdd[i].fi, unidadesFuncionais.ufAdd[i].valorfj, unidadesFuncionais.ufAdd[i].valorfk, unidadesFuncionais.ufAdd[i].operacao);
            //unidadesFuncionais.ufAdd[i].qtde_ciclos = 0;
            //acho que tem que fazer resultado ser resultado+destino
            //possiveis exemplos: bits resultado e bits para registrador ou pc ou memoria 
            //se for registrador, é de 0 a 31, se for pc pode ser 32 e memoria 33 ou qualquer outra coisa q kiser decidir 
            colocaBarramentoResultados(resultado);
            printBarramentoResultados();
            //printf("\nRESULTADOOOOO: %d", resultado);
        }
    }
    for(int i=0; i<unidadesFuncionais.qtdeMUL; i++){
        if(unidadesFuncionais.ufMul[i].qtde_ciclos!=0 && unidadesFuncionais.ufMul[i].qtde_ciclos!=-1){
            if(statusI[getIndiceInstrucao(unidadesFuncionais.ufMul[i].instrucao)].execucaoinicio==0){
                statusI[getIndiceInstrucao(unidadesFuncionais.ufMul[i].instrucao)].execucaoinicio=clocki;
            }
            unidadesFuncionais.ufMul[i].qtde_ciclos--;
            statusI[getIndiceInstrucaoEX(unidadesFuncionais.ufMul[i].instrucao)].execucaofim=clocki;
            //printf("\nMUL%d FALTAM %d CICLOS\n", i, unidadesFuncionais.ufMul[i].qtde_ciclos);
        }
        if(unidadesFuncionais.ufMul[i].qtde_ciclos==0 && (unidadesFuncionais.ufMul[i].fi!=0)){
            //executaInstrucao(unidadesFuncionais.ufMul[i].fi, unidadesFuncionais.ufMul[i].fj, unidadesFuncionais.ufMul[i].fk, unidadesFuncionais.ufMul[i].operacao);
            //unidadesFuncionais.ufMul[i].qtde_ciclos = 0;
            int resultado = executaInstrucao(unidadesFuncionais.ufMul[i].fi, unidadesFuncionais.ufMul[i].valorfj, unidadesFuncionais.ufMul[i].valorfk, unidadesFuncionais.ufMul[i].operacao);
            colocaBarramentoResultados(resultado);
            printBarramentoResultados();
        }
    }
    for(int i=0; i<unidadesFuncionais.qtdeINT; i++){
        if(unidadesFuncionais.ufInt[i].qtde_ciclos!=0 && unidadesFuncionais.ufInt[i].qtde_ciclos!=-1){
            if(statusI[getIndiceInstrucao(unidadesFuncionais.ufInt[i].instrucao)].execucaoinicio==0){
                statusI[getIndiceInstrucao(unidadesFuncionais.ufInt[i].instrucao)].execucaoinicio=clocki;
            }

            unidadesFuncionais.ufInt[i].qtde_ciclos--;
            statusI[getIndiceInstrucaoEX(unidadesFuncionais.ufInt[i].instrucao)].execucaofim=clocki;
            //printf("\nINT%d FALTAM %d CICLOS\n", i, unidadesFuncionais.ufInt[i].qtde_ciclos);
        }
        if(unidadesFuncionais.ufInt[i].qtde_ciclos==0 && (unidadesFuncionais.ufInt[i].fi!=0 || unidadesFuncionais.ufInt[i].operacao==15 ||
        (getOpcode(unidadesFuncionais.ufInt[i].instrucao)<=12 && getOpcode(unidadesFuncionais.ufInt[i].instrucao)>=9))){
            //printf("\nEntrou aqui");
            //executaInstrucao(unidadesFuncionais.ufInt[i].fi, unidadesFuncionais.ufInt[i].fj, unidadesFuncionais.ufInt[i].fk, unidadesFuncionais.ufInt[i].operacao);
            //unidadesFuncionais.ufInt[i].qtde_ciclos = 0;
            int resultado = executaInstrucao(unidadesFuncionais.ufInt[i].fi, unidadesFuncionais.ufInt[i].valorfj, unidadesFuncionais.ufInt[i].valorfk, unidadesFuncionais.ufInt[i].operacao);
            //printf("\nBINARIO INDO PRO BARRAMENTO: ");
            //printBinario(resultado);
            //printf("-------");
            //printf("DESTINO = %d FONTE1 = %d FONTE2 = %d OPERACAO=%d \n\n",unidadesFuncionais.ufInt[i].fi,unidadesFuncionais.ufInt[i].fj,unidadesFuncionais.ufInt[i].fk,unidadesFuncionais.ufInt[i].operacao);
            colocaBarramentoResultados(resultado);
            printBarramentoResultados();


        }
    }
    //printf("\n-----------------\n");
}


//TEMOS QUE LEVAR EM CONSIDERAÇÃO QUE INSTRUÇÕES DE STORE E SALTO NÃO TEM RAW ENTAO SEMPRE PODE ESCREVER
void escritaResultados(){
    //printf("\n-----------------ESCRITA---------------\n");
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
                //printf("\nFI DO ADD: %d FJ DO INT: %d", unidadesFuncionais.ufAdd[i].fi, unidadesFuncionais.ufInt[j].fj);
                //printf("\nRJ DO INT: %d", unidadesFuncionais.ufInt[1].rj);
                if((unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufInt[j].fj || unidadesFuncionais.ufInt[j].rj==0)
                && (unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufInt[j].fk || unidadesFuncionais.ufInt[j].rk==0)){
                }
                else{
                    checkAddB = 0;
                }
            }
            for(int j=0; j<unidadesFuncionais.qtdeADD; j++){
                if((unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufAdd[j].fj || unidadesFuncionais.ufAdd[j].rj==0)
                && (unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufAdd[j].fk || unidadesFuncionais.ufAdd[j].rk==0)){
                }
                else{
                    checkAddC = 0;
                }
            }

            linhaBarramento = pegaBarramentoResultados(executaInstrucao(unidadesFuncionais.ufAdd[i].fi, unidadesFuncionais.ufAdd[i].fj, unidadesFuncionais.ufAdd[i].fk, unidadesFuncionais.ufAdd[i].operacao));

            if(checkAddA && checkAddB && checkAddC && unidadesFuncionais.ufAdd[i].qtde_ciclos == 0 && unidadesFuncionais.ufAdd[i].fi != 0){
                for(int j=0; j<unidadesFuncionais.qtdeADD; j++){
                    if((unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufAdd[j].fj || unidadesFuncionais.ufAdd[j].rj==0)
                    && (unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufAdd[j].fk || unidadesFuncionais.ufAdd[j].rk==0)){
                        if(i!=j){
                            if(unidadesFuncionais.ufAdd[j].qj==&unidadesFuncionais.ufAdd[i]){
                                unidadesFuncionais.ufAdd[j].rj = 1;
                                unidadesFuncionais.ufAdd[j].qj = NULL;
                                vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufAdd[j].instrucao)]=1;
                            }
                            if(unidadesFuncionais.ufAdd[j].qk==&unidadesFuncionais.ufAdd[i]){
                                unidadesFuncionais.ufAdd[j].rk = 1;
                                unidadesFuncionais.ufAdd[j].qk = NULL;
                                vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufAdd[j].instrucao)]=1;
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
                            vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufMul[j].instrucao)]=1;
                        }
                        if(unidadesFuncionais.ufMul[j].qk==&unidadesFuncionais.ufAdd[i]){
                            unidadesFuncionais.ufMul[j].rk = 1;
                            unidadesFuncionais.ufMul[j].qk = NULL;
                            vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufMul[j].instrucao)]=1;
                        }
                    }
                }
                for(int j=0; j<unidadesFuncionais.qtdeINT; j++){
                    if((unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufInt[j].fj || unidadesFuncionais.ufInt[j].rj==0)
                    && (unidadesFuncionais.ufAdd[i].fi!=unidadesFuncionais.ufInt[j].fk || unidadesFuncionais.ufInt[j].rk==0)){
                        if(unidadesFuncionais.ufInt[j].qj==&unidadesFuncionais.ufAdd[i]){
                            unidadesFuncionais.ufInt[j].rj = 1;
                            unidadesFuncionais.ufInt[j].qj = NULL;
                            //printf("OPCODE: %d", unidadesFuncionais.ufInt[j].operacao);
                            if(unidadesFuncionais.ufInt[j].operacao==14 && unidadesFuncionais.ufInt[j].operacao!=15){
                                vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufInt[j].instrucao)]=0;
                            }
                            else{
                                vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufInt[j].instrucao)]=1;
                            }
                            //vetorForwarding[getIndiceInstrucao(unidadesFuncionais.ufInt[j].instrucao)]=1;
                        }
                        if(unidadesFuncionais.ufInt[j].qk==&unidadesFuncionais.ufAdd[i]){
                            unidadesFuncionais.ufInt[j].rk = 1;
                            unidadesFuncionais.ufInt[j].qk = NULL;
                            vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufInt[j].instrucao)]=1;
                        }
                    }
                }
                
                //printf("\nENTROU AQUI PRO ADD R%d R%d R%d\n",unidadesFuncionais.ufAdd[i].fi,unidadesFuncionais.ufAdd[i].fj,unidadesFuncionais.ufAdd[i].fk);
                linhaBarramento = pegaBarramentoResultados(executaInstrucao(unidadesFuncionais.ufAdd[i].fi, unidadesFuncionais.ufAdd[i].valorfj, unidadesFuncionais.ufAdd[i].valorfk, unidadesFuncionais.ufAdd[i].operacao));
                if(linhaBarramento!=-1){
                    vetorResultados[unidadesFuncionais.ufAdd[i].fi] = 0;
                    escreveNoDestino(barramentoResultados[linhaBarramento]);
                    statusI[getIndiceInstrucaoER(unidadesFuncionais.ufAdd[i].instrucao)].escrita=clocki;
                    unidadesFuncionais.ufAdd[i].instrucao=0;
                    unidadesFuncionais.ufAdd[i].busy = 0;
                    unidadesFuncionais.ufAdd[i].fi = 0;
                    unidadesFuncionais.ufAdd[i].fj = 0;
                    unidadesFuncionais.ufAdd[i].fk = 0;
                    unidadesFuncionais.ufAdd[i].valorfj = 0;
                    unidadesFuncionais.ufAdd[i].valorfk = 0;
                    unidadesFuncionais.ufAdd[i].operacao = 0;
                    unidadesFuncionais.ufAdd[i].qj = NULL;
                    unidadesFuncionais.ufAdd[i].qk = NULL;
                    unidadesFuncionais.ufAdd[i].rj = 0;
                    unidadesFuncionais.ufAdd[i].rk = 0;
                    instrucoesEfetivadas++;
                }
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
                && (unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufAdd[j].fk || unidadesFuncionais.ufAdd[j].rk==0)){
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
        (unidadesFuncionais.ufMul[i].fi != 0)){
            for(int j=0; j<unidadesFuncionais.qtdeMUL; j++){
                if((unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufMul[j].fj || unidadesFuncionais.ufMul[j].rj==0)
                && (unidadesFuncionais.ufMul[i].fi!=unidadesFuncionais.ufMul[j].fk || unidadesFuncionais.ufMul[j].rk==0)){
                    if(i!=j){
                        if(unidadesFuncionais.ufMul[j].qj==&unidadesFuncionais.ufMul[i]){
                            unidadesFuncionais.ufMul[j].rj = 1;
                            unidadesFuncionais.ufMul[j].qj = NULL;
                            vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufMul[j].instrucao)]=1;
                        }
                        if(unidadesFuncionais.ufMul[j].qk==&unidadesFuncionais.ufMul[i]){
                            unidadesFuncionais.ufMul[j].rk = 1;
                            unidadesFuncionais.ufMul[j].qk = NULL;
                            vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufMul[j].instrucao)]=1;
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
                        vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufAdd[j].instrucao)]=1;
                    }
                    if(unidadesFuncionais.ufAdd[j].qk==&unidadesFuncionais.ufMul[i]){
                        unidadesFuncionais.ufAdd[j].rk = 1;
                        unidadesFuncionais.ufAdd[j].qk = NULL;
                        vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufAdd[j].instrucao)]=1;
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
                            vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufInt[j].instrucao)]=1;
                        }
                        else{
                            vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufInt[j].instrucao)]=0;
                        }
                    }
                    if(unidadesFuncionais.ufInt[j].qk==&unidadesFuncionais.ufMul[i]){
                        unidadesFuncionais.ufInt[j].rk = 1;
                        unidadesFuncionais.ufInt[j].qk = NULL;
                        vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufInt[j].instrucao)]=1;
                    }
                }
            }
            linhaBarramento = pegaBarramentoResultados(executaInstrucao(unidadesFuncionais.ufMul[i].fi, unidadesFuncionais.ufMul[i].valorfj, unidadesFuncionais.ufMul[i].valorfk, unidadesFuncionais.ufMul[i].operacao));
            if(linhaBarramento!=-1){
                vetorResultados[unidadesFuncionais.ufMul[i].fi] = 0;
                escreveNoDestino(barramentoResultados[linhaBarramento]);
                statusI[getIndiceInstrucaoER(unidadesFuncionais.ufMul[i].instrucao)].escrita=clocki;
                unidadesFuncionais.ufMul[i].instrucao=0;
                unidadesFuncionais.ufMul[i].busy = 0;
                unidadesFuncionais.ufMul[i].fi = 0;
                unidadesFuncionais.ufMul[i].fj = 0;
                unidadesFuncionais.ufMul[i].fk = 0;
                unidadesFuncionais.ufMul[i].valorfj = 0;
                unidadesFuncionais.ufMul[i].valorfk = 0;
                unidadesFuncionais.ufMul[i].operacao = 0;
                unidadesFuncionais.ufMul[i].qj = NULL;
                unidadesFuncionais.ufMul[i].qk = NULL;
                unidadesFuncionais.ufMul[i].rj = 0;
                unidadesFuncionais.ufMul[i].rk = 0;
                instrucoesEfetivadas++;
            }
        }
        checkMulA = 1;
        checkMulB = 1;
        checkMulC = 1;
    }
    for(int i=0; i<unidadesFuncionais.qtdeINT; i++){
        if(unidadesFuncionais.ufInt[i].qtde_ciclos==0){
            if(getOpcode(unidadesFuncionais.ufInt[i].instrucao)<9 || getOpcode(unidadesFuncionais.ufInt[i].instrucao)>13){
                for(int j=0; j<unidadesFuncionais.qtdeADD; j++){
                    //printf("\nFI INT: %d, FJ ADD: %d, FK ADD: %d", unidadesFuncionais.ufInt[i].fi, unidadesFuncionais.ufInt[j].fj, unidadesFuncionais.ufInt[j].fk);
                    //printf("\n%d, %d", ((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufAdd[j].fj) || unidadesFuncionais.ufAdd[j].rj==0), ((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufAdd[j].fk) || unidadesFuncionais.ufAdd[j].rk==0));
                    if(((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufAdd[j].fj || unidadesFuncionais.ufAdd[j].rj==0)
                    && (unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufAdd[j].fk || unidadesFuncionais.ufAdd[j].rk==0))
                    || unidadesFuncionais.ufInt[i].operacao==15){
                        //printf("\nQk: %p; UF: %p", unidadesFuncionais.ufAdd[j].qk, &unidadesFuncionais.ufInt[i]);
                    }
                    else{
                        checkIntA = 0;
                    }
                }
                for(int j=0; j<unidadesFuncionais.qtdeMUL; j++){
                    if((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufMul[j].fj || unidadesFuncionais.ufMul[j].rj==0)
                    && (unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufMul[j].fk || unidadesFuncionais.ufMul[j].rk==0)){
                    }
                    else{
                        checkIntB = 0;
                    }
                }
                for(int j=0; j<unidadesFuncionais.qtdeINT; j++){
                    if((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufInt[j].fj || unidadesFuncionais.ufInt[j].rj==0)
                    && (unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufInt[j].fk || unidadesFuncionais.ufInt[j].rk==0)){
                    }
                    else{
                        checkIntC = 0;
                    }
                }
            }
        }
        //printf("\n\nCHECK fi = %d, fj = %d, fk = %d", unidadesFuncionais.ufInt[i].fi, unidadesFuncionais.ufInt[i].fj, unidadesFuncionais.ufInt[i].fk);
        //printf("\n\nCHECK %d, %d, %d", checkIntA, checkIntB, checkIntC);

        if(checkIntA && checkIntB && checkIntC && unidadesFuncionais.ufInt[i].qtde_ciclos == 0 && (unidadesFuncionais.ufInt[i].fi != 0
        || getOpcode(unidadesFuncionais.ufInt[i].instrucao)==15 || (getOpcode(unidadesFuncionais.ufInt[i].instrucao)<=12 && getOpcode(unidadesFuncionais.ufInt[i].instrucao)>=9))){
            //printf("\nentra aqui no 57");
            for(int j=0; j<unidadesFuncionais.qtdeINT; j++){
                if((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufInt[j].fj || unidadesFuncionais.ufInt[j].rj==0)
                    && (unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufInt[j].fk || unidadesFuncionais.ufInt[j].rk==0)){
                    if(i!=j){
                        if(unidadesFuncionais.ufInt[j].qj==&unidadesFuncionais.ufInt[i]){
                            unidadesFuncionais.ufInt[j].rj = 1;
                            unidadesFuncionais.ufInt[j].qj = NULL;
                            if(unidadesFuncionais.ufInt[j].operacao!=14 && unidadesFuncionais.ufInt[j].operacao!=15){
                                vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufInt[j].instrucao)]=1;
                            }
                            else{
                                vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufInt[j].instrucao)]=0;
                            }
                        }
                        if(unidadesFuncionais.ufInt[j].qk==&unidadesFuncionais.ufInt[i]){
                            unidadesFuncionais.ufInt[j].rk = 1;
                            unidadesFuncionais.ufInt[j].qk = NULL;
                            vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufInt[j].instrucao)]=1;

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
                        vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufAdd[j].instrucao)]=1;

                    }
                    if(unidadesFuncionais.ufAdd[j].qk==&unidadesFuncionais.ufInt[i]){
                        unidadesFuncionais.ufAdd[j].rk = 1;
                        unidadesFuncionais.ufAdd[j].qk = NULL;
                        vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufAdd[j].instrucao)]=1;

                    }          
                }
            }
            for(int j=0; j<unidadesFuncionais.qtdeMUL; j++){
                if((unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufMul[j].fj || unidadesFuncionais.ufMul[j].rj==0)
                    && (unidadesFuncionais.ufInt[i].fi!=unidadesFuncionais.ufMul[j].fk || unidadesFuncionais.ufMul[j].rk==0)){
                    if(unidadesFuncionais.ufMul[j].qj==&unidadesFuncionais.ufInt[i]){
                        unidadesFuncionais.ufMul[j].rj = 1;
                        unidadesFuncionais.ufMul[j].qj = NULL;
                        vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufMul[j].instrucao)]=1;

                    }
                    if(unidadesFuncionais.ufMul[j].qk==&unidadesFuncionais.ufInt[i]){
                        unidadesFuncionais.ufMul[j].rk = 1;
                        unidadesFuncionais.ufMul[j].qk = NULL;
                        vetorForwarding[getIndiceInstrucaoER(unidadesFuncionais.ufMul[j].instrucao)]=1;

                    }          
                }
            }

            //printf("\n\nCHECK %d, %d, %d", unidadesFuncionais.ufInt[i].fi, unidadesFuncionais.ufInt[i].fj, unidadesFuncionais.ufInt[i].fk);
            //printf("\n\n%d", pegaBarramentoResultados());
            linhaBarramento = pegaBarramentoResultados(executaInstrucao(unidadesFuncionais.ufInt[i].fi, unidadesFuncionais.ufInt[i].valorfj, unidadesFuncionais.ufInt[i].valorfk, unidadesFuncionais.ufInt[i].operacao));
            if(linhaBarramento!=-1){
                if(unidadesFuncionais.ufInt[i].operacao<9 || unidadesFuncionais.ufInt[i].operacao>13){
                    vetorResultados[unidadesFuncionais.ufInt[i].fi] = NULL;
                }
                else{
                    //stalled = 0;
                    if(unidadesFuncionais.ufInt[i].fi!=abs(unidadesFuncionais.ufInt[i].fi))
                        qtdeloops++;
                }
                escreveNoDestino(barramentoResultados[linhaBarramento]);
                statusI[getIndiceInstrucaoER(unidadesFuncionais.ufInt[i].instrucao)].escrita=clocki;
                unidadesFuncionais.ufInt[i].instrucao=0;
                unidadesFuncionais.ufInt[i].busy = 0;
                unidadesFuncionais.ufInt[i].fi = 0;
                unidadesFuncionais.ufInt[i].fj = 0;
                unidadesFuncionais.ufInt[i].fk = 0;
                unidadesFuncionais.ufInt[i].valorfj = 0;
                unidadesFuncionais.ufInt[i].valorfk = 0;
                unidadesFuncionais.ufInt[i].operacao = 0;
                unidadesFuncionais.ufInt[i].qj = NULL;
                unidadesFuncionais.ufInt[i].qk = NULL;
                unidadesFuncionais.ufInt[i].rj = 0;
                unidadesFuncionais.ufInt[i].rk = 0;
                instrucoesEfetivadas++;
            }
        }
        checkIntA = 1;
        checkIntB = 1;
        checkIntC = 1;
    }
    limpaBarramentoResultados();
    //printf("\n---------------------------\n");
}
