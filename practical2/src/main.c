#include "stm32f0xx.h"
#include <stdio.h> // for printf()



// Configure timer TIM2 to raise an interrupt 10 times per second.
// f = 10hz.

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

void setup_usart5() {
  RCC -> AHBENR |= RCC_AHBENR_GPIOCEN | RCC_AHBENR_GPIODEN;

  // configure PC12 to be routed to USART5_TX: AF2
  GPIOC -> MODER &= ~(0b11 << (12 * 2));
  GPIOC -> MODER |=  (0b10 << (12 * 2));
  GPIOC -> AFR[1] &= ~(0b1111 << 16);
  GPIOC -> AFR[1] |=  (0b0010 << 16);
  // configure PD2 to be routed to USART5_RX: AF2
  GPIOD -> MODER &= ~(0b11 << (2 * 2));
  GPIOD -> MODER |=  (0b10 << (2 * 2));
  GPIOD -> AFR[0] &= ~(0b1111 << 8);
  GPIOD -> AFR[0] |=  (0b0010 << 8);
  // Enable the RCC clock to the USART5 peripheral
  RCC -> APB1ENR |= RCC_APB1ENR_USART5EN;

  // Configure USART5 as follows:
  // (First, disable it by turning off its UE bit.)
  USART5 -> CR1 &= ~USART_CR1_UE;
  // Set a word size of 8 bits.
  // 8bit: M[1:0] = 00
  USART5 -> CR1 &= ~USART_CR1_M;
  // Set it for one stop bit.
  USART5 -> CR2 &= ~(USART_CR2_STOP);
  // Set it for no parity.
  USART5 -> CR1 &= ~(USART_CR1_PCE);
  // Use 16x oversampling.
  USART5 -> CR1 &= ~(USART_CR1_OVER8);
  // Use a baud rate of 115200 (115.2 kbaud).
  USART5 -> BRR = (uint16_t) (48000000 / 115200);
  // Enable the transmitter and the receiver by setting the TE and RE bits.
  USART5 -> CR1 |= USART_CR1_TE | USART_CR1_RE;
  // Enable the USART.
  USART5 -> CR1 |= USART_CR1_UE;
  // Finally, you should wait for the TE and RE bits to be acknowledged.
  // This indicates that the USART is ready to transmit and receive.
  while (((USART5 -> ISR) & (USART_ISR_TEACK | USART_ISR_REACK)) !=
          (USART_ISR_REACK | USART_ISR_TEACK));
}


void setup_tim2() {
	RCC -> APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2 -> PSC = 48000 - 1;
	TIM2 -> ARR = 100 - 1;
	TIM2 -> DIER |= TIM_DIER_UIE;
	TIM2 -> CR1 |= TIM_CR1_CEN;
	NVIC -> ISER[0] |= (1 << 15);

	GPIOB -> ODR |= 1;		// set row 1 high.
}

int getkey() {
	char key = 0;
	int cols = (GPIOB->IDR >> 4) & 0xf;
	if ((cols & 1) == 1) {
		// 1 pressed
		key = 1;
	}
	else if ((cols & 2 )== 2) {
		// 2 pressed
		key = 2;
	}
	return key;
}

int better_putchar(int ch) {
  if ((char)ch == '\n') {
    // wait for the TXE bit to be set
    while (((USART5 -> ISR) & USART_ISR_TXE) != USART_ISR_TXE);
    USART5 -> TDR = (uint8_t) ch;
    // write a '\r' to the TDR
    while (((USART5 -> ISR) & USART_ISR_TXE) != USART_ISR_TXE);
    USART5 -> TDR = (uint8_t) '\r';
    return ch;
  }
  while (((USART5 -> ISR) & USART_ISR_TXE) != USART_ISR_TXE);
  USART5 -> TDR = (uint8_t) ch;
  return ch;
}

int __io_putchar(int ch) {
  return better_putchar(ch);
}



int cnt_val = 0;
int cnt_en = 0;

void TIM2_IRQHandler() {
	// acknowledge the interrupt.
	TIM2 -> SR &= ~(TIM_SR_UIF);
	char key = getkey();
	// Check if the keypad button '1' is pressed.
	if (key == 1) {
		// enable the counter to be incremented,
		cnt_en = 1;
		// but set the counter to zero.
		cnt_val = 0;
		// As long as the button is pressed, the counter should remain zero.
		// When the button is no longer pressed,
		// the ISR will increment the counter.
		// This counter should then increment 10 times per second.
		for (;;) {
			key = getkey();
			if (key != 1) break;
		}
	}

	// Check if the keypad button '2' is pressed.
	else if (key == 2) {
		// If the counter is being incremented, turn off the variable
		// that enables the counter to be increment by the ISR.
		if (cnt_en == 1) cnt_en = 0;
	}

	if (cnt_en == 1) {
		cnt_val += 1;
		// Display the counter every time it is incremented
		// by sending the number to USART5.
		char buffer[10];
		sprintf(buffer, "%08d\n", cnt_val);
		printf(buffer);
	}
}

int main(void) {
	enable_ports();
	setup_tim2();
	setup_usart5();
	printf("welcome!\n");
	printf("press 1 to reset/start, 2 to stop\n");
}
