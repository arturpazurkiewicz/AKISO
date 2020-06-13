#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
int sigcnt = 0;
int sig = 0;
void sighandler(int signum) { // Procedura obsługi sygnału
 printf(" %d \n",signum);
 
}
int main() {
 printf(" %d \n",getpid());
 printf("Start programu \n");
 int i = 0;
 do
 {
 	i++;
 	signal(i, sighandler);
 } while (i<64);
 while (1);

 return 0;
} 
