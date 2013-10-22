.include "scheduler_asm.h"
.section .exceptions.exit.user, "xa"
.extern int global_flag

_MyThread:
	movia	r10, global_flag
	ldw		r11, (r10)
	beq		r11, r0, _Skip
	ldw r2,12(sp)
	ldw r3,16(sp)
	ldw r4,20(sp)
	ldw r5,24(sp)
	ldw r6,28(sp)
	ldw	r7,72(sp)
	ldw	r17,0(sp)
	addi	r12, r0, 0
	stw		r12, (r10)
	call 	mythread_scheduler

	stw 	r4, 12(sp)
	stw 	r5, 16(sp)
	stw 	r6, 20(sp)
	stw 	r7, 24(sp)
	stw		r8, 28(sp)
	stw 	r9, 72(sp)
	stw		r10, 0(sp)

_Skip:
