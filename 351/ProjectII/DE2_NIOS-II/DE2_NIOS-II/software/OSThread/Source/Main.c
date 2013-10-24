/*
 * Main.c
 *
 *  Created on: Oct 18, 2013
 *      Author: Zach
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
	for(i=0; i<NUM_THREADS; i++){
		if(threads[i].status == READY || threads[i].status == RUNNING){
			threadID = i;
			break;
		}
	}

	if(threadID != -1)
		currThread = &threads[threadID];
	else{
		currThread = NULL;
	}

	if(runnable_threads > 0){
		if(currThread->status == READY){
			//Load address of destroy function
			int x;
			asm("movia %0, destroy": "=r"(x));
			currThread->status = RUNNING;
			//Save original context on stack
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
		else if(currThread->status == RUNNING){
			//Save Context
			asm("mov %0, r9": "=r"(currThread->context[0]));
			asm("mov %0, r4": "=r"(currThread->context[1]));
			asm("mov %0, r5": "=r"(currThread->context[2]));
			asm("mov %0, r6": "=r"(currThread->context[3]));
			asm("mov %0, r7": "=r"(currThread->context[4]));
			asm("mov %0, r8": "=r"(currThread->context[5]));
			int dc;
			int x;
			asm("movia %0, destroy": "=r"(x));
			//Load Context
			asm("mov r9, %0": "=r"(dc): "r"(currThread->context[0]));
			asm("mov r4, %0": "=r"(dc): "r"(currThread->context[1]));
			asm("mov r5, %0": "=r"(dc): "r"(currThread->context[2]));
			asm("mov r6, %0": "=r"(dc): "r"(currThread->context[3]));
			asm("mov r7, %0": "=r"(dc): "r"(currThread->context[4]));
			asm("mov r8, %0": "=r"(dc): "r"(currThread->context[5]));
			//asm("mov r10, %0": "=r"(dc): "r"(x));
		}
	}
	else{
		 printf("Interrupted by the DE2 timer!\n");
	}
	ENABLE_INTERRUPTS();
}

alt_u32 mythread_handler (void * param_list){
	global_flag = 1;
	return ALARMTICKS(35); //Using the defined function above
}

void newTCB(TCB* tcb, int i){
	int x = 0;
	asm("movia %0, mythread": "=r"(x));
	tcb->context = malloc(sizeof(int) * 6);
	tcb->stack = malloc(sizeof(int) * 7);
	tcb->context[0] = x;
	tcb->context[1] = 0;
	tcb->context[2] = 0;
	tcb->context[3] = i;
	tcb->context[4] = 0;
	tcb->context[5] = 0;
}

void mythread_join(int i){
	runnable_threads++;
	threads[i].status = READY;
}

void mythread_create(int i){
	TCB tcb;
	newTCB(&tcb, i);
	threads[i] = tcb;
	threads[i].status = DONE;
}

void destroy(){
	printf("In destroy function\n");
	int dc;

	asm("mov r12, %0": "=r"(dc): "r"(currThread->stack[0]));
	asm("stw r12, 8(sp)");
	asm("mov r4, %0": "=r"(dc): "r"(currThread->stack[1]));
	asm("mov r5, %0": "=r"(dc): "r"(currThread->stack[2]));
	asm("mov r6, %0": "=r"(dc): "r"(currThread->stack[3]));
	asm("mov r7, %0": "=r"(dc): "r"(currThread->stack[4]));
	asm("mov r8, %0": "=r"(dc): "r"(currThread->stack[5]));
	asm("mov r9, %0": "=r"(dc): "r"(currThread->stack[6]));

	runnable_threads -= 1;
	currThread->status = DONE;
	free(currThread->context);
	free(currThread->stack);
	asm("mov r2, r4");
	asm("mov r3, r5");
	asm("mov r4, r6");
	asm("mov r5, r7");
	asm("mov r6, r8");
	asm("mov r7, r9");
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

