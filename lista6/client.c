#include <netdb.h> 
#include <stdio.h> 
#include <unistd.h>
#include <stdlib.h> 
#include <string.h>
#include <fcntl.h> 
#include <sys/socket.h> 
#define MAX 1024 
#define SA struct sockaddr
 
char buff[MAX];
int key;

int getKey(int fd)
{
    char encBuff[128];
    int p = getRandPrime();
    int g = getRandom(p/2, p);
    int a = getRandom(10, p/2);
    sprintf(encBuff, "%d", p);
    write(fd, encBuff, sizeof(encBuff));
    bzero(encBuff, sizeof(encBuff));
    sprintf(encBuff, "%d", g);
    write(fd, encBuff, sizeof(encBuff));
    bzero(buff, sizeof(buff));
    sprintf(encBuff, "%d", modPow(g,a,p));
    write(fd, encBuff, sizeof(encBuff));
    bzero(encBuff, sizeof(encBuff));
    read(fd, encBuff, sizeof(encBuff));
    int B = atoi(encBuff);
    return modPow(B, a, p);
}

void code(char* data, int size, int key){
    for(int i = 0; i < size; i++){
        data[0] = (char)((int)data[0] + key);
    }
}

int getRandom(int min, int max) {
    return rand() % (max - min) + min;
}

int isPrime(int a) {
    for (int i = 2; i * i <= a; i++) {
        if (a % i == 0) return 0;
    }
    return 1;
}

int getRandPrime() {
    int x = rand() % 1024 + 1024;	
    do {
		x--;
    } while (!isPrime(x));
    return x;
}

int modPow(int a, int b, int n)
{
    int res = 1;
    while(b > 0){
        res *= a;
        res %= n;
        b--;
    }
    return res;
}

void readFromServer(int sockfd) {
	bzero(buff, sizeof(buff)); 
    read(sockfd, buff, sizeof(buff));
    code(buff, sizeof(buff), -key);
}

int file_exist (char *filename)
{
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

void writeToServer(int sockfd){
    code(buff, sizeof(buff), key);
	write(sockfd, buff, sizeof(buff)); 
    bzero(buff, sizeof(buff));
}


void communicate(int sockfd) 
{     
    int n;
    key = getKey(sockfd);
    printf("Klucz: %d", key);
    while (1) {         
        n = 0; 
		bzero(buff, sizeof(buff)); 
		printf("\nKomenda> ");
        while ((buff[n++] = getchar()) != '\n');
		buff[n-1] = '\0';
		if ((strncmp(buff, "exit", 4)) == 0){
			printf("Klient wyszedł...\n");
			writeToServer(sockfd);
			break;
		}
		else if ((strncmp(buff, "lcd", 3)) == 0){
			chdir(buff+4);
			bzero(buff, sizeof(buff));
			getcwd(buff, sizeof(buff));
			printf("Aktualny katalog: %s", buff);
		}
		else if ((strncmp(buff, "lpwd", 4)) == 0){
			bzero(buff, sizeof(buff));
			getcwd(buff, sizeof(buff));
			printf("Aktualny katalog: %s", buff);
		}
		else if ((strncmp(buff, "lls", 3)) == 0){
			int pid;
			if((pid = fork()) == 0)
			{
				execlp("ls", "ls", NULL);
			}
			wait(&pid);
		}
		else if (strncmp("get", buff, 3) == 0) { 
			FILE* fp = fopen( buff+4, "wb");
			writeToServer(sockfd);
			readFromServer(sockfd);
			if (strncmp("OK", buff, 2) == 0){
				printf("Pobieranie..");
				if(fp != NULL){
					readFromServer(sockfd);
				    while( strncmp("EOF", buff, 3) != 0 ) {
						fprintf(fp, "%s", buff);
						readFromServer(sockfd);		            
				    }
					fprintf(fp, "\0");
				    fclose(fp);
					printf("Skończone");
				} else {
				    perror("File");
				}
			}
			else if (strncmp("FAIL", buff, 4) == 0) printf("Plik nie istnieje");
			else printf("%s", buff);
		} 
		else if (strncmp("put", buff, 3) == 0) { 			
			if( file_exist(buff+4) ) {
				FILE *fp = fopen(buff+4, "rb");
				
				writeToServer(sockfd);	
				readFromServer(sockfd);
				if (strncmp("OK", buff, 2) == 0){
					printf("Wysyłanie pliku..");				
					if(fp == NULL){
						close(sockfd); 
						perror("Błąd");
						exit(-1);
					}
					while( fread(buff, 1, sizeof(buff), fp) > 0 ){
						writeToServer(sockfd);
					}	
					fclose(fp);
					strcpy(buff, "EOF");
					writeToServer(sockfd);
					readFromServer(sockfd);
					printf("%s", buff);
				}
				else if (strncmp("FAIL", buff, 4) == 2) printf("Nie mogę wysłać pliku");
				else printf("%s", buff);
			} else {
				printf("Plik nie istnieje");
			}
			
		}
		else {
			writeToServer(sockfd);
			readFromServer(sockfd);
			printf("%s", buff);
		}
    }	
} 
  
int main() 
{
    srand(time(0));
    int sockfd, connfd; 
    struct sockaddr_in servaddr, cli; 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("Błąd tworzenia socketu...\n"); 
        exit(0); 
    } 
    else
        printf("Utworzenie socketu..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
  
    char address[16] = "127.0.0.1";
	int port = 8084;
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = inet_addr(address); 
    servaddr.sin_port = htons(port); 

    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("Nie połączono się z serwerem...\n"); 
        exit(0); 
    } 
    else
        printf("Połączono się z serwerem..\n"); 

    communicate(sockfd); 
    close(sockfd); 
} 

