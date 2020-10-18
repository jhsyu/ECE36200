#include "stm32f0xx.h"
#include <math.h>

// Be sure to change this to your login...
const char login[] = "xu1392";

void internal_clock(void);
void display_float(float);
void control(void);

const int GPIOAEN = 0x20000;
const int GPIOBEN = 0x40000;
const int GPIOCEN = 0x80000;

void setup_portc(){
  RCC -> AHBENR |= GPIOCEN;
  GPIOC -> MODER &= 0x3fffff;
  GPIOC -> MODER |= 0x155555;
  GPIOC -> ODR = 0x03f;
}
void copy_pa0_pc6(){
  RCC -> AHBENR |= GPIOAEN;
  GPIOA -> MODER &= ~0x3;
  GPIOA -> PUPDR &= ~0x3;
  GPIOA -> PUPDR |= 0x2;
  RCC -> AHBENR |= GPIOCEN;
  GPIOC -> MODER &= ~(0x3 << 12);
  GPIOC -> MODER |= (0x1 << 12);
  for (; ;) {
    GPIOC -> ODR = (GPIOC -> ODR & ~(0x1 << 6)) | ((GPIOA -> IDR & 0x1) << 6);
  }
}


//============================================================================
// setup_adc()    (Autotest #1)
// Configure the ADC peripheral and analog input pins.
// Parameters: none
//============================================================================
void setup_adc(void)
{

}

//============================================================================
// start_adc_channel()    (Autotest #2)
// Select an ADC channel, and initiate an A-to-D conversion.
// Parameters: n: channel number
//============================================================================
void start_adc_channel(int n)
{

}

//============================================================================
// read_adc()    (Autotest #3)
// Wait for A-to-D conversion to complete, and return the result.
// Parameters: none
// Return value: converted result
//============================================================================
int read_adc(void)
{
  return /* replace with something meaningful */ 0;
}

//============================================================================
// setup_dac()    (Autotest #4)
// Configure the DAC peripheral and analog output pin.
// Parameters: none
//============================================================================
void setup_dac(void)
{

}

//============================================================================
// write_dac()    (Autotest #5)
// Write a sample to the right-aligned 12-bit DHR, and trigger conversion.
// Parameters: sample: value to write to the DHR
//============================================================================
void write_dac(int sample)
{

}


//============================================================================
// Parameters for the wavetable size and expected DAC rate.
//============================================================================
#define N 1000
#define RATE 20000
short int wavetable[N];

//============================================================================
// init_wavetable()    (Autotest #6)
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
int stepd = 0;
int offseta = 0;
int offsetb = 0;
int offsetc = 0;
int offsetd = 0;

//============================================================================
// set_freq_n()    (Autotest #7)
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

void set_freq_d(float f)
{

}

//============================================================================
// Timer 6 ISR    (Autotest #8)
// The ISR for Timer 6 which computes the DAC samples.
// Parameters: none
// (Write the entire subroutine below.)
//============================================================================


//============================================================================
// setup_tim6()    (Autotest #9)
// Configure Timer 6 to raise an interrupt RATE times per second.
// Parameters: none
//============================================================================
void setup_tim6(void)
{

}

int main(void)
{
  //internal_clock(); // Use the internal oscillator if you need it
  //autotest(); // test all of the subroutines you wrote
  init_wavetable();
  setup_dac();
  setup_adc();
  setup_tim6();
  set_freq_a(261.626); // Middle 'C'
  set_freq_b(329.628); // The 'E' above middle 'C'
  //control();
  while(1) {
    for(int out=0; out<4096; out++) {
      if ((TIM6->CR1 & TIM_CR1_CEN) == 0)
      write_dac(out);
      start_adc_channel(0);
      int sample = read_adc();
      float level = 2.95 * sample / 4095;
      display_float(level);
    }
  }
}
