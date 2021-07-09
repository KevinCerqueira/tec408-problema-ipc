/*Autor: Estéfane Carmo de Souza, Kevin Cerqueira Gomes, Messias Jr. Lira da Silva
Componente Curricular: Sistemas Operacionais
Concluido em: 27/09/2020
Declaramos que este código foi elaborado por nós e não contém nenhum
trecho de código de outro colega ou de outro autor, tais como provindos de livros e
apostilas, e páginas ou documentos eletrônicos da Internet. Qualquer trecho de código
de outra autoria que não a minha está destacado com uma citação para o autor e a fonte
do código, e estou ciente que estes trechos não serão considerados para fins de avaliação.*/

#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<time.h>

//-----------CONSTANTES---------------------------------------------------------------------------------
#define QUANTIDADE (4)           //Quantidade de threads

#define QUER_ESCREVER (0)       //Tipo de estado do escritor
#define ESCREVENDO (1)         //Tipo de estado do escritor
#define JA_ESCREVI (2)        //Tipo de estado do escritor

#define ARQ_ANTERIOR (arquivoSincronizador+3-1)%3        //Número anterior com base no número do arquivo que foi modificado recentemente
#define ARQ_PROXIMO (arquivoSincronizador+1)%3         //Próximo número com base no número do arquivo que foi modificado recentemente
//----------PROTOTIPOS DAS FUNÇÕES----------------------------------------------------------------------
void sincronizar();                                        //Função que realiza a sincronização dos arquivos
void *escritores(void * arg);						      //Função da thread escritores
void tentaEscrever(int idArquivo, int numThread );       //Função que informa que o escritor deseja escrever e tenta escrever
void analisarEstado(int idArquivo, int numThread);      //Função que verifica se existe escritor escrevendo ou sincronização
void modificarArquivo(int numThread, int idArquivo);   //Função que realiza a escrita no arquivo
void *leitores(void * arg);                           //Função da thread leitores
int checarEscritor(int numArquivo,int numThread);    //Função que verifica se o arquivo está sendo escrito ou se está sendo sincronizado
void leitura(int numArquivo,int numThread);			//Função que realiza a leitura do arquivo
//----------STRUCT DE ESTADOS DOS ESCRITORES------------------------------------------------------------
typedef struct estados{
	int estado[QUANTIDADE];                              //Vetor para o estado da thread.
	int arquivo[QUANTIDADE];						   	//Vetor que armazena os números do arquivo para a determinada thread
}Estados;
//-----------VARÍAVEIS GLOBAIS--------------------------------------------------------------------------
char nomeArquivo[3][15]{                                  //Vetor de string que armazena os nomes dos arquivos.
	"documento1.txt", "documento2.txt",	"documento3.txt" //Nomes dos arquivos
};
Estados estadosEscritores;                             //Variavel para armazenar o estado e o arquivo dos escritores
pthread_t threadsLeitores[QUANTIDADE];                //Threads para os leitores
pthread_t threadsEscritores[QUANTIDADE];             //Threads para os escritores						
pthread_mutex_t regiaoCritica;                      //Semáforo para entrar na região crítica.
pthread_mutex_t mutexEscritores[QUANTIDADE];       //Mutex para cada escritor
bool sincroniza=false;     		
int arquivoSincronizador;  						//Armazena o numero do ultimo arquivo modificado
int indicesEscritor[QUANTIDADE];				//Armazena os numeros dos arquivos que serão escritos
int indicesLeitor[QUANTIDADE];          		//Armazena os numeros dos arquivos que serão lidos
//------------------------------------------------------------------------------------------------------
int main(){
	int indice,a=0, b=1, c=2, d=3;
	srand( (unsigned)time(NULL) );
	//Sorteia os numeros dos arquivos a serem lidos e salva no vetor
	for(indice=0;indice<QUANTIDADE;indice++){ 
		indicesLeitor[indice]=rand()%3;
		printf("L%d:%d---",indice,indicesLeitor[indice]);
	}
	//Sorteia os numeros dos arquivos a serem escritos e salva no vetor
	printf("\n");
	for(indice=0;indice<QUANTIDADE;indice++){ 
		indicesEscritor[indice]=rand()%3;
		printf("E%d:%d---",indice,indicesEscritor[indice]);
	}
	//Cria o mutex da regiao critica
	pthread_mutex_init( &(regiaoCritica), NULL);
	//Cria os mutex dos escritores
	for(indice=0;indice<4;indice++){ 
		pthread_mutex_init(&(mutexEscritores[indice]),NULL);
	}
	//Cria as threads dos Escritores
	pthread_create(&threadsEscritores[0],NULL,escritores,(void *)&(a));
	pthread_create(&threadsEscritores[1],NULL,escritores,(void *)&(b));
	pthread_create(&threadsEscritores[2],NULL,escritores,(void *)&(c));
	pthread_create(&threadsEscritores[3],NULL,escritores,(void *)&(d));
	//Cria as threads dos Leitores
	pthread_create(&threadsLeitores[0],NULL,leitores,(void *)&(a));
	pthread_create(&threadsLeitores[1],NULL,leitores,(void *)&(b));
	pthread_create(&threadsLeitores[2],NULL,leitores,(void *)&(c));
	pthread_create(&threadsLeitores[3],NULL,leitores,(void *)&(d));
	
	pthread_join(threadsEscritores[0], NULL);
	pthread_join(threadsEscritores[1], NULL);
	pthread_join(threadsEscritores[2], NULL);
	pthread_join(threadsEscritores[3], NULL);
	pthread_join(threadsLeitores[0], NULL);
	pthread_join(threadsLeitores[1], NULL);	
	pthread_join(threadsLeitores[2], NULL);
	pthread_join(threadsLeitores[3], NULL);	

	//Destroi as threads e os mutexs
	pthread_mutex_destroy( &(regiaoCritica) );
	for(indice=0;indice<QUANTIDADE;indice++){ 
		pthread_mutex_destroy(&(mutexEscritores[indice]));
	}
	pthread_exit(NULL);
}
//-------------------------------FUNÇÃO QUE SINCRONIZA OS ARQUIVOS-------------------------------------------------------
void sincronizar(){
	FILE *leitura;    											 //Ponteiro para leitura do arquivo
	FILE *escrita1, *escrita2; 									//Ponteiro para escrita nos arquivos
	char texto[50];
	sincroniza=true;											//Indica que esta sincronizando
	leitura=fopen(nomeArquivo[arquivoSincronizador],"r");  		//Abre o arquivo recentemente modificado
	escrita1=fopen(nomeArquivo[ARQ_ANTERIOR],"w");				//Abre o arquivo que será atualizado
	escrita2=fopen(nomeArquivo[ARQ_PROXIMO],"w");				//Abre o arquivo que será atualizado
	printf("\n-SINCRONIZADOR: Estou sincronizando os arquivos");
	while(!feof(leitura)){										//Será lido enquanto não for o final do arquivo
		fgets(texto,50,leitura); 								//Ler o texto
		fprintf(escrita1,"%s",texto); 							//Escreve o conteúdo lido em um dos arquivos;
		fprintf(escrita2,"%s",texto); 							//Escreve o conteúdo lido em um dos arquivos;
	}
	fclose(leitura); 											//Fecha o arquivo
	fclose(escrita1); 											//Fecha o arquivo 
	fclose(escrita2);											//Fecha o arquivo
	sincroniza=false;											//Indica que já sincronizou
	printf("\n-SINCRONIZADOR:Os arquivos ja foram sincronizados");
}
//----------------------------------FUNÇÃO DA THREAD ESCRITOR---------------------------------------------------------------
void *escritores(void * arg){
	int *numThread= (int *) arg;
	int indice, idArquivo;
	while(1){
		idArquivo= indicesEscritor[(*numThread)];  		 //Número do arquivo a ler escrito
		tentaEscrever(idArquivo, (*numThread));			//Verifica se pode escrever no arquivo sorteado
		modificarArquivo((*numThread),idArquivo);      //Modifica o arquivo, através da escrita
	}
}
//--------------------------------------------------------------------------------------------------------------------------
void tentaEscrever(int idArquivo, int numThread ){
	pthread_mutex_lock(&(regiaoCritica));                         //Entra na regiao Critica   
	estadosEscritores.estado[numThread]= QUER_ESCREVER;			 //Muda o estado do escritor atual para QUER_ESCREVER
	estadosEscritores.arquivo[numThread]=idArquivo;				//Salva o número do arquivo que escritor atual irá escrever
	printf("\n-ESCRITOR %d: Desejo escrever no arquivo %s",numThread,nomeArquivo[idArquivo]);
	analisarEstado(idArquivo,numThread);						//Verificaa se há escritores escrevendo ou sincronizadores ativos
	pthread_mutex_unlock(&(regiaoCritica));                      //Sai da regiao Critica
	pthread_mutex_lock( &(mutexEscritores[numThread]) );         //Bloqueia o escritor se não conseguir escrever
}
//--------------------------------------------------------------------------------------------------------------------------
void analisarEstado(int idArquivo, int numThread){ 
	int indice=0;    //Variável contadora para o loop
	int ocupado=0;  //Indica se algum aquivo está sendo escrito ou sincronizado. ocupado=0 não está e ocupado=1 está. 
  	while(ocupado!=1 && indice<QUANTIDADE){
	  	if(estadosEscritores.estado[numThread]==QUER_ESCREVER && indice!=numThread && estadosEscritores.estado[indice]==ESCREVENDO){ //Se houver escritor ativo no momento
	  		ocupado=1;                                                                              //Muda o valor da variavel ocupado para 1
	  		printf("\n-ESCRITOR %d: No momento, o escritor %d esta escrevendo",numThread,indice);
		} else if(sincroniza){             //Se estiver havendo sincronização nos arquivos
		  	ocupado=1;                    //Muda o valor da variavel ocupado para 1                    
		  	printf("\n-ESCRITOR %d: No momento, os arquivos estao sendo sincronizados", numThread);
		}
		indice++; //Incrementa a váriavel contadora
	}
	if(ocupado==0){ //Se não está sendo realizada sincronização ou escrita nos arquivos
	  	estadosEscritores.estado[numThread]=ESCREVENDO; //Altera o estado do escritor para escrevendo
	  	pthread_mutex_unlock( &(mutexEscritores[numThread]) );   //Desbloqueia o escritor
	}
}
//------------------------FUNCAO QUE MODIFICA O ARQUIVO--------------------------------------------------------------------
void modificarArquivo(int numThread, int idArquivo){
  	FILE *ponteiroEscrita;				 //Ponteiro para arquivo
  	arquivoSincronizador=idArquivo;     //Armazena o valor do arquivo que será escrito
  	int segundos;
  	pthread_mutex_lock(&(regiaoCritica));   //Entra na região critica
	ponteiroEscrita=fopen(nomeArquivo[idArquivo],"a"); //Abre o arquivo em modo leitura/escrita
	if(ponteiroEscrita==NULL){ //Verifica se houve erro ao abrir o arquivo 
		estadosEscritores.estado[numThread]=JA_ESCREVI; 
		printf("\n-ESCRITOR %d: Erro ao abrir o arquivo",numThread); //E printa mensagem de erro
	}
	else{
		fprintf(ponteiroEscrita,"\nSou o Escritor %d, estou escrevendo.",numThread); //Escreve uma frase no arquivo
		fclose(ponteiroEscrita); 													 //Fecha o arquivo
		printf("\n-ESCRITOR %d: Estou escrevendo no arquivo %s",numThread,nomeArquivo[idArquivo]);
		estadosEscritores.estado[numThread]=JA_ESCREVI;                              //Muda o estado do escritor
		printf("\n-ESCRITOR %d: Acabei de escrever no arquivo %s",numThread,nomeArquivo[idArquivo]);
		sincroniza=true;        //Indica que tem sincronizador ativo      
		sincronizar();          //Chama a função para sincronizar os arquivos
	}
	pthread_mutex_unlock(&(regiaoCritica));   //Sai da região crítica
	segundos= 1+(rand()%10);
	sleep(segundos);						//Dorme por alguns segundos	
}
//-------------------------------FUNCAO DA THREAD LEITOR--------------------------------------------------------------------------------
void *leitores(void * arg){
	int *numThread= (int *) arg; //Armazena o número da thread
	printf("\nO LEITOR %d foi criado",*numThread);
	int numArquivo, haEscritor, segundos;
	while(1){
		numArquivo=indicesLeitor[(*numThread)];	       //Número do arquivo a ser lido
		printf("\n-LEITOR %d: deseja ler o arquivo %s",(*numThread),nomeArquivo[numArquivo]);
		haEscritor=checarEscritor(numArquivo,(*numThread));  //Checa se tem escritor escrevendo no mesmo arquivo ou sincronizadores ativos
		if(haEscritor==0){                           //Se não tiver
			leitura(numArquivo,(*numThread));        //Realiza a leitura do arquivo
		}
		segundos= 1+(rand()%10);
		sleep(segundos);       //Dorme por um tempo
	}	
}
//---------------FUNCAO QUE CHECA SE TEM ESCRITOR NO MESMO ARQUIVO DESEJADO-------------------------------------------
int checarEscritor(int numArquivo,int numThread){ //Retorna 0 se não tem escritor no mesmo arquivo ou sincronizadores e 1 se tiver
  	int indice=0;		//Armazena o número da thread
  	int escrevendo=0;  //Indica se algum aquivo está sendo escrito ou sincronizado. escrevendo=0 não está e escrevendo=1 está.
  	while(indice<QUANTIDADE && escrevendo==0){
	  	if(estadosEscritores.estado[indice]==ESCREVENDO && estadosEscritores.arquivo[indice]==numArquivo){ //Se houver escritor escrevendo no arquivo desejado
	  		escrevendo=1;  				//Muda o valor da variavel escrevendo para 1
	  		printf("\n-LEITOR %d: O arquivo desejado esta sendo escrito",numThread);
		} else if(sincroniza){ //Se tiver sincronizador ativo
		  	escrevendo=1;	 //Muda o valor da variável para 1
		  	printf("\n-LEITOR %d: No momento, esta sendo sincronizado", numThread);
		  }
		  indice++;  //Incrementa a variável contadora
	}
 	 return escrevendo;   
}
//------------------------------------LEITURA-----------------------------------------------------------------------
void leitura(int numArquivo,int numThread){
	char texto[50];
	FILE *ponteiroLeitura;
	ponteiroLeitura=fopen(nomeArquivo[numArquivo],"r");              //Abre o arquivo em modo leitura
	if(ponteiroLeitura==NULL){ 								        //Verifica se houve erro ao abrir o arquivo 
		printf("\n-LEITOR %d: Erro ao abrir o arquivo",numThread); //e printa mensagem de erro
	}
	else{
		printf("\n-LEITOR %d: Estou lendo o arquivo %s",numThread,nomeArquivo[numArquivo]); 
		do{																		//Será lido enquanto não for o final do arquivo
			fgets(texto,50,ponteiroLeitura); 								   //Ler o texto
			printf("\n\t-Leitor %d: Texto do arquivo: %s",numThread, texto); //Imprime na tela o conteúdo lido;
		}while(feof(ponteiroLeitura)==0);
		fclose(ponteiroLeitura); 											//Fecha o arquivo
		printf("\n-LEITOR %d: Finalizei a leitura do arquivo %s",numThread,nomeArquivo[numArquivo]);
	}	
}
