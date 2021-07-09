/**LEIAAAAAAA
O CODIGO APRESENTA ERROS EM: 

-SÓ TEM 1 SINCRONIZADOR
-OS LEITORES POR TER MENOS PRIORIDADE, NÃO ESTÃO CONSEGUINDO LER
-OS ARQUIVOS ESTAO SENDO SINCRONIZADOS, PORÉM SEM UMA THREAD
-AINDA NAO CRIA AS THREADS DE FORMA ALEATORIA COMO É PEDIDO NO PROBLEMAS
-PRECISEI FAZER UM VETOR PARA ARMAZENAR OS INDICES DOS ARQUIVOS A SEREM LIDOS/ESCRITOS POIS A FUNÇÃO RAND() DENTRO DAS THREADS GERAM SEMPRE O MESMO NUMERO
*/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<time.h>


//-----------CONSTANTES---------------------------------------------------------------------------------
#define QUANTIDADE (4)
#define QUER_ESCREVER (0)
#define ESCREVENDO (1)
#define JA_ESCREVI (2)
#define SINCRONIZANDO (0)
#define FINALIZADO (1)
#define ARQ_ANTERIOR (arquivoSincronizador+3-1)%3
#define ARQ_PROXIMO (arquivoSincronizador+1)%3
//----------PROTOTIPOS DAS FUNÇÕES----------------------------------------------------------------------
void *escritores(void * arg);
void realizarEscrita(int numThread,int idArquivo);
void modificarArquivo(int numThread,int idArquivo);
void *leitores(void * arg);
int checarEscritor(int idArquivo);
void analisarEstado(int idArquivo,int numThread);
void leitura(int idArquivo,int numThread);
void * sincronizar(void *argumento);
void tentaEscrever(int idArquivo, int numThread );
void sincronizar();
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
pthread_t threadSincronizador;
pthread_mutex_t regiaoCritica;                      //Semáforo para entrar na região crítica.
pthread_mutex_t mutexEscritores[QUANTIDADE];        //Mutex para cada escritor
pthread_mutex_t mutexLeitores[QUANTIDADE]; 
int estadoSincronizadores[2]; 
bool sincronia;     
int arquivoSincronizador;  
int indicesEscritor[QUANTIDADE];
int indicesLeitor[QUANTIDADE];          
//------------------------------------------------------------------------------------------------------
int main(){
	int indice, criacao;
	srand( (unsigned)time(NULL) );
	//sorteia os numeros dos arquivos a serem lidos e salva no vetor
	for(indice=0;indice<QUANTIDADE;indice++){ 
		indicesLeitor[indice]=rand()%3;
		printf("L%d:%d---",indice,indicesLeitor[indice]);
	}
	//sorteia os numeros dos arquivos a serem escritos e salva no vetor
	for(indice=0;indice<QUANTIDADE;indice++){ 
		indicesEscritor[indice]=rand()%3;
		printf("E%d:%d---",indice,indicesEscritor[indice]);
	}
	//cria o mutex da regiao critica
	pthread_mutex_init( &(regiaoCritica), NULL);
	 //cria os mutex dos escritores
	for(indice=0;indice<QUANTIDADE;indice++){ 
		pthread_mutex_init(&(mutexEscritores[indice]),NULL);
	}
/*  //cria os mutex dos leitores
	for(indice=0;indice<QUANTIDADE;indice++){
		pthread_mutex_init(&(mutexLeitores[indice]),NULL);
	}*/
	//Cria as threads dos Escritores
	for(indice=0;indice<QUANTIDADE;indice++){ 
		criacao=pthread_create(&threadsEscritores[indice],NULL,escritores,(void *)&(indice));
		if(criacao){
			printf("Erro ao criar a thread %d",indice);
		}
	}
	//Cria as threads dos leitores
	for(indice=0;indice<QUANTIDADE;indice++){
		pthread_create(&threadsLeitores[indice],NULL,leitores,(void *)&indice);
	}
	
	//Destroi as threads e os mutex
	pthread_mutex_destroy( &(regiaoCritica) );
	for(indice=0;indice<QUANTIDADE;indice++){ 
		pthread_mutex_destroy(&(mutexEscritores[indice]));
	}
	//	pthread_exit(NULL);
}
//--------------------------------------------------------------------------------------------------------------
void * sincronizadores(void *argumento){
	if(sincronia==true){
		sincronizar();
		sincronia=false;
	}
//	pthread_mutex_unlock(&(regiaoCritica));//saindo da regiao Critica
}
void sincronizar(){
	FILE *leitura;
	FILE *escrita1, *escrita2;
	char texto[50];
	estadoSincronizadores[0]=SINCRONIZANDO;
	printf("\n-SINCRONIZADOR: Estou sincronizando os arquivos");
	leitura=fopen(nomeArquivo[arquivoSincronizador],"r");
	escrita1=fopen(nomeArquivo[ARQ_ANTERIOR],"w");
	escrita2=fopen(nomeArquivo[ARQ_PROXIMO],"w");
	do{//Será lido enquanto não for o final do arquivo
		fgets(texto,50,leitura); //Ler o texto
		fprintf(escrita1,"%s",texto); //escreve o conteúdo no arquivo;
		fprintf(escrita2,"%s",texto); //escreve o conteúdo no arquivo;
	}while(feof(leitura)==0);
	fclose(leitura); //fecha o arquivo
	fclose(escrita1); //fecha o arquivo 
	fclose(escrita2); //fecha o arquivo
	estadoSincronizadores[0]=FINALIZADO;
	printf("\n-SINCRONIZADOR:Os arquivos ja foram sincronizados");
}
//----------------------------------FUNCAO ESCRITORES-----------------------------------------------------------------------
void *escritores(void * arg){
	int *numThread= (int *) arg;
	int indice, idArquivo;
	while(1){
		idArquivo= indicesEscritor[(*numThread)];  //número do arquivo a ler lido
		tentaEscrever(idArquivo, (*numThread));
		modificarArquivo((*numThread),idArquivo); //Modifica o arquivo, através da escrita
	}
}
void tentaEscrever(int idArquivo, int numThread ){
	pthread_mutex_lock(&(regiaoCritica)); //Entrando na regiaoCritica   ///Nem sei se assim ta certo, to meio bugada
	printf("\n-ESCRITOR %d: Desejo escrever no arquivo %d, %s",numThread,idArquivo,nomeArquivo[idArquivo]);
	estadosEscritores.estado[numThread]= QUER_ESCREVER;
	estadosEscritores.arquivo[numThread]=idArquivo;
	analisarEstado(idArquivo,numThread);
	pthread_mutex_unlock(&(regiaoCritica));//saindo da regiao Critica
	pthread_mutex_lock( &(mutexEscritores[numThread]) ); //Bloqueia o escritor se não conseguir escrever
}
//----------------------------------------------------------------------------------------------------------------------
void analisarEstado(int idArquivo, int numThread){ //Retorna 0 se não tem escritor no mesmo arquivo e 1 se tem escritor no arquivo.
  int indice=0;
  int escrevendo=0; //Indica se há outro escrito escrevendo no mesmo arquivo desejado. escrevendo=1 tem outro escritor. escrevendo=0 não tem outro escritor ;
  while(indice<QUANTIDADE && escrevendo==0){
  	if(estadosEscritores.estado[indice]==ESCREVENDO || estadoSincronizadores[indice]==SINCRONIZANDO){ //Se houver escrevendo no arquivo desejado
  		escrevendo=1;                                                                                 //Muda o valor da variavel escrevendo para 1
  		printf("\n-ESCRITOR %d:No momento ha outro escritor neste arquivo. %d: %s",numThread,idArquivo, nomeArquivo[idArquivo]);//***depois pode apagar se quiser, só para verificacao no momento
	  }
	  indice++;  //incrementa a variável indice
  }
  if(escrevendo==0){
  	estadosEscritores.estado[numThread]=ESCREVENDO; //Altera o estado do escritor para escrevendo, o que indica que nenhum escritor poderá modificar
  	pthread_mutex_unlock( &(mutexEscritores[numThread]) );   //desbloqueia o escritor
  }
}
//------------------------FUNCAO QUE MODIFICA O ARQUIVO--------------------------------------------------------------------
void modificarArquivo(int numThread, int idArquivo){
  	FILE *ponteiroEscrita;
  	arquivoSincronizador=idArquivo;
  	pthread_mutex_lock(&(regiaoCritica)); 
	ponteiroEscrita=fopen(nomeArquivo[idArquivo],"a"); //Abre o arquivo em modo leitura/escrita
	if(ponteiroEscrita==NULL){ //Verifica se houve erro ao abrir o arquivo 
		printf("\nErro ao abrir o arquivo"); //e printa mensagem de erro
		estadosEscritores.estado[numThread]==JA_ESCREVI; 
	}
	else{
		printf("\n-ESCRITOR %d: Estou escrevendo no arquivo %s",numThread,nomeArquivo[idArquivo]);//printa na tela
		fprintf(ponteiroEscrita,"\nSou o Escritor %d, estou escrevendo.",numThread); //Escreve uma frase no arquivo
		fclose(ponteiroEscrita);  //Fecha o arquivo
		printf("\n-ESCRITOR %d: acabei de escrever no arquivo",numThread);
		estadosEscritores.estado[numThread]==JA_ESCREVI; 
		sincronizar();
	}
	pthread_mutex_unlock(&(regiaoCritica)); 
}



//----------FUNCAO LEITORES----------------------------------------------------------------------------------
/*Esta função realiza a leitura dos arquivos*/
void *leitores(void * arg){
	int *numThread= (int *) arg;
	int numArquivo, haEscritor;
	printf("\nO LEITOR %d foi criado",*(numThread));
	while(1){
		numArquivo=indicesEscritor[(*numThread)];	//Sorteia um numero aleatorio que determinará o nome do arquivo
		printf("\n-LEITOR %d: deseja ler o arquivo %d: %s",(*numThread),numArquivo,nomeArquivo[numArquivo]);
		haEscritor=checarEscritor(numArquivo);
		if(haEscritor==0){  //Se não tiver escritor escrevendo no mesmo aqruivo, o leitor pode ler
			printf("\n-LEITOR %d: Estou lendo o arquivo %d: %s",(*numThread),numArquivo,nomeArquivo[numArquivo]);
			leitura(numArquivo,(*numThread));
		}
		else{ //Se houver escritor escrevendo o leitor não ler e imprime na tela a mensagem
			printf("\n-LEITOR %d: O arquivo desejado esta sendo escrito ou sincronizado",(*numThread));    
			int segundo= 1+(rand()%10);
			sleep(segundo);
		}
	}
}
//---------------FUNCAO QUE CHEGA SE TEM ESCRITOR NO MESMO ARQUIVO DESEJADO-------------------------------------------
int checarEscritor(int numArquivo){ //Retorna 0 se não tem escritor no mesmo arquivo e 1 se tem escritor no arquivo.
  int indice=0;
  int escrevendo=0; //Indica se há outro escrito escrevendo no mesmo arquivo desejado. escrevendo=1 tem outro escritor. escrevendo=0 não tem outro escritor ;
  while(indice<QUANTIDADE && escrevendo==0){
  	if((estadosEscritores.estado[indice]==ESCREVENDO && estadosEscritores.arquivo[indice]==numArquivo) || estadoSincronizadores[indice]==SINCRONIZANDO){ //Se houver escrevendo no arquivo desejado
  		escrevendo=1;  //Muda o valor da variavel escrevendo para 1
	}	
	  indice++;  //incrementa a variável indice
  }
  return escrevendo;   
}

//------------------------------------LEITURA-----------------------------------------------------------------------
void leitura(int numArquivo,int numThread){
	char texto[50];
	FILE *ponteiroLeitura;
	ponteiroLeitura=fopen(nomeArquivo[numArquivo],"r");//Abre o arquivo em modo leitura
	printf("\n-----Conteudo lido pelo Leitor %d:-----",numThread);
	do{//Será lido enquanto não for o final do arquivo
		fgets(texto,50,ponteiroLeitura); //Ler o texto
		printf("\n\tTexto do arquivo: %s",texto); //Imprime na tela o conteúdo lido;
	}while(feof(ponteiroLeitura)==0);
	fclose(ponteiroLeitura); //fecha o arquivo
	printf("\n-LEITOR %d: Finalizei a leitura do arquivo %d: %s",numThread,numArquivo,nomeArquivo[numArquivo]);
}


