#include<stdio.h>
#include <stdlib.h>

char nomeArquivo[3][15]{
	"documento1.txt", "documento2.txt",	"documento3.txt"
};
void lerArquivo(int numThread){
	char texto[50];
	FILE *ponteiroLeitura;
	int indice= rand()% 3;
	
	printf("\nSou a Thread %d, estou lendo o documento %d",numThread,indice+1);
	
	ponteiroLeitura=fopen(nomeArquivo[indice],"r");
	do{
		fgets(texto,50,ponteiroLeitura);
	//	printf("\n%s",texto);
	}while(feof(ponteiroLeitura)==0);
	fclose(ponteiroLeitura);	
}
void escreverArquivo(int numThread){
	FILE *ponteiroEscrita;
	int indice= rand()% 2;
	printf("\nSou a Thread %d, estou escrevendo no arquivo %d",numThread,indice+1);
	ponteiroEscrita=fopen(nomeArquivo[indice],"a");
	fprintf(ponteiroEscrita,"\nSou a Thread %d, estou escrevendo.",numThread,indice);
	fclose(ponteiroEscrita);
}

int main(){
	int i=1;
	for(i=0;i<4;i++){
		lerArquivo(i);
	}
	for(i=0;i<4;i++){	
		escreverArquivo(i);
	}
	for(i=0;i<4;i++){
		lerArquivo(i);
	}
}
