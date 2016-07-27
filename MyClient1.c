/*
#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> */
#include "MyHeader.h"

int main(int argc, char **argv)
{
	int sd;
	struct sockaddr saddr;
	int r;
	char buffer[BSIZE];
	reservation newRes; //creating a new struct: reservation
	
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
	

	printf("Enter Seats Class and Number. Then your card ID :\n");
	printf("Class-0:A, 1:B, 2:C, 3:D,  Seats Number:1-4  \n");
	printf("eg.: 0 1 (0 A-Class ticket) 56468 \n");

	/*read user type*/
	memset(buffer,'\0',BSIZE);
	fgets(buffer,BSIZE,stdin);
	
		
	sscanf(buffer,"%d %d %d ", newRes.info, newRes.info+1, newRes.info+2);

	/*send order*/
	write(sd,&newRes,sizeof(reservation));
	
	if(fork()==0)
	{
		for(;;){
			sleep(WAIT_TIME);
			printf("Sorry for the delay");
		}
	}
	
	/*read confirmation*/
	memset(buffer,'\0',BSIZE);
	read(sd,buffer,BSIZE);
	
	printf("--------------------------------------------\n%s\n--------------------------------------------\n", buffer);
	
	close(sd);
	exit(0);
}
