/*
 * Main.c
 *
 *  Created on: Oct 18, 2013
 *      Author: Zach Sims and Elliot Mitchell
 */
#include <stdio.h>
#include <malloc.h>
#include "sys/alt_alarm.h"
#include "alt_types.h"
#include "../main.h"

TCB threads[NUM_THREADS];
TCB* currThread = NULL;

void mythread(int thread_id){
	// The declaration of j as an integer was added on 10/24/2011
	int i, j, n;

	n = (thread_id % 2 == 0)? 10: 15;
	for (i = 0; i < n; i++){
		printf("This is message %d of thread #%d.\n", i, thread_id);
		for (j = 0; j < MAX; j++);
	}
}

void mythread_scheduler(param_list){
	DISABLE_INTERRUPTS();
	int i, threadID = -1;
	//This determines which thread to run or which thread is running
	//Uses a basic FIFO thread order
	for(i=0; i<NUM_THREADS; i++){
		if(threads[i].status == READY || threads[i].status == RUNNING){
			threadID = i;
			break;
		}
	}
	//If a thread is available to run or is running,
	//we set the current thread pointer to that thread
	if(threadID != -1)
		currThread = &threads[threadID];
	else
		currThread = NULL;

	//If there are threads to be ran
	if(runnable_threads > 0){
		//If this is the first time a thread will start
		if(currThread->status == READY){
			//Load address of destroy function
			int x = (int) destroy;

			//Set the currThread to running
			currThread->status = RUNNING;

			//Save original context on stack
			//This is the context of the main function
			asm("mov %0, r17": "=r"(currThread->stack[0]));
			asm("mov %0, r4": "=r"(currThread->stack[1]));
			asm("mov %0, r5": "=r"(currThread->stack[2]));
			asm("mov %0, r6": "=r"(currThread->stack[3]));
			asm("mov %0, r7": "=r"(currThread->stack[4]));
			asm("mov %0, r8": "=r"(currThread->stack[5]));
			asm("mov %0, r9": "=r"(currThread->stack[6]));

			//Load context of new thread
			asm("mov r9, %0": "=r"(i): "r"(currThread->context[0]));
			asm("mov r4, %0": "=r"(i): "r"(currThread->context[1]));
			asm("mov r5, %0": "=r"(i): "r"(currThread->context[2]));
			asm("mov r6, %0": "=r"(i): "r"(currThread->context[3]));
			asm("mov r7, %0": "=r"(i): "r"(currThread->context[4]));
			asm("mov r8, %0": "=r"(i): "r"(currThread->context[5]));
			asm("mov r10, %0": "=r"(i): "r"(x));
		}
		//If a thread is already running, we do not need to change
		//the context of the stack at all.
	}
	else{//There are no mor threads to run
		 printf("Interrupted by the DE2 timer!\n");
	}
	ENABLE_INTERRUPTS();
}

alt_u32 mythread_handler (void * param_list){
	global_flag = 1;
	return ALARMTICKS(35); //Using the defined function above
}

void newTCB(TCB* tcb, int i){
	//Determing the addres of mythread
	//So the context of new thread can
	//be changed to start at mythread
	int x = (int)mythread;
	//Create a context to hold the context
	//of the thread
	tcb->context = malloc(sizeof(int) * 6);
	//Create a stack to hold the context
	//of the prototype_os()
	tcb->stack = malloc(sizeof(int) * 7);
	tcb->context[0] = x;
	tcb->context[3] = i;
}

void mythread_join(int i){
	//Tell the scheduler that a thread is available
	runnable_threads++;
	//Set that thread to ready
	threads[i].status = READY;
}

void mythread_create(int i){
	//Creates a new thread with
	//the given ID
	TCB tcb;
	newTCB(&tcb, i);
	threads[i] = tcb;
	//Sets the thread off until
	//mythread_join is used
	//on this thread
	threads[i].status = DONE;
}
/*
 * This function will clean up the current TCB and
 * then return to the original function unless it
 * get interrupted.  It alerts the user that the program
 * is here first.  It then loads the context of the
 * prototype_os here to return to it until another interrupt
 * Finally it free the context and stack of the currentTCB
 * and turns the thread off *
 * */
void destroy(){
	printf("In destroy function\n");
	int dc;

	asm("mov r12, %0": "=r"(dc): "r"(currThread->stack[0]));
	asm("stw r12, 8(sp)");
	asm("mov r2, %0": "=r"(dc): "r"(currThread->stack[1]));
	asm("mov r3, %0": "=r"(dc): "r"(currThread->stack[2]));
	asm("mov r4, %0": "=r"(dc): "r"(currThread->stack[3]));
	asm("mov r5, %0": "=r"(dc): "r"(currThread->stack[4]));
	asm("mov r6, %0": "=r"(dc): "r"(currThread->stack[5]));
	asm("mov r7, %0": "=r"(dc): "r"(currThread->stack[6]));

	runnable_threads -= 1;
	currThread->status = DONE;
	free(currThread->context);
	free(currThread->stack);
}

void prototype_os(param_list)
{
	int i;
	for(i=0; i<NUM_THREADS; i++){
		mythread_create(i);
	}

	for(i=0; i<NUM_THREADS; i++){
		mythread_join(i);
	}

	//Set the timer to be 1 second
	alt_alarm_start(&alarm, 10, mythread_handler, NULL);

	while (1)
	{
		printf ("This is the prototype os for my exciting CSE351 course projects!\n");
		int j=0;
		//MAX is the amount of wait time before the next printf above
		for (j = 0 ; j < MAX; j++);
	}
}

int main(){
	prototype_os();
	return 0;
}

