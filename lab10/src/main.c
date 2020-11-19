
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
  USART5 -> CR1 &= ~(USART_CR1_M0 | USART_CR1_M1);
  // Set it for one stop bit.
  USART5 -> CR2 &= ~(USART_CR2_STOP);
  // Set it for no parity.
  USART5 -> CR1 &= ~(USART_CR1_PCE);
  // Use 16x oversampling.
  USART5 -> CR1 &= ~(USART_CR1_OVER8);
  // Use a baud rate of 115200 (115.2 kbaud).
  USART5 -> BRR = 115200;
  // Enable the transmitter and the receiver by setting the TE and RE bits.
  USART5 -> CR1 |= USART_CR1_TE | USART_CR1_RE;
  // Enable the USART.
  USART5 -> CR1 |= USART_CR1_UE;
  // Finally, you should wait for the TE and RE bits to be acknowledged.
  // This indicates that the USART is ready to transmit and receive.
  while (USART5 -> CR1 & (USART_CR1_TE | USART_CR1_RE) != (USART_CR1_RE | USART_CR1_TE));
}


// Write your subroutines above.

const char testline[] =
  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789\r\n";

int main()
{
    setup_usart5();

    // Uncomment these when you're asked to...
    //setbuf(stdin, 0);
    //setbuf(stdout, 0);
    //setbuf(stderr, 0);

    // Test 2.2 simple_putchar()
    //
    //for(;;)
    //    for(const char *t=testline; *t; t++)
    //        simple_putchar(*t);

    // Test for 2.3 simple_getchar()
    //
    //for(;;)
    //    simple_putchar( simple_getchar() );

    // Test for 2.4 and 2.5 __io_putchar() and __io_getchar()
    //
    //printf("Hello!\n");
    //for(;;)
    //    putchar( getchar() );

    // Test for 2.6
    //
    //for(;;) {
    //    printf("Enter string: ");
    //    char line[100];
    //    fgets(line, 99, stdin);
    //    line[99] = '\0'; // just in case
    //    printf("You entered: %s", line);
    //}

    // Test for 2.7
    //
    //enable_tty_interrupt();
    //for(;;) {
    //    printf("Enter string: ");
    //    char line[100];
    //    fgets(line, 99, stdin);
    //    line[99] = '\0'; // just in case
    //    printf("You entered: %s", line);
    //}

    // Test for 2.8 Test the command shell and clock.
    //
    //enable_tty_interrupt();
    //setup_tim14();
    //FATFS fs_storage;
    //FATFS *fs = &fs_storage;
    //f_mount(fs, "", 1);
    //command_shell();

    return 0;
}
