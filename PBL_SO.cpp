#include<stdio.h>
#include <stdlib.h>
#include<pthread.h>
#include <unistd.h>

//-----------CONSTANTES---------------------------------------------------------------------------------
#define QUANTIDADE (4)
#define QUER_ESCREVER (0)
#define ESCREVENDO (1)
#define JA_ESCREVI (2)
//----------PROTOTIPOS DAS FUNÇÕES----------------------------------------------------------------------
void *escritores(void * arg);
void realizarEscrita(int numThread,int idArquivo);
void modificarArquivo(int numThread,int idArquivo);
void *leitores(void * arg);
//----------STRUCT DE ESTADOS DOS ESCRITORES------------------------------------------------------------
typedef struct estados{
	int estado[QUANTIDADE];                                //Vetor para o estado da thread.
	int arquivo[QUANTIDADE];
}Estados;
//-----------VARÍAVEIS GLOBAIS--------------------------------------------------------------------------
char nomeArquivo[3][15]{                                //Vetor de string que armazena os nomes dos arquivos.
	"documento1.txt", "documento2.txt",	"documento3.txt" //Nomes dos arquivos
};
Estados estadosEscritores;                             //Variavel para armazenar o estado e o arquivo dos escritores
pthread_t threadsLeitores[QUANTIDADE];                //Threads para os leitores
pthread_t threadsEscritores[QUANTIDADE];             //Threads para os escritores
pthread_mutex_t regiaoCritica;                      //Semáforo para entrar na região crítica.
//------------------------------------------------------------------------------------------------------
int main(){
	int indice, criacao;
	for(indice=0;indice<QUANTIDADE;indice++){
		pthread_create(&threadsLeitores[indice],NULL,leitores,(void *)&indice);
	}
	for(indice=0;indice<QUANTIDADE;indice++){
		criacao=pthread_create(&threadsEscritores[indice],NULL,escritores,(void *)&(indice));
		if(criacao){
			printf("Erro ao criar a thread %d",indice);
		}
	}
//Destroi as threads e o muxer
	pthread_mutex_destroy( &(regiaoCritica) );
//	pthread_exit(NULL);
}

//FUNCAO ESCRITORES
void *escritores(void * arg){
	int *numThread= (int *) arg;
	int indice, idArquivo;
	while(1){
		idArquivo=0;//rand()%3;  //Sorteia o número do arquivo a ler lido
		pthread_mutex_lock(&(regiaoCritica)); //Entrando na regiaoCritica   ///Nem sei se assim ta certo, to meio bugada
		estadosEscritores.estado[(*numThread)]= QUER_ESCREVER;
		estadosEscritores.arquivo[(*numThread)]=idArquivo;
		printf("\n-Escritor %d: Desejo escrever no arquivo %s",(*numThread),nomeArquivo[idArquivo]);
		realizarEscrita((*numThread),idArquivo);
		pthread_mutex_unlock(&(regiaoCritica));//saindo da regiao Critica
		
		//O QUE FAZEMOS QUEM TÁ ESPERANDO?????????????????????????????????????????
		for(indice=0; indice<QUANTIDADE; indice++){//quem estava esperando para escrever no mesmo arquivo agora pode escrever
	  		if(estadosEscritores.estado[indice]==QUER_ESCREVER && estadosEscritores.arquivo[indice]==idArquivo){
	  			realizarEscrita(indice,idArquivo);
		  	}
		}
	}
}
//--------------------------PENSAR EM UM NOME Q FAZ SENTIDO
void realizarEscrita(int numThread,int idArquivo){
  int indice=0;
  int escrevendo=0; //Indica se há outro escrito escrevendo no mesmo arquivo desejado. escrevendo=1 tem outro escritor. escrevendo=0 não tem outro escritor ;
  for(indice=0; indice<QUANTIDADE; indice++){
  	if(estadosEscritores.estado[indice]==ESCREVENDO && estadosEscritores.arquivo[indice]==idArquivo){
  		escrevendo=1;
  		printf("\nNo exato momento o escritor %d esta escrevendo neste arquivo %s",indice, nomeArquivo[idArquivo]);//***depois pode apagar se quiser, só para verificacao no momento
	  }
  }
  if(escrevendo==0){
  	estadosEscritores.estado[numThread]==ESCREVENDO; //Altera o estado do escritor para escrevendo, o que indica que nenhum escritor poderá modificar
  	modificarArquivo(numThread,idArquivo); //Modifica o arquivo, através da escrita
  }
}
//------------------------FUNCAO QUE MODIFICA O ARQUIVO-------------------------------------------
void modificarArquivo(int numThread, int idArquivo){
  	FILE *ponteiroEscrita;
	ponteiroEscrita=fopen(nomeArquivo[idArquivo],"a"); //Abre o arquivo em modo leitura/escrita
	if(ponteiroEscrita==NULL){ //Verifica se houve erro ao abrir o arquivo 
		printf("\nErro ao abrir o arquivo"); //e printa mensagem de erro
	}
	else{
		printf("\n-Escritor %d: Estou escrevendo no arquivo %s",numThread,nomeArquivo[idArquivo]);//printa na tela
		fprintf(ponteiroEscrita,"\nSou o Escritor %d, estou escrevendo.",numThread); //Escreve uma frase no arquivo
		fclose(ponteiroEscrita);  //Fecha o arquivo
		printf("\nEscritor %d: acabei de escrever no arquivo",numThread);
	}
	estadosEscritores.estado[numThread]==JA_ESCREVI; //Altera o estado do escritor
}

//----------FUNCAO LEITORES
/*Esta função realiza a leitura dos arquivos*/
void *leitores(void * arg){
	int *numThread= (int *) arg;
	int idArquivo;
	char texto[50];
	FILE *ponteiroLeitura;
	printf("\nO Leitor %d foi criado",*(numThread));
	while(1){
		idArquivo=0; //rand()% 3;	//Sorteia um numero aleatorio que determinará o nome do arquivo
		ponteiroLeitura=fopen(nomeArquivo[idArquivo],"r");//Abre o arquivo em modo leitura
		printf("\n-Leitor %d: Estou lendo o arquivo %d: %s",(*numThread),idArquivo,nomeArquivo[idArquivo]);
		printf("\n---------------Conteudo do arquivo %s lido pelo Leitor %d------------------",nomeArquivo[idArquivo],(*numThread));
		do{//Será lido enquanto não for o final do arquivo
			fgets(texto,50,ponteiroLeitura); //Ler o texto
			printf("\n\t%s",texto); //Imprime na tela
		}while(feof(ponteiroLeitura)==0);
		fclose(ponteiroLeitura); //fecha o arquivo
		printf("\n-Leitor %d: Finalizei a leitura do arquivo %d: %s",(*numThread),idArquivo,nomeArquivo[idArquivo]);	
	}
}


