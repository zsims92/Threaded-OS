/*
 * Main.h
 *
 *  Created on: Oct 18, 2013
 *      Author: Zach
 */

#ifndef MAIN_H_
#define MAIN_H_

#define NUM_THREADS 8
//Defined a macro to calculate the time in seconds
#define ALARMTICKS(x) ((alt_ticks_per_second()*(x))/10)
//MAX value to wait before normal OS operation
#define MAX 100000
// disable an interrupt

#define DISABLE_INTERRUPTS() { asm("wrctl status, zero"); }
#define ENABLE_INTERRUPTS() { asm("movi et, 1");asm("wrctl status, et");}

int runnable_threads;
static alt_alarm alarm;
int global_flag;

typedef enum{
	READY=1,
	RUNNING,
	DONE,
} THREAD_STATUS;

typedef struct{
	int id;
	THREAD_STATUS status;
	int* sp;
	int* fp;
	int* context;
} TCB;

void destroy();
void mythread(int thread_id);
void mythread_scheduler(int sp, int fp);
alt_u32 mythread_handler (void * param_list);
void newTCB(TCB* tcb, int i);
void mythread_join(int i);
void mythread_create(int i);
void destroy();
void prototype_os();

#endif /* MAIN_H_ */
