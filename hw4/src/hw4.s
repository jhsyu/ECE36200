.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.global login
login: .string "xu1392"
hello_str: .string "Hello, %s!\n"
mult2_str: .string "%d * %d = %d\n"
mult3_str: .string "%d * %d * %d = %d\n"
list_str:  .string "%s %05d %s %d students in %s, %d\n"
.align 2
.global hello
hello:
	push {lr}
  bl   serial_init
  ldr  r0, =hello_str
  ldr  r1, =login
  bl   printf
	pop  {pc}

showmult2_str: .string "%d * %d = %d\n"
.align 2
.global showmult2
showmult2:
	push {lr}
  movs r2,r1
  movs r1,r0
  ldr  r0,=mult2_str
  movs r3,r1
  muls r3,r2
  bl   printf
	pop  {pc}

// Add the rest of the subroutines here
.global showmult3
showmult3:
	push {r4-r7,lr}
  movs r3,r2
  movs r2,r1
  movs r1,r0
  ldr  r0,=mult3_str
  movs r4,r1
  muls r4,r2
  muls r4,r3
  sub sp,#4
  str  r4,[sp,#0]
  bl   printf
  add  sp,#4
	pop  {r4-r7,pc}

.global listing
listing:
  push {r4-r7,lr}
  ldr  r5,[sp,#20]    // r5: char* season
  ldr  r6,[sp,#24]    // r6: int year
  sub  sp,#12
  str  r3,[sp,#0]
  str  r5,[sp,#4]
  str  r6,[sp,#8]
  // shift r0-r2 to r1-r3
  movs r3,r2
  movs r2,r1
  movs r1,r0
  ldr  r0,=list_str
  bl   printf
  add  sp,#12
  pop  {r4-r7,pc}

.global trivial
trivial:
  push {r4-r7,lr}
  sub  sp,#400
  mov r4,sp         // r4: int* arr
  movs r5,#0        // r5: index x
while50:
  cmp  r5,#100
  bge  if50
  lsls r6,r5,#2     // [r4,r6]: &arr[x]
  movs r7,#1
  adds r7,r5
  str  r7,[r4,r6]
  adds r5,#1
  b    while50
if50:
  cmp  r0,#99
  bls  endif50
  movs r0,#99
endif50:
  lsls r0,r0,#2
  ldr  r1,[r4,r0]
  add  sp,#400
  movs r0,r1
  pop  {r4-r7,pc}

.global reverse_puts
reverse_puts:
  push {r4-r7,lr}
  movs r4,#0        // r4: len
while60:
  ldrb r1,[r0,r4]
  cmp  r1,#0
  beq  endwhile60
  adds r4,#1
  b    while60
endwhile60:
  movs r2,#3
  movs r5,r4
  adds r5,#4
  bics r5,r2        // r5: newlen

  //sub  sp,r5
  mov  r2,sp
  subs r2,r5
  mov  sp,r2
  mov  r6,sp        // r6: char* buffer
  movs r2,#0
  strb r2,[r6,r4]   // buffer[len] = 0
  movs r7,#0        // r7: index x
while61:
  cmp  r7,r4
  bge  endwhile61
  ldrb r2,[r0,r7]   // r2: s[x]
  movs r3,r4
  subs r3,#1        // r3: len -1
  subs r3,r7        // r3: len - 1 - x
  strb r2,[r6,r3]   // [r6,r3]: buffer[r3]
  adds r7,#1
  b    while61
endwhile61:
  movs r7,r0
  movs r0,r6
  bl   puts
  movs r0,r7
  add  sp,r5
  pop  {r4-r7,pc}

.global sumsq
sumsq:
  push {r4-r7,lr}
  sub  sp,#400
if70:
  cmp  r0,#99
  bls  if71
  movs r0,#99
if71:
  cmp  r1,#99
  bls  endif71
  movs r1,#99
endif71:
  movs r4,#1        // r4: step
if72:
  cmp  r0,r1
  bne  elseif72
  movs r4,#0
elseif72:
  cmp  r0,r1
  bls  endif72
  subs r4,#2
endif72:
  movs r2,#0        // r2: index x
  mov  r5,sp        // r5: int* tmp
                    // &tmp[n] = [r5,r3]
while70:
  cmp  r2,#99
  bhi  endwhile70
  lsls r3,r2,#2     // r3: mem shift.
  movs r6,r2
  muls r6,r6        // r6: x^2
  str  r6,[r5,r3]
  adds r2,#1
  b    while70
endwhile70:
  movs r6,#0        // r6: sum
  movs r2,r0        // r2: index x
while71:
  lsls r3,r2,#2
  ldr  r7,[r5,r3]
  adds r6,r7
  cmp  r2,r1
  beq  endwhile71
  adds r2,r4
  b    while71
endwhile71:
  movs r0,r6
  add  sp,#400
  pop  {r4-r7,pc}
