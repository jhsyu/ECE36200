
//============================================================================
// ECE 362 lab experiment 7 -- Pulse-Width Modulation
//============================================================================

#include "stm32f0xx.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for M_PI

// Be sure to change this to your login...
const char login[] = "xu1392";

#define TIM1_BDTR_MOE			(1 << 15)
#define TIM1_CCMR2_OC4PE	(1 << 11)
#define TIM1_CCER_CC1E		(1 << 0 )
#define TIM1_CCER_CC2E		(1 << 4 )
#define TIM1_CCER_CC3E		(1 << 8 )
#define TIM1_CCER_CC4E		(1 << 12)

//============================================================================
// setup_tim1()    (Autotest #1)
// Configure Timer 1 and the PWM output pins.
// Parameters: none
//============================================================================
void setup_tim1()
{
	RCC-> AHBENR |= RCC_AHBENR_GPIOAEN;
	// TIM1_CH1 PA8  	TIM1_CH2 PA9
	// TIM1_CH3 PA10	TIM1_CH4 PA11

	// Configure AF of PA 8-11
	GPIOA -> MODER &= ~(0xff << 16);
	GPIOA -> MODER |=  (0xaa << 16);

	// enable RCC clock for TIM1
	RCC -> APB2ENR |= RCC_APB2ENR_TIM1EN;
	// Configure GPIOA -> AFR TIM1 output is AF2.
	GPIOA -> AFR[1] &= ~(0xffff);
	GPIOA -> AFR[1] |=  (0x2222);
	// enable TIM1 in BDTR.
	TIM1 -> BDTR |= TIM1_BDTR_MOE;
	// set up clock.
	TIM1 -> PSC = 1 - 1;
	TIM1 -> ARR = 2400 - 1;
	//set PWM mode 1.
	TIM1 -> CCMR1 &= ~(0b111 << 4);
	TIM1 -> CCMR1 |=  (0b110 << 4);
	TIM1 -> CCMR1 &= ~(0b111 << 12);
	TIM1 -> CCMR1 |=  (0b110 << 12);
	TIM1 -> CCMR2 &= ~(0b111 << 4);
	TIM1 -> CCMR2 |=  (0b110 << 4);
	TIM1 -> CCMR2 &= ~(0b111 << 12);
	TIM1 -> CCMR2 |=  (0b110 << 12);
	// set preload_en for TIM1_CH4.
	TIM1 -> CCMR2 |= TIM1_CCMR2_OC4PE;
	// enable channel output.
	TIM1 -> CCER |= TIM1_CCER_CC1E;
	TIM1 -> CCER |= TIM1_CCER_CC2E;
	TIM1 -> CCER |= TIM1_CCER_CC3E;
	TIM1 -> CCER |= TIM1_CCER_CC4E;
	// enable the timer.
	//TIM1 -> DIER |= TIM_DIER_UIE;
	TIM1 -> CR1 |= TIM_CR1_CEN;
	//NVIC -> ISER[0] |= (1 << TIM1_CC_IRQn);

}

//============================================================================
// Parameters for the wavetable size and expected synthesis rate.
//============================================================================
#define N 1000
#define RATE 20000
short int wavetable[N];

//============================================================================
// init_wavetable()    (Autotest #2)
// Write the pattern for one complete cycle of a sine wave into the
// wavetable[] array.
// Parameters: none
//============================================================================
void init_wavetable(void){
  for(int i=0; i < N; i++)
    wavetable[i] = 32767 * sin(2 * M_PI * i / N);
}

//============================================================================
// Global variables used for four-channel synthesis.
//============================================================================
int volume = 2048;
int stepa = 0;
int stepb = 0;
int stepc = 0;
int stepd = 0; // not used
int offseta = 0;
int offsetb = 0;
int offsetc = 0;
int offsetd = 0; // not used

//============================================================================
// set_freq_n()    (Autotest #2)
// Set the four step and four offset variables based on the frequency.
// Parameters: f: The floating-point frequency desired.
//============================================================================
void set_freq_a(float f){
  stepa = f * N / RATE * (1<<16);
  if (f == 0) {
    stepa = 0;
    offseta = 0;
  }
}

void set_freq_b(float f){
  stepb = f * N / RATE * (1<<16);
  if (f == 0) {
    stepb = 0;
    offsetb = 0;
  }
}

void set_freq_c(float f){
  stepc = f * N / RATE * (1<<16);
  if (f == 0) {
    stepc = 0;
    offsetc = 0;
  }

}

void set_freq_d(float f){
  stepd = f * N / RATE * (1<<16);
  if (f == 0) {
    stepd = 0;
    offsetd = 0;
  }

}


//============================================================================
// Timer 6 ISR    (Autotest #2)
// The ISR for Timer 6 which computes the DAC samples.
// Parameters: none
// (Write the entire subroutine below.)
//============================================================================
void TIM6_DAC_IRQHandler() {
  // acknowledge interrupt.
  TIM6 -> SR &= !(TIM_SR_UIF);
  //DAC -> SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
  offseta += stepa;
  if ((offseta >> 16) >= N) offseta -= (N << 16);
  offsetb += stepb;
  if ((offsetc >> 16) >= N) offsetc -= (N << 16);
  offsetc += stepc;
  if ((offsetb >> 16) >= N) offsetb -= (N << 16);
  offsetd += stepd;
  if ((offsetd >> 16) >= N) offsetd -= (N << 16);
  int sample = wavetable[offseta >> 16]
             + wavetable[offsetb >> 16]
             + wavetable[offsetc >> 16]
             + wavetable[offsetd >> 16];
  //sample = (sample >> 5) + 2048;
	sample = ((sample * volume) >> 17) + 1200;
  if (sample > 4095) sample = 4095;
  else if (sample < 0) sample = 0;
  TIM1 -> CCR4 = sample;

}


//============================================================================
// setup_tim6()    (Autotest #2)
// Set the four step and four offset variables based on the frequency.
// Parameters: f: The floating-point frequency desired.
//============================================================================
void setup_tim6(void){
  // enable RCC clock for TIM6.
  RCC -> APB1ENR |= RCC_APB1ENR_TIM6EN;
  // set ARR and PSC value.
  TIM6 -> PSC = 60 - 1;
  TIM6 -> ARR = 40 - 1;
  // write UIE bit.
  TIM6 -> DIER |= TIM_DIER_UIE;
  TIM6 -> CR1 |= TIM_CR1_CEN;
  NVIC -> ISER[0] |= (1 << TIM6_DAC_IRQn);
}

//============================================================================
// enable_ports()    (Autotest #3)
// Configure GPIO Ports B and C.
// Parameters: none
//============================================================================
void enable_ports(){
	RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
	// set PB0 - pb3 as output.
	for (int i = 0; i < 4; i++) {
		GPIOB -> MODER &= ~(0b11 << (2*i));
		GPIOB -> MODER |= (0b01 << (2*i));
	}
	for (int i = 4; i < 8; i++) {
		// set PB4 - PB7 as input.
		GPIOB -> MODER &= ~(0b11 << (2*i));
		// set PB4 - PB7 as pull down.
		GPIOB -> PUPDR &= ~(0b11 << (2*i));
		GPIOB -> PUPDR |=  (0b10 << (2*i));
	}


	RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
	// set PC0 - PC10 as output.
	for (int i = 0; i < 11; i++) {
		GPIOC -> MODER &= ~(0b11 << (2*i));
		GPIOC -> MODER |=  (0b01 << (2*i));
	}

}

char offset;
char history[16];
char display[8];
char queue[2];
int  qin;
int  qout;

//============================================================================
// show_digit()    (Autotest #4)
// Output a single digit on the seven-segment LED array.
// Parameters: none
//============================================================================
void show_digit(){
	int off = offset & 7;
	GPIOC -> ODR = (off << 8) | display[off];
}

//============================================================================
// set_row()    (Autotest #5)
// Set the row active on the keypad matrix.
// Parameters: none
//============================================================================
void set_row(){
	int row = offset & 3;
	GPIOB -> BSRR = 0xf0000 | (1 << row);
}

//============================================================================
// get_cols()    (Autotest #6)
// Read the column pins of the keypad matrix.
// Parameters: none
// Return value: The 4-bit value read from PC[7:4].
//============================================================================
int get_cols(){
	return (GPIOB->IDR >> 4) & 0xf;
}

//============================================================================
// insert_queue()    (Autotest #7)
// Insert the key index number into the two-entry queue.
// Parameters: n: the key index number
//============================================================================
void insert_queue(int n){
	queue[qin] = n | 0x80;
	qin = (qin == 0) ? 1 : 0;
}

//============================================================================
// update_hist()    (Autotest #8)
// Check the columns for a row of the keypad and update history values.
// If a history entry is updated to 0x01, insert it into the queue.
// Parameters: none
//============================================================================
void update_hist(int cols){
	int row = offset & 3;
	for (int i = 0; i < 4; i++) {
		history[4*row + i] = (history[4*row+i]<<1) + ((cols>>i)&1);
		if (history[4*row + i] == 0x1) {
			insert_queue(4 * row + i);
		}
	}
}

//============================================================================
// Timer 7 ISR()    (Autotest #9)
// The Timer 7 ISR
// Parameters: none
// (Write the entire subroutine below.)
//============================================================================
void TIM7_IRQHandler(){
	TIM7 -> SR &= !(TIM_SR_UIF);
	show_digit();
  int cols = get_cols();
  update_hist(cols);
  offset = (offset + 1) & 0x7; // count 0 ... 7 and repeat
  set_row();
}

//============================================================================
// setup_tim7()    (Autotest #10)
// Configure timer 7.
// Parameters: none
//============================================================================
void setup_tim7(){
	// enable RCC clock for TIM6.
	RCC -> APB1ENR |=  RCC_APB1ENR_TIM7EN;
	RCC -> APB1ENR &= ~RCC_APB1ENR_TIM6EN;
	// set ARR and PSC value.
	TIM7 -> PSC = 800 - 1;
	TIM7 -> ARR = 60 - 1;
	// write UIE bit.
	TIM7 -> DIER |= TIM_DIER_UIE;
	TIM7 -> CR1 |= TIM_CR1_CEN;
	NVIC -> ISER[0] |= (1 << TIM7_IRQn);

}

//============================================================================
// getkey()    (Autotest #11)
// Wait for an entry in the queue.  Translate it to ASCII.  Return it.
// Parameters: none
// Return value: The ASCII value of the button pressed.
//============================================================================
int getkey(){
	for(;;) {
		asm volatile ("wfi" : :);   			// wait for an interrupt
		if (queue[qout] == 0) continue;   // go back to waiting
		else {
			char temp = queue[qout];
			queue[qout] = 0;
			qout = (qout == 1) ? 0 : 1;
			temp &= 0x7f;
			switch (temp) {
				case 0: return '1';
				case 1: return '2';
				case 2: return '3';
				case 3: return 'A';
				case 4: return '4';
				case 5: return '5';
				case 6: return '6';
				case 7: return 'B';
				case 8: return '7';
				case 9: return '8';
				case 10: return '9';
				case 11: return 'C';
				case 12: return '*';
				case 13: return '0';
				case 14: return '#';
				case 15: return 'D';
			}
		}
	}
	return 0;
}
//============================================================================
// This is a partial ASCII font for 7-segment displays.
// See how it is used below.
//============================================================================
const char font[] = {
	[' '] = 0x00,
	['0'] = 0x3f,
	['1'] = 0x06,
	['2'] = 0x5b,
	['3'] = 0x4f,
	['4'] = 0x66,
	['5'] = 0x6d,
	['6'] = 0x7d,
	['7'] = 0x07,
	['8'] = 0x7f,
	['9'] = 0x67,
	['A'] = 0x77,
	['B'] = 0x7c,
	['C'] = 0x39,
	['D'] = 0x5e,
	['*'] = 0x49,
	['#'] = 0x76,
	['.'] = 0x80,
	['?'] = 0x53,
	['b'] = 0x7c,
	['r'] = 0x50,
	['g'] = 0x6f,
	['i'] = 0x10,
	['n'] = 0x54,
	['u'] = 0x1c,
};

// Shift a new character into the display.
void shift(char c)
{
	memcpy(display, &display[1], 7);
	display[7] = font[c];
}

// Turn on the dot of the rightmost display element.
void dot()
{
	display[7] |= 0x80;
}

// Read an entire floating-point number.
float getfloat()
{
	int num = 0;
	int digits = 0;
	int decimal = 0;
	int enter = 0;
	memset(display,0,8);
	display[7] = font['0'];
	while(!enter) {
		int key = getkey();
		if (digits == 8) {
			if (key != '#')
			continue;
		}
		switch(key) {
			case '0':
			if (digits == 0)
			continue;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			num = num*10 + key-'0';
			decimal <<= 1;
			digits += 1;
			if (digits == 1)
			display[7] = font[key];
			else
			shift(key);
			break;
			case '*':
			if (decimal == 0) {
				decimal = 1;
				dot();
			}
			break;
			case '#':
			enter = 1;
			break;
			default: continue; // ABCD
		}
	}
	float f = num;
	while (decimal) {
		decimal >>= 1;
		if (decimal)
		f = f/10.0;
	}
	return f;
}

// Read a 6-digit BCD number for RGB components.
int getrgb()
{
	memset(display, 0, 8);
	display[0] = font['r'];
	display[1] = font['r'];
	display[2] = font['g'];
	display[3] = font['g'];
	display[4] = font['b'];
	display[5] = font['b'];
	int digits = 0;
	int rgb = 0;
	for(;;) {
		int key = getkey();
		if (key >= '0' || key <= '9') {
			display[digits] = font[key];
			digits += 1;
			rgb = (rgb << 4) + (key - '0');
		}
		if (digits == 6)
		break;
	}
	return rgb;
}

//============================================================================
// setrgb()    (Autotest #12)
// Accept a BCD-encoded value for the 3 color components.
// Update the CCR values appropriately.
// Parameters: rgb: the RGB color component values
//============================================================================
void setrgb(int rgb){
	int r 	= (rgb >> (4*4)) & 0xff;
	int g 	= (rgb >> (4*2)) & 0xff;
	int b 	= (rgb >> (4*0)) & 0xff;

	int arr = TIM1 -> ARR;
	r = ((r >> 4) * 10 + (r & 0xf)) * (arr + 1) / 100;
	g = ((g >> 4) * 10 + (g & 0xf)) * (arr + 1) / 100;
	b = ((b >> 4) * 10 + (b & 0xf)) * (arr + 1) / 100;

	TIM1 -> CCR1 = (arr + 1) - r;
	TIM1 -> CCR2 = (arr + 1) - g;
	TIM1 -> CCR3 = (arr + 1) - b;
}

void internal_clock();
void demo();
void autotest();

int main(void)
{
	//internal_clock();
	//demo();

    autotest();
	enable_ports();
	init_wavetable();
	set_freq_a(261.626); // Middle 'C'
	set_freq_b(329.628); // The 'E' above middle 'C'
	set_freq_c(391.996); // The 'G' above middle 'C'
	setup_tim1();
	setup_tim6();
	setup_tim7();

	display[0] = font['r'];
	display[1] = font['u'];
	display[2] = font['n'];
	display[3] = font['n'];
	display[4] = font['i'];
	display[5] = font['n'];
	display[6] = font['g'];
	for(;;) {
		char key = getkey();
		if (key == 'A')
		set_freq_a(getfloat());
		else if (key == 'B')
		set_freq_b(getfloat());
		else if (key == 'C')
		set_freq_c(getfloat());
		else if (key == 'D')
		setrgb(getrgb());
	}


	//setup_tim1();
	//int rgb = 0x112599;
	//setrgb(rgb);

}
