#include "MyHeader.h"

/*===========================Signal Handlers============================*/
/*Function for handling children termination.
Added to avoid zombie processes.*/
void sig_chld(int signo);
/*Function for handling server termination.
Added to release resources.*/
void sig_int(int signo);
/*Function for informing children for complete reservations.*/
void sig_res(int signo);
/*===========================Signal Handlers============================*/

/*===========================Misc Functions============================*/
/* A function that returns a random number between 1 and 10.*/
int percent_value(); 
/*Function for initializing shared memory data*/
void init_data();
/*Function for closing all semaphores. Called by children and parent.*/
void semaphores_close();
/*Function for unlinking semaphores. Called by parent only.*/
void semaphores_unlink();
/*===========================Misc Functions============================*/

//Pointer to shared memory
data* sh_mem;
//Shared memory handler
unsigned int sh_mem_id;

//====================================== Main===========================================//
int main(int argc, char **argv)
{
	int sd;
	struct sockaddr saddr;
	int r, i, j, m, n;
	struct sockaddr inaddr;
	unsigned int inlen;
	unsigned int conn;
	int child_id;
	reservation newRes;
	char buffer[BSIZE];
	struct timeval stop;
	
	
	//Defining signal handlers 
	signal(SIGCHLD,sig_chld);
	signal(SIGINT,sig_int);

	//Shared memory creation 
	sh_mem_id=shmget((key_t)SH_MEM_NAME,(size_t) sizeof(data),(int) (0600|IPC_CREAT));
	if(sh_mem_id<0)
	{
		printf("Failed to create shared memory.\n");
		exit(1);
	}
	
	//Attach server to shared memory 
	sh_mem=(data*)shmat(sh_mem_id,NULL,0);
	if(sh_mem==(data*)-1)
	{
		printf("Failed to attach server to shared memory.\n");
		exit(1);
	}
	
	//Initialize shared data. We use the arguments defined in Myheader.h.
	init_data(NUMBER_OF_OPERATORS, NUMBER_OF_TERMINALS, A_SEATS, B_SEATS, C_SEATS, D_SEATS);
	
/*///////////////////////////////////////////////////////////////////////////////////////////////
						THEATER CHECK -PROCESS 

After next line we create a new process that it continously checks,if all class get empty.
When this happen it set 'theater status' into full.
In the end, it prints several information about the reservations,
such as a foreground of the theater.

This process runs simultaneously with its parent.
//////////////////////////////////////////////////////////////////////////////////////////////*/
	if ( fork() == 0 ) { 
	//Only Child process hets in here.	
	
		//Child gets shared memory handler
		sh_mem_id=shmget((key_t)SH_MEM_NAME,sizeof(data),0600); 
		if(sh_mem_id<0)
		{
			printf("Child failed to retrieve shared memory handler.\n");
			exit(1);
		}
		//Child is attached to shared memory.
		sh_mem=(data*)shmat(sh_mem_id,NULL,0);
		if(sh_mem==(data*)-1)
		{
			printf("Failed to attach child to shared memory.\n");
			exit(1);
		}
		
		//Infinite loop used to check if theater gets Full.
		while(1){
			
			/*Check every class if they are empty*/
			if(sh_mem->Theater[1]==0 && sh_mem->Theater[2]==0 && sh_mem->Theater[3]==0 && sh_mem->Theater[4]==0 ){
						sh_mem->Theater[0]=1; //If all classes are empty, set 'theater status' full. 
						sleep(2); // 2 secands delay. 
					}
			/*Check if theater is full*/
			if(sh_mem->Theater[0]==1){
				printf("===================================================================");
				printf("\nTheater got FULL. \nTheater server will now close.\n");
				int a=((double)sh_mem->com/(sh_mem->com+sh_mem->incom))*100;
				int b=((double)sh_mem->incom/(sh_mem->com+sh_mem->incom))*100;
				printf("\n---Completed Reservations:%d  Incompleted:%d \n",sh_mem->com,sh_mem->incom);
				printf("Percent of completed reservations: %d%% \n",a);
				printf("Percent of incompleted reservations: %d%% \n",b);				
				printf("Theater Income=%d E.\n",sh_mem->Theater_account);
				printf("--Theater reservations's foreground--	\n");
				
				/*--------Printing the foreground of Class A ------*/
				printf("-Class A: \n");
				//checking every reservation
				for(m=0; m< NUM_RES; m++){ 
					//Checking if this reservation is about Class A &&
					//if this reservation is done.
					if(sh_mem->reservations[m].info[0]==0 && sh_mem->reservations[m].info[5]==1){
						//Printing the reservation's number: one time per seat
						for (i=0; i<=sh_mem->reservations[m].info[1]; i++);{
							printf("[%d]",m);
						}
					}
				}	
				/*--------Printing the foreground of Class B ------*/
				printf("\n-Class B: \n");
				//checking every reservation
				for(m=0; m< NUM_RES; m++){
					//Checking if this reservation is about Class A &&
					//if this reservation is done.
					if(sh_mem->reservations[m].info[0]==1 && sh_mem->reservations[m].info[5]==1){
						//Printing the reservation's number: one time per seat
						for (n=0; n<=sh_mem->reservations[m].info[1]; n++);{
							printf("[%d]",m);
						}
					}
				}	
				/*--------Printing the foreground of Class C ------*/
				printf("\n-Class C: \n");
				//checking every reservation
				for(m=0; m< NUM_RES; m++){
					//Checking if this reservation is about Class A &&
					//if this reservation is done.
					if(sh_mem->reservations[m].info[0]==2 && sh_mem->reservations[m].info[5]==1){
						//Printing the reservation's number: one time per seat
						for (i=0; i<=sh_mem->reservations[m].info[1]; i++);{
							printf("[%d]",m);
						}
					
					}
				}	
				/*--------Printing the foreground of Class D ------*/
				printf("\n-Class D: \n");
				//checking every reservation
				for(m=0; m< NUM_RES; m++){
					//Checking if this reservation is about Class A &&
					//if this reservation is done.
					if(sh_mem->reservations[m].info[0]==3 && sh_mem->reservations[m].info[5]==1){
						//Printing the reservation's number: one time per seat
						for (i=0; i<=sh_mem->reservations[m].info[1]; i++);{
							printf("[%d]",m);
						}
					}
				}
				printf("\n");
				printf(" Press ctrl-c to close server....");
				break;	//breaking while
			}
		}
		exit(0);	//End of process
	}
	
/*///////////////////////////////////////////////////////////////////////////////////////////////
						TRANSFER MONEY -PROCESS 

After next line we create a new process that is transfering the income of the reservations.
from Company_account to Theater_account.

This process runs simultaneously with its parent.
//////////////////////////////////////////////////////////////////////////////////////////////*/
	if ( fork() == 0 ) { 
	//Only Child process hets in here.
		//Child gets shared memory handler
		sh_mem_id=shmget((key_t)SH_MEM_NAME,sizeof(data),0600); 
		if(sh_mem_id<0)
		{
			printf("Child failed to retrieve shared memory handler.\n");
			exit(1);
		}
		
		//Child is attached to shared memory.
		sh_mem=(data*)shmat(sh_mem_id,NULL,0);
		if(sh_mem==(data*)-1)
		{
			printf("Failed to attach child to shared memory.\n");
			exit(1);
		}
		
		
		//Infinite loop used to keep the process opened.
		while(1){
			if(sh_mem->Theater[0]==0){
			sleep(TRANSFER_TIME); 	// <TRANSFER_TIME> between each transfer.
			sem_wait(sh_mem->res_sem);	//locking res_sem to make the transfer
			printf("\n***********************************\n");
			printf("Funds transfered to Theater account: %d E",sh_mem->Company_account);
			sh_mem->Theater_account=sh_mem->Theater_account+sh_mem->Company_account;	//transfer
			sh_mem->Company_account=0;	//Initializing company's account
			printf("\n***********************************\n");
			sem_post(sh_mem->res_sem);	//leaving res_sem
			}
			else if(sh_mem->Theater[0]==1){
				break; // If theater got full, break 'while' and  end the process. 
			}
		}
		exit(0);	//End of process
	}
//---------------------------------------------------------------------------------------OPERATORS
/*///////////////////////////////////////////////////////////////////////////////////////////////
						OPERATORS -PROCESS 

We create <NUM_OF_OPERATORS> new processes, which act like the operators of our company.

These processes run simultaneously with their parent.
//////////////////////////////////////////////////////////////////////////////////////////////*/
	for (i=0; i<sh_mem->operators; i++) {
	//Only Child process hets in here.
		child_id=fork();
		if(child_id==0)
		{
			/*Child gets shared memory handler*/
			sh_mem_id=shmget((key_t)SH_MEM_NAME,sizeof(data),0600); 
			if(sh_mem_id<0)
			{
				printf("Child failed to retrieve shared memory handler.\n");
				exit(1);
			}
			
			/*Child is attached to shared memory.*/
			sh_mem=(data*)shmat(sh_mem_id,NULL,0);
			if(sh_mem==(data*)-1)
			{
				printf("Failed to attach child to shared memory.\n");
				exit(1);
			}
			
			//---------------------------------------------------------------------------------
			
			//Infinite loop.
			//If there are reservations, serve them.
			while(1) {
				sem_wait(sh_mem->oper_sem); 	//Locking oper_sem		
				/*Only one process gets in here each time*/
				if(sh_mem->res_head < sh_mem->res_tail && sh_mem->reservations[sh_mem->res_head].info[4]!=3 ) {

					int head = sh_mem->res_head;	//From now on use "head" as the process's ID
					
					//Inform the qeue of reservations ,which reservation is first in line.
					sh_mem->res_head++;		
					
					//Set in terminal's qeue,this process ID 
					sh_mem->term_tail++;
					
					sem_post(sh_mem->oper_sem);		//Leaving oper_sem  



					//From now on use "seatsClass" as the current reservation's class
					int seatsClass = sh_mem->reservations[head].info[0];	
					//From now on use "seatsNum" as the current reservation's Number of seats
					int seatsNum = sh_mem->reservations[head].info[1];	

					
					printf("Checking number of available seats...\n");   
					
					//Check if 'A' is our current reservation's class
					if (sh_mem->reservations[head].info[0]==0){ 
					//Check if available seats in Class A are enough for current reservation						
						if(sh_mem->reservations[head].info[1] >sh_mem->Theater[1]){
							printf ("No seat available in Class A\n");
							sh_mem->reservations[head].info[5]=3;//(info[5]=3)=No available seats in this Class.
						}
					}
					//Check if 'B' is our current reservation's class
					if (sh_mem->reservations[head].info[0]==1){ 
					//Check if available seats in Class B are enough for current reservation					

						if(sh_mem->reservations[head].info[1] >sh_mem->Theater[2]){
							printf ("No seat available in Class B\n");
							sh_mem->reservations[head].info[5]=3;//(info[5]=3)=No available seats in this Class.
						}
					}
					//Check if 'C' is our current reservation's class
					if (sh_mem->reservations[head].info[0]==2){ 
					//Check if available seats in Class C are enough for current reservation						

						if(sh_mem->reservations[head].info[1] >sh_mem->Theater[3]){
							printf ("No seat available in Class C\n");
							//If available seats aren't enough 
							sh_mem->reservations[head].info[5]=3; //(info[5]=3)=No available seats in this Class.
						}
					}
					//Check if 'D' is our current reservation's class
					if (sh_mem->reservations[head].info[0]==3){ 
					//Check if available seats in Class D are enough for current reservation						

						if(sh_mem->reservations[head].info[1] >sh_mem->Theater[4]){
							printf ("No seat available in Class D\n");
							sh_mem->reservations[head].info[5]=3;//(info[5]=3)=No available seats in this Class.
						}

					}
					
					// "Checking for Seats"
					sleep(SEATFIND_TIME); // <SEATFIND_TIME> delay
					
					/*----At this point seats are ckecked---------*/
					
					/*Cheking if we have any 'asnwer' from the Bank (terminals*/
					while(1){  
					if(sh_mem->reservations[head].info[4]==2){
						//(info[5]=3)=Checked for available seats and Consumer's bank account. 
						break;	
						}
					}
					
					/*-------------At this point seats and client's account are ckecked-----------------------*/
					
					
					/*Checking the value of info[5].
					-If its 2,then: wrong card_ID/Insufficient account's balance.
					-If its 3,then there are not enough seats in this Class.
					If none of these happen,we can continue in reserving the seats.		*/
					if(sh_mem->reservations[head].info[5] != 2 && sh_mem->reservations[head].info[5] != 3){ //dhladh entaksei me thn karta kai me theseis

						//---------------Check if 'A' is our current reservation's class
						if (sh_mem->reservations[head].info[0]==0){ 
							sem_wait(sh_mem->seat_sem);		//Locking seat_sem	
							//SECOND check if available seats in Class A are enough for current reservation.
							if(sh_mem->reservations[head].info[1] >sh_mem->Theater[1]){
								printf ("Seats taken in A class by another Operator.\n");
								sh_mem->reservations[head].info[5]=3;//(info[5]=3)=No available seats in this Class.
							}
							else{
								//Bring up to date Class's variable -> Theater[1]=Class A 
								sh_mem->Theater[1]=sh_mem->Theater[1]-seatsNum;
								//Counting the tickets's cost and saving it.
								//info[3]=Cost
								sh_mem->reservations[head].info[3]=sh_mem->reservations[head].info[1]*50;
								sh_mem->reservations[head].info[5]=1;//(info[5]=1)=Done. ->Current reservation is completed.
							}
							sem_post(sh_mem->seat_sem);		//Leaving seat_sem	
						}
						//---------------Check if 'B' is our current reservation's class
						if (sh_mem->reservations[head].info[0]==1){ 
							sem_wait(sh_mem->seat_sem);		//Locking seat_sem
							//SECOND check if available seats in Class B are enough for current reservation.							
							if(sh_mem->reservations[head].info[1] >sh_mem->Theater[2]){
								printf ("Seats taken in B class by another Operator.\n");
								sh_mem->reservations[head].info[5]=3;//(info[5]=3)=No available seats in this Class.
							}
							else{
								//Bring up to date Class's variable -> Theater[2]=Class B 
								sh_mem->Theater[2]=sh_mem->Theater[2]-seatsNum;
								//Counting the tickets's cost and saving it.
								//info[3]=Cost
								sh_mem->reservations[head].info[3]=sh_mem->reservations[head].info[1]*40;
								sh_mem->reservations[head].info[5]=1;//(info[5]=1)=Done. ->Current reservation is completed.
							}
							sem_post(sh_mem->seat_sem);		//Leaving seat_sem	
						}
						//---------------Check if 'C' is our current reservation's class
						if (sh_mem->reservations[head].info[0]==2){ 
							
							sem_wait(sh_mem->seat_sem);		//Locking seat_sem
							//SECOND check if available seats in Class C are enough for current reservation.							
							if(sh_mem->reservations[head].info[1] >sh_mem->Theater[3]){
								printf ("Seats taken in C class by another Operator.\n");
								sh_mem->reservations[head].info[5]=3;//(info[5]=3)=No available seats in this Class.
								
							}
							else{
								//Bring up to date Class's variable -> Theater[3]=Class C 
								sh_mem->Theater[3]=sh_mem->Theater[3]-seatsNum;
								//Counting the tickets's cost and saving it.
								//info[3]=Cost
								sh_mem->reservations[head].info[3]=sh_mem->reservations[head].info[1]*35;
								sh_mem->reservations[head].info[5]=1;//(info[5]=1)=Done. ->Current reservation is completed.
							}
							sem_post(sh_mem->seat_sem);		//Leaving seat_sem	
						}
						//---------------Check if 'D' is our current reservation's class
						if (sh_mem->reservations[head].info[0]==3){
							
							sem_wait(sh_mem->seat_sem);		//Locking seat_sem
							//SECOND check if available seats in Class D are enough for current reservation.							
							if(sh_mem->reservations[head].info[1] >sh_mem->Theater[4]){
								printf ("Seats taken in D class by another Operator.\n");
								sh_mem->reservations[head].info[5]=3;//(info[5]=3)=No available seats in this Class.
							}
							else{
								//Bring up to date Class's variable -> Theater[4]=Class D
								sh_mem->Theater[4]=sh_mem->Theater[4]-seatsNum;
								//Counting the tickets's cost and saving it.
								//info[3]=Cost
								sh_mem->reservations[head].info[3]=sh_mem->reservations[head].info[1]*30; 
								sh_mem->reservations[head].info[5]=1;//(info[5]=1)=Done. ->Current reservation is completed.
							}
							sem_post(sh_mem->seat_sem);		//Leaving seat_sem	
						}

						/*At this point, seats are saved.*/
							
							
						sh_mem->reservations[head].info[4]=3;//(info[4]=3)=Reservation process ended.
						
						
						if(sh_mem->reservations[head].info[5]==3){
							//if there weren't enough available seats in the class we wanted (info[5]=3) 
							//print a message in our screen
							printf("Reservation %d Incompleted:\nNot enough seats.(Seats taken by another Operator.\n",head);
						}
						else if(sh_mem->reservations[head].info[5]==1){
						//our reservations is completed 
						//(info[5]=1)=Done.
						
							gettimeofday(&stop, NULL);
							//Printing all the information we want to know about our current reservation 
							printf("Reservation Completed!\n");
							printf("Reservation Num:  %d, %d seats of class %d  %f msec\n", head, seatsNum, seatsClass, (float) (stop.tv_sec-sh_mem->reservations[head].reserveTime.tv_sec) * 1000000 + (stop.tv_usec-sh_mem->reservations[head].reserveTime.tv_usec));
						}
						
						//Printing Current available theater seats.
						printf("\nCurrent available theater seats: A:%d B:%d C:%d D:%d \n",sh_mem->Theater[1],sh_mem->Theater[2],sh_mem->Theater[3],sh_mem->Theater[4]);

					}	
					else{
					//In our first check there weren't enough seats 
					printf("Reservation %d Incompleted:\nNot enough seats or insufficient account's balance/inaccurate cardID. \n",head);
					sh_mem->reservations[head].info[4]=3;//(info[4]=3)=Reservation process ended.
					}
				}
				else {
					sem_post(sh_mem->oper_sem);//Leaving oper_sem 
					//Everytime there is no reservation in our qeue, leave oper_sem and wait to check again.
				}
			}
			exit(0);
		}
	}
// --------------------------------------------------------------------------------------TERMINALS
/*///////////////////////////////////////////////////////////////////////////////////////////////
						OPERATORS -PROCESS 

We create <NUM_OF_TERMINALS> new processes, which act like the terminals of the bank.

These processes run simultaneously with their parent.
//////////////////////////////////////////////////////////////////////////////////////////////*/
	for (j=0; j<sh_mem->terminals; j++) {
		child_id=fork();
		if(child_id==0){
			//Child gets shared memory handler
			sh_mem_id=shmget((key_t)SH_MEM_NAME,sizeof(data),0600); 
			if(sh_mem_id<0)
			{
				printf("Child failed to retrieve shared memory handler.\n");
				exit(1);
			}
			
			//Child is attached to shared memory.
			sh_mem=(data*)shmat(sh_mem_id,NULL,0);
			if(sh_mem==(data*)-1)
			{
				printf("Failed to attach child to shared memory.\n");
				exit(1);
			}
			

			
			/*read queue, if there is work do it */
			while(1) {
				sem_wait(sh_mem->term_sem); //elegxos an yparxei diathesimo terminal			
				if(sh_mem->term_head < sh_mem->term_tail) {

					int head = sh_mem->term_head;//!!!!!!!!!!!!!!!!
					sh_mem->term_head++;

					sem_post(sh_mem->term_sem);

					
					printf("Checking cardID and account's balance...\n");
					//periptwsh apotyxias
					if (percent_value(head)==7){
						sh_mem->reservations[head].info[5] = 2;
					}

					//////////////////////////////
					sleep(CARDCHECK_TIME);
					///////////////////////////
			
					
					sh_mem->reservations[head].info[4]=2; // CHECKED
				}
				else {
					sem_post(sh_mem->term_sem);
				}
			}
			exit(0);
		}
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////	

//--------------------------------SOCKETS-----------------------------------------------//
	/*Getting a socket handler*/
	sd=socket(AF_UNIX,SOCK_STREAM,0);
	if(sd<0)
	{
		printf("Error in creating socket.\n");
		exit(1);
	}

	/*Delete socket file if already exists*/
	unlink(SOCKET_NAME);  // "MySock"

	/*Specify socket address details*/
	saddr.sa_family = AF_UNIX;
	strcpy(saddr.sa_data,SOCKET_NAME);
	/*Bind socket*/
	r=bind(sd,&saddr,sizeof(saddr));
	if(r<0)
	{
		printf("Error in binding socket.\n");
		exit(1);
	}	
	/*Listen to socket*/
	r=listen(sd,1); /*size of listening queue is 1*/
	if(r<0)
	{
		printf("Error in listening.\n");
		exit(1);
	}
	
	
	while(1) //
	{
		inlen=sizeof(inaddr);

		/*Wait for connection attempt*/
		conn=accept(sd,&inaddr,&inlen);
		if(conn<0)
		{
			printf("Connection attempt failed.\n");
			continue;
		}
		
		/*Connection established, spawn a child to serve it*/
		child_id=fork();
		if(child_id==0)
		{
			signal(SIGUSR1,sig_res);
			
			/*Child gets shared memory handler*/
			sh_mem_id=shmget((key_t)SH_MEM_NAME,sizeof(data),0600); 
			if(sh_mem_id<0)
			{
				printf("Child failed to retrieve shared memory handler.\n");
				exit(1);
			}
			
			/*Child is attached to shared memory.*/
			sh_mem=(data*)shmat(sh_mem_id,NULL,0);
			if(sh_mem==(data*)-1)
			{
				printf("Failed to attach child to shared memory.\n");
				exit(1);
			}
			
			/*read user reservation*/
			read(conn,&newRes,sizeof(reservation));
			
			
			
			printf("-------------------------------------------------------------\n");
			/*TODO:add reservation to queue*/
			/*TODO:add pid to queue*/
			sem_wait(sh_mem->res_sem);  		// trying to get res_sem opened
			int resNum = sh_mem->res_tail;		
			sh_mem->reservations[sh_mem->res_tail].info[0] = newRes.info[0]; //CLASS
			sh_mem->reservations[sh_mem->res_tail].info[1] = newRes.info[1]; //NUMBER
			sh_mem->reservations[sh_mem->res_tail].info[2] = newRes.info[2]; //CARD_ID
			sh_mem->reservations[sh_mem->res_tail].info[4] = 1; //STATUS -diavasthke
			
			if(sh_mem->Theater[0]==1){
				//If Theater is full set Curretnt reservation's status:
				//(info[4]=3)-Reservation process ended
				sh_mem->reservations[resNum].info[4]=3;
			}
			sh_mem->reservations[sh_mem->res_tail].procNumber = getpid();
			
			gettimeofday(&(sh_mem->reservations[sh_mem->res_tail].reserveTime), NULL);
			sh_mem->res_tail++;
			sem_post(sh_mem->res_sem);		 // leaving the oper_sem
			
			if(sh_mem->Theater[0]==0){
			
				while(1){  //Checking current reservation's status
				if(sh_mem->reservations[resNum].info[4]==3){
					break;	
					}
				}
			}
			else{
				
				sh_mem->reservations[resNum].info[5]=4;
				printf("Current reservation incompleted!\nTheater if FULL.\n No further reservations can be accepted. \n");
			}
			
			
			 switch (sh_mem->reservations[resNum].info[5]) {
                case 1:
                        sprintf(buffer, "Reservation %d completed !\nClass: %d\nSeats Number: %d\ncard_ID: %d \nCost: %d E",resNum, sh_mem->reservations[resNum].info[0], sh_mem->reservations[resNum].info[1], sh_mem->reservations[resNum].info[2],sh_mem->reservations[resNum].info[3]);	
						sem_wait(sh_mem->res_sem);
						sh_mem->Company_account=sh_mem->Company_account+sh_mem->reservations[resNum].info[3];
						sh_mem->com++;
						sem_post(sh_mem->res_sem);	
						break;
				case 2:
                        sprintf(buffer, "Reservation %d incompleted !\nInaccurate cardID or insufficient account's balance.",resNum);				
						sem_wait(sh_mem->res_sem);
						sh_mem->incom++;
						sem_post(sh_mem->res_sem);	
						break;
				case 3:
                        sprintf(buffer,"Reservation %d incompleted !\nNo seats available in this Class.",resNum);				
						sem_wait(sh_mem->res_sem);
						sh_mem->incom++;
						sem_post(sh_mem->res_sem);	
						break;
				case 4:
                        sprintf(buffer, "Reservation %d incompleted !\nTheater is FULL.",resNum);				
						sem_wait(sh_mem->res_sem);
						sh_mem->incom++;
						sem_post(sh_mem->res_sem);	
						break;
				default:
						sprintf(buffer, "Kati paixthke me th diergasia %d",resNum);	
			}

			write(conn,buffer,BSIZE);
			
			semaphores_close();

			/*deattach child from shared memory*/
			r=shmdt(sh_mem); 
			if(r<0)
			{	
				printf("Failed to deattach child from shared memory.\n");
				exit(1);
			}
			exit(0);
		}
		close(conn);
	}
	exit(0); //!!!!!!!!!!!
}
//=================================== End of Main========================================//

// Signal if reservations is completed  
void sig_res(int signo)
{
	printf("Reservation complete.\n");
}


/*Function for handling children termination.
Added to avoid zombie processes  */
void sig_chld(int signo)
{
	pid_t pid;
	int stat;
	while((pid=waitpid(-1,&stat,WNOHANG))>0){
		//printf("Child %d terminated.\n",pid);
	}
}

/*Function for handling server termination.
Added to release resources. */
void sig_int(int signo)
{
	int r;

	semaphores_close();
	//printf("Semaphores closed.\n");
	semaphores_unlink();
	//printf("Semaphores unlinked.\n");
	/*Deattach server from shared memory*/
	r=shmdt(sh_mem);
	if(r==-1) {
		printf("Failed to deattach server from shared memory.\n");
		exit(1);
	}
	//printf("Shared memory deattached.\n");

	/*Destroy shared memory*/
	shmctl(sh_mem_id,IPC_RMID,NULL);
	//printf("Shared memory destroyed.\n");

	//printf("Server terminated .\n");
	exit(0);
}


/*Function for initializing shared memory data  */
void init_data(int operators, int terminals, int As, int Bs, int Cs, int Ds)
{
	int i;

	for(i=0;i<NUM_RES;i++) 
	{
		sh_mem->reservations[i].info[0]=0; // Class 1-4
		sh_mem->reservations[i].info[1]=0;	// Number 1-4
		sh_mem->reservations[i].info[2]=0;	// Card_id
		sh_mem->reservations[i].info[3]=0; //Money
		sh_mem->reservations[i].info[4]=0; //Current status
		sh_mem->reservations[i].info[5]=0; //Reservation's conclusion - 0:den exei ginei tpt akoma
	}
	
	// Initializing our semaphores.
	sh_mem->res_sem = sem_open(RES_SEM_NAME, O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO, 1);
	if (sh_mem->res_sem == SEM_FAILED){
		printf("Couldn't open semaphore-ressem.\n");
		shmdt(sh_mem);
		shmctl(sh_mem_id,IPC_RMID,NULL);
		exit(1);
	}
	sh_mem->oper_sem = sem_open(OPER_SEM_NAME, O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO, 1);
	if (sh_mem->oper_sem == SEM_FAILED){
		printf("Couldn't open semaphore-opersem.\n");
		shmdt(sh_mem);
		shmctl(sh_mem_id,IPC_RMID,NULL);
		exit(1);
	}
	sh_mem->term_sem = sem_open(TERM_SEM_NAME, O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO, 1);
	if (sh_mem->term_sem == SEM_FAILED){
		printf("Couldn't open semaphore-termsem.\n");
		shmdt(sh_mem);
		shmctl(sh_mem_id,IPC_RMID,NULL);
		exit(1);
	}
	
	sh_mem->seat_sem = sem_open(SEAT_SEM_NAME, O_CREAT|O_RDWR, S_IRWXU|S_IRWXG|S_IRWXO, 1);
	if (sh_mem->seat_sem == SEM_FAILED){
		printf("Couldn't open semaphore-seatsem.\n");
		shmdt(sh_mem);
		shmctl(sh_mem_id,IPC_RMID,NULL);
		exit(1);
	}

	// Initializing everything else
	sh_mem->Theater[0]= 0; //Theater[0] -> THEATER STATUS. 
	sh_mem->Theater[1]= As;
	sh_mem->Theater[2]= Bs;
	sh_mem->Theater[3]= Cs;
	sh_mem->Theater[4]= Ds;
	sh_mem->com=0;	
	sh_mem->incom=0;	
	sh_mem->Theater_account=0;
	sh_mem->Company_account=0;	
	sh_mem->res_head = 0;
	sh_mem->res_tail = 0;
	sh_mem->term_head = 0;
	sh_mem->term_tail = 0;
	sh_mem->operators = operators;
	sh_mem->terminals = terminals;
}


/*Function for closing all semaphores. Called by children and parent. */
void semaphores_close()
{
	/*Global semaphores*/
	sem_close(sh_mem->seat_sem);	//Closing seamaphore "seat_sem"
	sem_close(sh_mem->res_sem);		//Closing semaphore of reservations
	sem_close(sh_mem->oper_sem);	//Closing semaphore of operators
	sem_close(sh_mem->term_sem);	//Closing semaphore of terminals
    //exit(0);	//TERMINALS
}

/*Function for unlinking semaphores. Called by parent only.*/
void semaphores_unlink()
{
	/*Global semaphores*/
	sem_unlink(SEAT_SEM_NAME);	//Unlinking semaphore "seat_sem"
	sem_unlink(RES_SEM_NAME);	//Unlinking semaphore of reservations
	sem_unlink(OPER_SEM_NAME);	//Unlinking semaphore of operators
	sem_unlink(TERM_SEM_NAME);	//Unlinking semaphore of terminals
}

/* A function that returns a random number between 1 and 10.
It uses the function rand().
The seed it uses in rand changes every time it runs.
(srand() uses time to achieve it) .
*/
int percent_value(int cc)
{
	srand ( time(NULL)*cc );
	int x = rand() %10 + 1;
	cc++;
	return(x);
}
