//Headers
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <time.h>

#include <unistd.h>

//Defines
#define SOCKET_NAME "MySock"	//socket name
#define BSIZE 8192	//buffer size
#define SH_MEM_NAME 0x101010	//socket
#define NUM_RES 1000	//number of max reservations(completed or not)
#define SEAT_SEM_NAME "seatsem"		//semaphore name 
#define RES_SEM_NAME "ressem"	//semaphore name 
#define OPER_SEM_NAME "opersem"	//semaphore name 
#define TERM_SEM_NAME "termsem"	//semaphore name 
#define SEATFIND_TIME 6		//time for checking seats
#define CARDCHECK_TIME 2	//time for card
#define TRANSFER_TIME 30	//every <TRANSFER_TIME> we trasfer money to theater account
#define WAIT_TIME 10	//every <WAIT_TIME> we apologize for waiting
#define NUMBER_OF_OPERATORS 10	//Number of operators
#define NUMBER_OF_TERMINALS 4	//Number of terminals
#define A_SEATS 100	//Theater_Seats [1] Class A
#define B_SEATS	130	//Theater_Seats [2] Class B
#define C_SEATS	180	//Theater_Seats [3] Class C
#define D_SEATS	230	//Theater_Seats [4] Class D

/*Struct for each reservation*/
typedef struct reservation
{
	int procNumber;	// proccess number who made the reservation
	int info[6];	//An array to save reservation's informations			
	/*  Each seat in this array has its own meaning:
	[0]-Seats Class
	[1]-Seats Number
	[2]-Card_id of the consumer 
	[3]-Cost - (If the reservation is completed, we save here the ticket's total cost
	[4]-Current reservation status:
			*0-Initialized.
			*1-Sended to Operators. 
			*2-Checked for available seats and Consumer's bank account. 
			*3-Reservation process ended. 
	[5]-Reservation's conclusion: 
				*0-Nothing yet. 
				*1-Done.
				*2-Wrong card_ID/Insufficient account's balance. 
				*3-No available seats in this Class.
				*4-No available seats, because theater is full.
	*/
	struct timeval reserveTime; // time the reservation was made
}reservation; //struct's name

/*Struct for each data*/
typedef struct data
{	
	int Theater[5]; 
	/*
	Theater[0] -> THEATER STATUS !
			If Theater is Full, then Theater[0]= 1
			else Theater[0]= 0.
	Theater[1] -> Available seats in Class A
	Theater[2] -> Available seats in Class B
	Theater[3] -> Available seats in Class C
	Theater[4] -> Available seats in Class D
	*/
	reservation reservations[NUM_RES];
	pid_t pid[NUM_RES];
	int operators;	//Number of operators
	int terminals;	//Number of terminals
	//We use 2 variables to check the qeue of reservations
	int res_head;	//Qeue's head for reservations
	int res_tail;	//Qeue's tails for reservations
	//We use 2 variables to check the qeue of terminals
	int term_head;  //Qeue's head for terminals
	int term_tail;	//Qeue's tail for terminals
	int com;	//counter for completed reservations
	int incom;	//counter for incompleted reservations
	int Theater_account;	//Theater's bank account
	int Company_account;	//Company's bank account
	sem_t *res_sem;		//A semaphore for locking reservations.
	sem_t *seat_sem;	 //A semaphore to control which operator can change seat's data.
	sem_t *oper_sem;	 //A semaphore for locking operators.
	sem_t *term_sem;	 //A semaphore for locking terminals.
}data;//struct's name


 
 
