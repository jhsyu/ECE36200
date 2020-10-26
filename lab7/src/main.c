
//============================================================================
// ECE 362 lab experiment 7 -- Pulse-Width Modulation
//============================================================================

#include "stm32f0xx.h"
#include <string.h> // for memset() declaration
#include <math.h>   // for M_PI

// Be sure to change this to your login...
const char login[] = "xu1392";

//============================================================================
// setup_tim1()    (Autotest #1)
// Configure Timer 1 and the PWM output pins.
// Parameters: none
//============================================================================
void setup_tim1()
{
	RCC-> AHBENR |= GPIOAEN;
	// TIM1_CH1 PA8  	TIM1_CH2 PA9
	// TIM1_CH3 PA10	TIM1_CH4 PA11

	// Configure AF of PA 8-11
	GPIOA -> MODER &= ~(0xff << 16);
	GPIOA -> MODER |= (0xaa << 16);



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
void init_wavetable(void)
{
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
void set_freq_a(float f)
{
}

void set_freq_b(float f)
{
}

void set_freq_c(float f)
{
}


//============================================================================
// Timer 6 ISR    (Autotest #2)
// The ISR for Timer 6 which computes the DAC samples.
// Parameters: none
// (Write the entire subroutine below.)
//============================================================================


//============================================================================
// setup_tim6()    (Autotest #2)
// Set the four step and four offset variables based on the frequency.
// Parameters: f: The floating-point frequency desired.
//============================================================================
void setup_tim6()
{
}

//============================================================================
// enable_ports()    (Autotest #3)
// Configure GPIO Ports B and C.
// Parameters: none
//============================================================================
void enable_ports()
{
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
void show_digit()
{
}

//============================================================================
// set_row()    (Autotest #5)
// Set the row active on the keypad matrix.
// Parameters: none
//============================================================================
void set_row()
{
}

//============================================================================
// get_cols()    (Autotest #6)
// Read the column pins of the keypad matrix.
// Parameters: none
// Return value: The 4-bit value read from PC[7:4].
//============================================================================
int get_cols()
{
}

//============================================================================
// insert_queue()    (Autotest #7)
// Insert the key index number into the two-entry queue.
// Parameters: n: the key index number
//============================================================================
void insert_queue(int n)
{
}

//============================================================================
// update_hist()    (Autotest #8)
// Check the columns for a row of the keypad and update history values.
// If a history entry is updated to 0x01, insert it into the queue.
// Parameters: none
//============================================================================
void update_hist(int cols)
{
}

//============================================================================
// Timer 7 ISR()    (Autotest #9)
// The Timer 7 ISR
// Parameters: none
// (Write the entire subroutine below.)
//============================================================================


//============================================================================
// setup_tim7()    (Autotest #10)
// Configure timer 7.
// Parameters: none
//============================================================================
void setup_tim7()
{
}

//============================================================================
// getkey()    (Autotest #11)
// Wait for an entry in the queue.  Translate it to ASCII.  Return it.
// Parameters: none
// Return value: The ASCII value of the button pressed.
//============================================================================
int getkey()
{
	return 0; // replace this
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
void setrgb(int rgb)
{
}

void internal_clock();
void demo();
void autotest();

int main(void)
{
	//internal_clock();
	demo();
	//autotest();
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
}
