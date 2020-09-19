.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.data
.align 4
// Your global variables go here
.global sum
sum: .word 0
.global source
source: .word 3,4,7,11,18,29,47,76,123,199,322
.global str
str: .string "hello, 01234 world! 56789=-"


.text
.global intsub
intsub:		ldr r0,=sum
			ldr r1,[r0]		// r1 holds sum
			ldr r0,=source
			ldr r2, [r0]	// r2 holds currenct elem.
			movs r3,#0		// r3 holds index i

while1:		cmp r3,#10
			bge endwhile1

if1: 		cmp r1,#25
			bge else1
			adds r1,r2
			adds r0,#4		// shift the address to next elem.
			ldr r2,[r0]		// load the next elem.
			adds r1,r2		// add to sum
			adds r3,#1		// update index.
			b endif1

else1: 		lsls r2,r2,#1 	// multiply the current element by 2
			subs r1,r2		// subs from sum
			adds r0,#4		// shift the address to next elem.
			ldr r2,[r0]		// load the next elem.
			adds r3,#1		// update index.
endif1:
			b while1
endwhile1:
			ldr r0,=sum
			str r1,[r0]
    		bx lr



.global charsub
charsub:
			ldr r0,=str			// r0 holds the string address.
			ldrb r1,[r0]		// r1 holds current char.
			movs r2,#0			// r2 holds index i


while2:
			cmp r1,#0
			beq endwhile1
			movs r3,r1			// r3 serves as temp.
			adds r0,#1			// r0 points to next elem.
			ldrb r1,[r0]		// load the next char
			cmp r1,#0
			beq endwhile1
if2:
			cmp  r3,#0x61
			blt  endif2
			cmp  r3,#0x7a
			bgt  endif2

			subs r0,#1			// now r0 ponits to first char
			strb r1,[r0]		// store the second char to first.
			adds r0,#1			// now r0 points to second char
			strb r3,[r0]		// store the first char to second.
endif2:
			adds r2,#2			// update index.
			adds r0,#1			// updates r0
			ldrb r1,[r0]		// update r1.
			b while2


endwhile2:
			bx lr


.global login
login: .string "score:" // Make sure you put your login here.
.align 2
.global main
main:
    bl autotest // uncomment AFTER you debug your subroutines
    bl intsub
    bl charsub
    bkpt
