#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "MyHeader.h"
#include <time.h>

int random_ID();
int random_Class();
int random_Number();

int main(int argc, char **argv)
{	
	int w=getpid();
	int a=0;
	int b=0;
	int sd;
	struct sockaddr saddr;
	int r;
	char buffer[BSIZE];
	reservation newRes;

	/*Getting a socket handler*/
	sd=socket(AF_UNIX,SOCK_STREAM,0);
	if(sd<0)
	{
		printf("Error in creating socket.\n");
		exit(1);
	}

	/*Specify socket address details*/
	saddr.sa_family = AF_UNIX;
	strcpy(saddr.sa_data,SOCKET_NAME);
	
	//Attempt to connect to server
	r=connect(sd,&saddr,sizeof(saddr));
	if(r<0)
	{
		printf("Failed to connect to server.\n");
		exit(1);
	}
	
	printf("RANDOM RESERVATION PROCEEDED! -->");
	
	//Random seat's class
	b=random_Class(w);
	if(b==1) { a=0;}
	else if(b==2 || b==3) {a=1;}
	else if(b==4 || b==5 || b==6) {a=2;}
	else if(b==7 || b==8 ||b==9 || b==10) {a=3;}
	newRes.info[0]=a;
	newRes.info[1]=random_Number(w);
	newRes.info[2]=random_ID(w);
	printf("%d %d %d\n",newRes.info[0],newRes.info[1],newRes.info[2]);

	memset(buffer,'\0',BSIZE);

	sprintf(buffer,"%d %d %d ", newRes.info[0], newRes.info[1], newRes.info[2]);

	/*send order*/
	write(sd,&newRes,sizeof(reservation));
	/*read confirmation*/
	memset(buffer,'\0',BSIZE);
	read(sd,buffer,BSIZE);
	
	printf("--------------------------------------------\n%s\n--------------------------------------------\n", buffer);
	
	close(sd);
	exit(0);
}

int random_ID(int cc)
{
	srand ( time(NULL)*cc );
	int x = rand() %9999999 +1000000 ; // 
	cc++;
	return(x);
}
int random_Class(int cc)
{
	srand ( time(NULL)*cc );
	int x = rand() %10+1 ;
	cc++;
	return(x);
}
int random_Number(int cc)
{
	srand ( time(NULL)*cc+5 );
	int x = rand() %4 + 1;
	cc++;
	return(x);
}
