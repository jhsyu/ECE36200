
//============================================================================
// ECE 362 lab experiment 10 -- Asynchronous Serial Communication
//============================================================================

#include "stm32f0xx.h"
#include "ff.h"
#include "diskio.h"
#include "fifo.h"
#include "tty.h"
#include <string.h> // for memset()
#include <stdio.h> // for printf()

void advance_fattime(void);
void command_shell(void);
struct fifo input_fifo;  // input buffer

// Write your subroutines below.
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

int simple_putchar(int ch) {
  // Wait for the USART5 ISR TXE to be set.
  while (((USART5 -> ISR) & USART_ISR_TXE) != USART_ISR_TXE);
  // Write the argument to the USART5 TDR (transmit data register).
  USART5 -> TDR = (uint8_t) ch;
  // Return the argument that was passed in.
  return ch;
}

int simple_getchar() {
  int rdata = 0;
  while ((USART5 -> ISR & USART_ISR_RXNE) != USART_ISR_RXNE);
  rdata = USART5 -> RDR;
  return rdata;
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

int better_getchar() {
  int rdata = 0;
  while ((USART5 -> ISR & USART_ISR_RXNE) != USART_ISR_RXNE);
  rdata = USART5 -> RDR;
  if (rdata == '\r') rdata = '\n';
  return rdata;
}

int interrupt_getchar() {
  char ch;
  while(fifo_newline(&input_fifo) == 0) {
    asm volatile ("wfi");
  }
  ch = fifo_remove(&input_fifo);
  return ch;
}

void USART3_4_5_6_7_8_IRQHandler() {
  // Check and clear the ORE flag.
  if ((USART5 -> ISR & USART_ISR_ORE) == USART_ISR_ORE)
  USART5 -> ICR |= USART_ICR_ORECF;
  // Read the new character from the USART5 RDR.
  char newch = USART5 -> RDR;
  // Check if the input_fifo is full. If it is, return from the ISR.
  // (Throw away the character.)
  int rtv = fifo_full(&input_fifo);
  if (rtv == 1) return;
  // Call insert_echo_char() with the character.
  insert_echo_char(newch);

}

void enable_tty_interrupt() {
  USART5 -> CR1 |= USART_CR1_RXNEIE;
  NVIC -> ISER[0] |= (1 << 29);
}

void setup_spi1() {
  // Enable the RCC clock to GPIOA.
  RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
  // Configure PA1 to be a general-purpose output.
  GPIOA -> MODER &= ~(0b11 << (1*2));
  GPIOA -> MODER |=  (0b01 << (1*2));
  // Configure GPIOA so that pins 5, 6, and 7 are routed to SPI1.
  // PA5: SPI1_SCK
  // PA6: SPI1_MISO
  // PA7: SPI1_MOSI
  GPIOA -> MODER &= ~(0x3f << (5*2));
  GPIOA -> MODER |=  (0x2a << (5*2));
  GPIOA -> AFR[0] &= ~(0xfff << (4*5));
  // Enable the internal pull-up resistor for pin 6 (MISO).
  GPIOA -> PUPDR &= ~(0b11 << (6*2));
  GPIOA -> PUPDR |=  (0b01 << (6*2));
  // Enable the RCC clock to the SPI1 peripheral.
  RCC -> APB2ENR |= RCC_APB2ENR_SPI1EN;
  // Disable the SPI1 peripheral by turning off the SPE bit.
  SPI1 -> CR1 &= ~SPI_CR1_SPE;
  // Set it for as low a baud rate as possible.
  SPI1 -> CR1 &= ~SPI_CR1_BR;
  // Ensure that BIDIMODE and BIDIOE are cleared.
  SPI1 -> CR1 &= ~(SPI_CR1_BIDIOE | SPI_CR1_BIDIMODE);
  // Enable Master mode.
  SPI1 -> CR1 |= SPI_CR1_MSTR;
  // Set NSSP and configure the peripheral for an 8-bit word (default).
  SPI1 -> CR2 |= SPI_CR2_NSSP;
  SPI1 -> CR2 &= ~SPI_CR2_DS;
  SPI1 -> CR2 |= SPI_CR2_DS_0 | SPI_CR2_DS_1 | SPI_CR2_DS_2;
  // Set the bit that sets the FIFO-reception threshold to 8-bits.
  SPI1 -> CR2 |= SPI_CR2_FRXTH;
  // Enable the SPI1 peripheral.
  SPI1 -> CR1 |= SPI_CR1_SPE;
}

void spi_high_speed() {
  // Disables the SPI1 SPE bit.
  SPI1 -> CR1 &= ~SPI_CR1_SPE;
  // Configure SPI1 for a 6 MHz SCK rate.
  SPI1 -> CR1 &= ~SPI_CR1_BR;
  SPI1 -> CR1 |= SPI_CR1_BR_1;
  // Re-enable the SPI1 SPE bit.
  SPI1 -> CR1 |= SPI_CR1_SPE;
}

void TIM14_IRQHandler() {
  // Acknowledge the interrupt.
  TIM14 -> SR &= ~(TIM_SR_UIF);
  // Invoke advance_fattime()
  advance_fattime();
}

void setup_tim14() {
  // f = 0.5hz.
  RCC -> APB1ENR |= RCC_APB1ENR_TIM14EN;
  TIM14 -> PSC = 960000 - 1;
  TIM14 -> ARR = 100 - 1;
  TIM14 -> DIER |= TIM_DIER_UIE;
  TIM14 -> CR1 |= TIM_CR1_CEN;
  NVIC -> ISER[0] |= (1 << 19);

}

int __io_putchar(int ch) {
  return better_putchar(ch);
}

int __io_getchar(void) {
  return interrupt_getchar();
}



// Write your subroutines above.

const char testline[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789\r\n";

int main()
{
    setup_usart5();

    // Uncomment these when you're asked to...
    setbuf(stdin, 0);
    setbuf(stdout, 0);
    setbuf(stderr, 0);

    // Test 2.2 simple_putchar()
    //
    // for(;;)
    //     for(const char *t=testline; *t; t++)
    //         simple_putchar(*t);

    // Test for 2.3 simple_getchar()
    //
    // for(;;)
    //     simple_putchar( simple_getchar() );

    // Test for 2.4 and 2.5 __io_putchar() and __io_getchar()
    //
    // printf("Hello!\n");
    // for(;;)
    //     putchar( getchar() );

    // Test for 2.6
    //
    // for(;;) {
    //     printf("Enter string: ");
    //     char line[100];
    //     fgets(line, 99, stdin);
    //     line[99] = '\0'; // just in case
    //     printf("You entered: %s", line);
    // }

    // Test for 2.7
    //
    // enable_tty_interrupt();
    // for(;;) {
    //     printf("Enter string: ");
    //     char line[100];
    //     fgets(line, 99, stdin);
    //     line[99] = '\0'; // just in case
    //     printf("You entered: %s", line);
    // }

    // Test for 2.8 Test the command shell and clock.

    enable_tty_interrupt();
    setup_tim14();
    FATFS fs_storage;
    FATFS *fs = &fs_storage;
    f_mount(fs, "", 1);
    command_shell();

    return 0;
}
