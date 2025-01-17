.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

//===================================================================
// ECE 362 Lab Experiment 5
// Timers
//===================================================================

// RCC configuration registers
.equ  RCC,      0x40021000
.equ  AHBENR,   0x14
.equ  GPIOCEN,  0x00080000
.equ  GPIOBEN,  0x00040000
.equ  GPIOAEN,  0x00020000
.equ  APB1ENR,  0x1c
.equ  TIM6EN,   1<<4
.equ  TIM7EN,   1<<5

// NVIC configuration registers
.equ NVIC, 0xe000e000
.equ ISER, 0x100
.equ ICER, 0x180
.equ ISPR, 0x200
.equ ICPR, 0x280
.equ IPR,  0x400
.equ TIM6_DAC_IRQn, 17
.equ TIM7_IRQn, 18

// Timer configuration registers
.equ TIM6, 0x40001000
.equ TIM7, 0x40001400
.equ TIM_CR1,  0x0
.equ TIM_CR2,  0x4
.equ TIM_DIER, 0xc
.equ TIM_SR,   0x10
.equ TIM_EGR,  0x14
.equ TIM_CNT,  0x24
.equ TIM_PSC,  0x28
.equ TIM_ARR,  0x2c

// Timer configuration register bits
.equ TIM_CR1_CEN,  1<<0
.equ TIM_DIER_UDE, 1<<8
.equ TIM_DIER_UIE, 1<<0
.equ TIM_SR_UIF,   1<<0

// GPIO configuration registers
.equ  GPIOC,    0x48000800
.equ  GPIOB,    0x48000400
.equ  GPIOA,    0x48000000
.equ  MODER,    0x00
.equ  PUPDR,    0x0c
.equ  IDR,      0x10
.equ  ODR,      0x14
.equ  BSRR,     0x18
.equ  BRR,      0x28

//===========================================================================
// enable_ports  (Autotest 1)
// Enable the RCC clock for GPIO ports B and C.
// Parameters: none
// Return value: none
.global enable_ports
enable_ports:
	push {lr}
	// Student code goes below
	// enable port B
	ldr 	r0, =RCC
	ldr 	r1, [r0, #AHBENR]
	ldr 	r2, =GPIOBEN
	orrs  r1, r2

	// enable port C
	ldr 	r2, =GPIOCEN
	orrs  r1, r2
	// write memory.
	str   r1, [r0, #AHBENR]

	ldr 	r0, =GPIOB
	ldr 	r1, [r0, #MODER]
	// configure pb0 - pb3 as output.
	ldr   r2, =0xff
	bics  r1, r2
	ldr   r2, =0x55
	orrs  r1, r2
	// configure pb4 - pb7 as input
	ldr 	r2, =0xff00
	bics  r1, r2
	str 	r1, [r0, #MODER]
	// Configures PB4 – PB7 to be internally pulled low
	ldr 	r1, [r0, #PUPDR]
	bics  r1, r2
	ldr   r2, =0xaa00
	orrs  r1, r2
	str 	r1, [r0, #PUPDR]

	// configure PC0 - PC10 as output.
	ldr 	r0, =GPIOC
	ldr 	r1, [r0, #MODER]
	ldr 	r2, =0x3fffff
	bics  r1, r2
	ldr 	r2, =0x155555
	orrs  r1, r2
	str 	r1, [r0,#MODER]


	// Student code goes above
	pop  {pc}

//===========================================================================
// Timer 6 Interrupt Service Routine  (Autotest 2)
// Parameters: none
// Return value: none
// Write the entire subroutine below
.global	TIM6_DAC_IRQHandler
.type TIM6_DAC_IRQHandler, %function
TIM6_DAC_IRQHandler:
	push  {lr}
	// acknowledge the interrupt.
	ldr 	r0, =TIM6
	ldr 	r1, [r0, #TIM_SR]
	ldr   r2, =TIM_SR_UIF
	bics  r1, r2
	str 	r1, [r0, #TIM_SR]
	// toggle PC6.
	ldr 	r0, =GPIOC
	ldr 	r1, [r0, #ODR]
	movs  r2, #1
	lsls  r2, r2, #6
	eors  r1, r2
	str 	r1, [r0, #ODR]
	pop   {pc}


//===========================================================================
// setup_tim6  (Autotest 3)
// Configure timer 6
// Parameters: none
// Return value: none
.global setup_tim6
setup_tim6:
	push {lr}

	// enable RCC clock.
	ldr 	r0, =RCC
	ldr 	r1, [r0, #APB1ENR]
	ldr 	r2, =1
	// TIM6_EN: RCC_APB1ENR bit4
	lsls  r2, r2, #4
	orrs  r1, r2
	str 	r1, [r0, #APB1ENR]

	// configure TIM6_PSC
	ldr 	r0, =TIM6
	ldr 	r1, =48000-1
	str 	r1, [r0, #TIM_PSC]

	// configure TIM6_ARR
	ldr 	r1, =500-1
	str 	r1, [r0, #TIM_ARR]

	// configure TIM6_DIER
	ldr 	r1, [r0, #TIM_DIER]
	ldr 	r2, =TIM_DIER_UIE
	orrs  r1, r2
	str 	r1, [r0, #TIM_DIER]

	// configure TIM_CR1_CEN.
	ldr 	r1, [r0, #TIM_CR1]
	ldr 	r2, =TIM_CR1_CEN
	orrs  r1, r2
	str 	r1, [r0, TIM_CR1]

	// unmask the interrupt
	ldr 	r0, =NVIC
	ldr 	r1, =ISER
	ldr 	r2, =(1<<TIM6_DAC_IRQn)
	str 	r2, [r0, r1]


	pop  {pc}

.data
.global display
display: .byte 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07
.global history
history: .space 16
.global font
font: .byte 0x06, 0x5b, 0x4f, 0x77, 0x66, 0x6d, 0x7d, 0x7c, 0x07, 0x7f, 0x67, 0x39, 0x49, 0x3f, 0x76, 0x5e
.global offset
offset: .byte 0
.text

//===========================================================================
// show_digit  (Autotest 4)
// Set up the Port C outputs to show the digit for the current
// value of the offset variable.
// Parameters: none
// Return value: none
// Write the entire subroutine below.
.global show_digit
show_digit:
	push  {r4-r7, lr}
	// calculate off
	ldr 	r0, =offset
	ldrb 	r1, [r0]				// r1: value of offset
	movs  r2, #7
	ands  r1, r2					// r1: off
	// get GPIOC -> ODR
	ldr 	r0, =GPIOC
	ldr 	r2, [r0, #ODR]

	// calculate (off << 8) | display[off]
	movs  r3, #8
	movs  r4, r1					// r4: copy of off
	lsls  r4, r3					// r4: off << 8

	ldr 	r3, =display
	ldrb 	r5, [r3, r1]		// r5: display[off]
	orrs	r4, r5
	str 	r4, [r0, #ODR]

	pop   {r4-r7, pc}


//===========================================================================
// get_cols  (Autotest 5)
// Return the current value of the PC8 - PC4.
// Parameters: none
// Return value: 4-bit result of columns active for the selected row
// Write the entire subroutine below.
.global get_cols
get_cols:
	push  {lr}
	ldr 	r0, =GPIOB
	ldr 	r1, [r0, #IDR]
	lsrs  r1, r1, #4
	ldr 	r2, =0xf
	ands  r1, r2
	movs  r0, r1
	pop 	{pc}


//===========================================================================
// update_hist  (Autotest 6)
// Update the history byte entries for the current row.
// Parameters: r0: cols: 4-bit value read from matrix columns
// Return value: none
// Write the entire subroutine below.
.global update_hist
update_hist:
	push	{r4-r7, lr}
	// r0: cols
	ldr 	r1, =offset
	ldrb 	r4, [r1]
	movs  r1, #3
	ands  r4, r1				// r4: row = offset & 3

	movs  r5, #0				// r5: index i
	ldr 	r1, =history  // r1: &history[0]
while0:
	cmp 	r5, #4
	bge   endwhile0
	movs  r2, r4
	lsls  r2, r2, #2
	adds  r2, r5				// r2: 4*row+i
	ldrb 	r3, [r1, r2]
	lsls  r3, r3, #1		// r3: history[4*row+i]<<1
	movs  r6, r0
	lsrs  r6, r5
	movs  r7, #1
	ands  r6, r7				// r6: (cols>>i)&1
	adds  r3, r6
	strb 	r3, [r1, r2]
	adds  r5, #1
	b			while0
endwhile0:


	pop 	{r4-r7, pc}


//===========================================================================
// set_row  (Autotest 7)
// Set PB3 - PB0 to represent the row being scanned.
// Parameters: none
// Return value: none
// Write the entire subroutine below.
.global set_row
set_row:
	push	{lr}
	ldr 	r0, =offset
	ldr 	r1, [r0]
	movs  r2, #3
	ands  r1, r2				// r1: row = offset & 3

	ldr 	r0, =GPIOB
	//ldr 	r1, [r0,#BSRR]
	ldr 	r2, =0xf0000
	movs  r3, #1
	lsls  r3, r1
	orrs  r2, r3
	str 	r2, [r0, #BSRR]

	pop 	{pc}


//===========================================================================
// Timer 7 Interrupt Service Routine  (Autotest 8)
// Parameters: none
// Return value: none
// Write the entire subroutine below
.global	TIM7_IRQHandler
.type TIM7_IRQHandler, %function
TIM7_IRQHandler:
	push	{lr}
	// acknowledge the interrupt.
	ldr 	r0, =TIM7
	ldr 	r1, [r0, #TIM_SR]
	ldr   r2, =TIM_SR_UIF
	bics  r1, r2
	str 	r1, [r0, #TIM_SR]

	bl		show_digit
	bl  	get_cols				// r0: cols
	bl  	update_hist

	ldr 	r0, =offset
	ldr 	r1, [r0]				// r1: offset
	adds  r1, #1
	movs  r2, #0x7
	ands  r1, r2
	str 	r1, [r0]
	bl		set_row
	pop 	{pc}


//===========================================================================
// setup_tim7  (Autotest 9)
// Configure Timer 7.
// Parameters: none
// Return value: none
.global setup_tim7
setup_tim7:
	push {lr}
	// enable RCC clock.
	ldr 	r0, =RCC
	ldr 	r1, [r0, #APB1ENR]
	// disable TIM6
	ldr 	r2, =1
	lsls  r2, r2, #4
	bics  r1, r2
	// enable TIM7
	lsls  r2, r2, #1
	orrs  r1, r2
	// write register
	str 	r1, [r0, #APB1ENR]

	// configure TIM7_PSC
	ldr 	r0, =TIM7
	ldr 	r1, =4800-1
	str 	r1, [r0, #TIM_PSC]

	// configure TIM6_ARR
	ldr 	r1, =10-1
	str 	r1, [r0, #TIM_ARR]

	// configure TIM6_DIER
	ldr 	r1, [r0, #TIM_DIER]
	ldr 	r2, =TIM_DIER_UIE
	orrs  r1, r2
	str 	r1, [r0, #TIM_DIER]

	// configure TIM_CR1_CEN.
	ldr 	r1, [r0, #TIM_CR1]
	ldr 	r2, =TIM_CR1_CEN
	orrs  r1, r2
	str 	r1, [r0, TIM_CR1]

	// unmask the interrupt
	ldr 	r0, =NVIC
	ldr 	r1, =ISER
	ldr 	r2, =(1<<TIM7_IRQn)
	str 	r2, [r0, r1]


	pop  {pc}


//===========================================================================
// get_keypress  (Autotest 10)
// Wait for and return the number (0-15) of the ID of a button pressed.
// Parameters: none
// Return value: button ID
.global get_keypress
get_keypress:
	push {r4-r7, lr}
	// Student code goes below
	ldr 	r0, =offset
	ldrb 	r1, [r0]				// r1: offset

endlessloop0:
	wfi
if0:
	movs  r2, #3
	ands  r2, r1					// r2: offset & 3
	cmp 	r2, #0
	bne   endlessloop0

	ldr 	r3, =history		// r3: &history[0]
	movs  r4, #0					// r4: index i
while1:
  cmp 	r4, #16
	bge   endwhile1
	ldrb 	r5, [r3, r4]
	cmp 	r5, #1
	beq   return
	adds  r4, #1
	b			while1
endwhile1:
	b 		endlessloop0
return:
	movs  r0, r4
	pop  {r4-r7, pc}
	// Student code goes above


//===========================================================================
// handle_key  (Autotest 11)
// Shift the symbols in the display to the left and add a new digit
// in the rightmost digit.
// ALSO: Create your "font" array just above.
// Parameters: ID of new button to display
// Return value: none
.global handle_key
handle_key:
	push {r4-r7, lr}
	// Student code goes below
	movs	r1, #0xf
	ands  r0, r1					// r0: int key
	movs  r1, #0					// r1: index i
	ldr 	r2, =display		// r2: &display
	ldr 	r3, =font				// r3: &font
while2:
	cmp 	r1, #7
	bge   endwhile2
	movs  r4, r1
	adds  r1, #1
	ldrb	r5, [r2, r1]		// load display[i+1]
	strb  r5, [r2, r4]		// store tp display[i]
	b 		while2
endwhile2:
	movs  r1, #7
	//subs  r0, #1
	ldrb 	r4, [r3, r0]
	strb  r4, [r2, r1]
	// Student code goes above
	pop  {r4-r7, pc}

.global login
login: .string "xu1392"
.align 2

//===========================================================================
// main
// Already set up for you.
// It never returns.
.global main
main:
	//bl  check_wiring
	bl  autotest
	bl  enable_ports
	bl  setup_tim6
	bl  setup_tim7
	//movs r0, #1
	//bl  handle_key
endless_loop:
	bl   get_keypress
	bl   handle_key
	b    endless_loop
