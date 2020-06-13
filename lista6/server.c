#include <pthread.h>
#include <unistd.h>
#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <fcntl.h>
#include <signal.h> 
#include <sys/socket.h> 
#include <sys/types.h>
#include <time.h>
#define MAX 1024 
#define PORT 8084
#define SA struct sockaddr 
  
int sockfd;
char* password = "Ja";

int getKey(int fd)
{
    char buff[128];
    bzero(buff, sizeof(buff));
    read(fd, buff, sizeof(buff));
    int p = atoi(buff);
    bzero(buff, sizeof(buff));
    read(fd, buff, sizeof(buff));
    int g = atoi(buff);
    bzero(buff, sizeof(buff));
    read(fd, buff, sizeof(buff));
    int A = atoi(buff);
    int b = getRandom(10, p/2);
    sprintf(buff, "%d", modPow(g, b, p));
    write(fd, buff, sizeof(buff));
    return modPow(A, b, p);
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


int getRandom(int min, int max) {
    return rand() % (max - min) + min;
}

int file_exist (char *filename)
{
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

void c_handler(int sig)
{
	printf("Zamykam serwer");
	close(sockfd);
	exit(0);
}

void code(char* data, int size, int key){
    for(int i = 0; i < size; i++){
        data[0] = (char)((int)data[0] + key);
    }
}

void readFromClient(int sockfd, char* buff, int size, int key) {
    bzero(buff, size);
    read(sockfd, buff, size);
    code(buff, size, -key);
}

void writeToClient(int sockfd, char* buff, int size, int key){
    code(buff, size, key);
    write(sockfd, buff, size);
    bzero(buff, size);
}

// Funkcja odpowiedzialna za porozumiewanie się serwera z klientem
void* communicate(void* args) 
{ 
	int fd = *(int*) args;
	int key = getKey(fd);
	printf("Id klucz %d", key);
    char buff[MAX] = "Cześć :)"; 
	char path[MAX];
	getcwd(path, sizeof(path));
    int n; 
	for (;;) {
        readFromClient(fd, buff, sizeof(buff), key);
		if (strncmp("exit", buff, 4) == 0) { 
            printf("Wyjście z serwera...\n"); 
            close(fd);
			pthread_exit(NULL); 
        } 
		else if (strncmp("log", buff, 3) == 0) { 
            if (strncmp(password, buff+4, strlen(password)) == 0){
				strcpy(buff, "Logowanie się powiodlo :)");
                writeToClient(fd, buff, sizeof(buff), key);
				break;
			} else {
				strcpy(buff, "Logowanie się nie powiodło :(");
                writeToClient(fd, buff, sizeof(buff), key);
			}
        }
		else {
			strcpy(buff, "Aby się zalogować wpisz: log <password>");
            writeToClient(fd, buff, sizeof(buff), key);
		}
	}
    for (;;) {
        readFromClient(fd, buff, sizeof(buff), key);
		if (strncmp("pwd", buff, 3) == 0) {
            writeToClient(fd, path, sizeof(buff), key);
		}
		else if (strncmp("ls", buff, 3) == 0) {
		    //Warning
			int pid;
			int pipes[2];
			pipe(pipes);
			if((pid = fork()) == 0)
			{
				chdir(path);
				dup2(pipes[1], 1);
				close(pipes[0]);
                close(pipes[1]);
				execlp("ls", "ls", NULL);
			}
            close(pipes[1]);
			read(pipes[0], buff, sizeof(path));
            close(pipes[0]);
			wait(&pid);
			writeToClient(fd, buff, sizeof(buff), key);
		}
        else if (strncmp("cd", buff, 2) == 0) { 
			char* end = strchr(buff, '\n');
			if(end > buff) *end = '\0';
			chdir(path);
			chdir(buff+3);
			getcwd(path, sizeof(path));
            writeToClient(fd, path, sizeof(buff), key);
		}
		else if (strncmp("put", buff, 3) == 0) { 			
			FILE* fp = fopen( buff+4, "wb");
		    if(fp != NULL){
				strcpy(buff, "OK");
                writeToClient(fd, buff, sizeof(buff), key);
                readFromClient(fd, buff, sizeof(buff), key);	
		        while( strncmp("EOF", buff, 3) != 0 ) {
					fprintf(fp, "%s", buff);
                    readFromClient(fd, buff, sizeof(buff), key);			            
		        }
				fprintf(fp, "\0");
		        fclose(fp);
		    } else {
				strcpy(buff, "FAIL");
                writeToClient(fd, buff, sizeof(buff), key);
		        perror("File");
		    }
			printf("Pobieranie pliku się powiodło");
			strcpy(buff, "Wysłano plik");
            writeToClient(fd, buff, sizeof(buff), key);
		} else if (strncmp("get", buff, 3) == 0) { 
			if( file_exist(buff+4) ) {
				int b;
				FILE *fp = fopen(buff+4, "rb");
				bzero(buff, sizeof(buff));
				strcpy(buff, "OK");
                writeToClient(fd, buff, sizeof(buff), key);
				bzero(buff, sizeof(buff));
				while( (b = fread(buff, 1, sizeof(buff), fp)) > 0 ){
					printf("%s\n", buff);
                    writeToClient(fd, buff, sizeof(buff), key);
    				bzero(buff, sizeof(buff));
				}	
				fclose(fp);
				strcpy(buff, "EOF");
                writeToClient(fd, buff, sizeof(buff), key);
    			bzero(buff, sizeof(buff));
				printf("Wysłanie pliku się powiodło");
			} else {
				strcpy(buff, "FAIL");
                writeToClient(fd, buff, sizeof(buff), key);
			}
			
		}
        else if (strncmp("exit", buff, 4) == 0) { 
            close(fd);
			pthread_exit(NULL); 
        } 
		else {
			strcpy(buff, "Nieznana komenda");
            writeToClient(fd, buff, sizeof(buff), key);
		}		
    }	 	
} 
int main() 
{
    srand(time(0));
	signal(SIGINT, c_handler);
    int connfd, len; 
    struct sockaddr_in servaddr, cli; 
  
    // Tworzenie i weryfikacja socketu
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("Błąd tworzenia socketu...\n"); 
        exit(0); 
    } 
    else
        printf("Utworzono socket..\n"); 
    bzero(&servaddr, sizeof(servaddr)); 
  
    // IP, PORT 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(PORT); 
  
    // Wiązanie nowo utworzonego socketu z danym adresem IP i weryfikacja 
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
        printf("Błąd wiązania socketów...\n"); 
        exit(0); 
    } 
    else
        printf("Połączono socket..\n"); 
  
    // Serwer nasłuchuje i weryfikuje 
    if ((listen(sockfd, 5)) != 0) { 
        printf("Błąd nasłuchiwania...\n"); 
        exit(0); 
    } 
    else
        printf("Serwer nasłuchuje..\n"); 
    len = sizeof(cli); 
  
    // Akceptuj pakiet danych od klienta i zweryfikuj 
	while(1){
    	connfd = accept(sockfd, (SA*)&cli, &len); 
	    pthread_t t;
        pthread_create(&t, NULL, communicate, &connfd);
	}
  	close(sockfd);     
} 

