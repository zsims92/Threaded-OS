.include "scheduler_asm.h"
.section .exceptions.exit.user, "xa"
#This code will:
#	1. Check if an interrupt occured
#	2. If not, then continue as normal
#	3. Else it will load the context of the interrupting
#	   thread into r4-r9, and the ra into r17
#	4. It will then call mythread_scheduler
#	5. After returning, it will store the values or r4-r9
#	   back on the sp.  If a new thread was started then
#	   the values with be different.  Else they will stay
#	   the same.

_MyThread:
	movia	r10, global_flag
	ldw		r11, (r10)
	beq		r11, r0, _Skip
	stw		r0, (r10)
	ldw r4,12(sp)
	ldw r5,16(sp)
	ldw r6,20(sp)
	ldw r7,24(sp)
	ldw r8,28(sp)
	ldw	r9,72(sp)
	ldw	r17,0(sp)
	call 	mythread_scheduler

	stw 	r4, 12(sp)
	stw 	r5, 16(sp)
	stw 	r6, 20(sp)
	stw 	r7, 24(sp)
	stw		r8, 28(sp)
	stw 	r9, 72(sp)
	stw		r10, 0(sp)

_Skip:
