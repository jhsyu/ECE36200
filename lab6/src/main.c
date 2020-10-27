#include "stm32f0xx.h"
#include <math.h>
#include <stdio.h>

#define PA01_ANALOG_MODE            (0xf)
#define PA4_ANALOG_MODE             (0x3 << 8)
#define RCC_CR2_HSI14_ON            (0x1)
#define RCC_CR2_HSI14_RDY           (0x1 << 1)
#define SOFTWARE_TRIGER1            (0b111100)
#define TIM6_DAC_IRQN               17




// Be sure to change this to your login...
const char login[] = "xu1392";

void internal_clock(void);
void display_float(float);
void control(void);

//============================================================================
// setup_adc()    (Autotest #1)
// Configure the ADC peripheral and analog input pins.
// Parameters: none
//============================================================================
void setup_adc(void){
  // enable RCC clock for GPIOA.
  RCC -> AHBENR  |= RCC_AHBENR_GPIOAEN;
  // configue PA0 and PA1 as DAC input pin.
  GPIOA -> MODER |= PA01_ANALOG_MODE;
  // enable RCC clock for adc.
  RCC -> APB2ENR |=  RCC_APB2ENR_ADCEN;
  // turn on 14Mhz clock.
  RCC -> CR2 |= RCC_CR2_HSI14_ON;
  // wait for clock ready.
  while(!(RCC -> CR2 & RCC_CR2_HSI14_RDY))
  // enable adc in ADC_CR.
  ADC1 -> CR |= ADC_CR_ADEN;
  // wait for ADC ready.
  while (!(ADC1 -> ISR & ADC_ISR_ADRDY));
}

//============================================================================
// start_adc_channel()    (Autotest #2)
// Select an ADC channel, and initiate an A-to-D conversion.
// Parameters: n: channel number
//============================================================================
void start_adc_channel(int n){
  ADC1 -> CHSELR = 0;
  ADC1 -> CHSELR |= 1 << n;
  while(!(ADC1 -> ISR & ADC_ISR_ADRDY));
  ADC1 -> CR |= ADC_CR_ADSTART;
}

//============================================================================
// read_adc()    (Autotest #3)
// Wait for A-to-D conversion to complete, and return the result.
// Parameters: none
// Return value: converted result
//============================================================================
int read_adc(void){
  int res;
  char temp[20];
  while(!(ADC1 -> ISR & ADC_ISR_EOC));
  sprintf(temp, "%d", ADC1 -> DR);
  res = atoi(temp);
  return res;
}

//============================================================================
// setup_dac()    (Autotest #4)
// Configure the DAC peripheral and analog output pin.
// Parameters: none
//============================================================================
void setup_dac(void){
  RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
  GPIOA -> MODER |= PA4_ANALOG_MODE;
  RCC -> APB1ENR |= RCC_APB1ENR_DACEN;
  DAC -> CR |= SOFTWARE_TRIGER1;
  DAC -> CR |= DAC_CR_EN1;
}

//============================================================================
// write_dac()    (Autotest #5)
// Write a sample to the right-aligned 12-bit DHR, and trigger conversion.
// Parameters: sample: value to write to the DHR
//============================================================================
void write_dac(int sample){
  while((DAC -> SWTRIGR & DAC_SWTRIGR_SWTRIG1) == DAC_SWTRIGR_SWTRIG1);
  DAC -> DHR12R1 = sample;
  DAC -> SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
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
// Timer 6 ISR    (Autotest #8)
// The ISR for Timer 6 which computes the DAC samples.
// Parameters: none
// (Write the entire subroutine below.)
//============================================================================
void TIM6_DAC_IRQHandler() {
  // acknowledge interrupt.
  TIM6 -> SR &= !(TIM_SR_UIF);
  DAC -> SWTRIGR |= DAC_SWTRIGR_SWTRIG1;
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
  sample = (sample >> 5) + 2048;
  if (sample > 4095) sample = 4095;
  else if (sample < 0) sample = 0;
  DAC -> DHR12R1 = sample;

}

//============================================================================
// setup_tim6()    (Autotest #9)
// Configure Timer 6 to raise an interrupt RATE times per second.
// Parameters: none
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
  NVIC -> ISER[0] |= (1 << TIM6_DAC_IRQN);
}

int main(void)
{
  //internal_clock(); // Use the internal oscillator if you need it
  autotest(); // test all of the subroutines you wrote
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
