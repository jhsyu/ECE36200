
//============================================================================
// ECE 362 lab experiment 8 -- SPI and DMA
//============================================================================

#include "stm32f0xx.h"
#include "lcd.h"
#include <stdio.h> // for sprintf()

// Be sure to change this to your login...
const char login[] = "xu1392";


// Prototypes for misc things in lcd.c
void nano_wait(unsigned int);

// Write your subroutines below.

void setup_bb() {
	// enable RCC clock for GPIOB
	RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
	// Configure PB12, 13, 15 as output.
	// NSS: 	PB12
	// SCK: 	PB13
	// MOSI: 	PB15
	GPIOB -> MODER &= ~(0b11001111 << (12 * 2));
	GPIOB -> MODER |=  (0b01000101 << (12 * 2));
	// initialization: PB12 -> high (NSS)
	GPIOB -> ODR |= 1 << 12;
	//								 PB13 -> low  (SCK)
	GPIOB -> ODR &= ~(1 << 13);
	GPIOB -> ODR &= ~(1 << 15);

}

void small_delay() {
	nano_wait(100000);
}

void bb_write_bit(int bit_value) {
	// pb15 is the mosi pin.
	GPIOB -> ODR &= ~(1 << 15);
	GPIOB -> ODR |= ((0b1 & bit_value) << 15);
	small_delay();
	// sck high. pb13
	GPIOB -> ODR |= (0b1 << 13);
	small_delay();
	// sck low.
	GPIOB -> ODR &= ~(0b1 << 13);
}

void bb_write_byte(int write_byte) {
	for (int i = 0; i < 8; i++) {
		bb_write_bit((write_byte >> (7-i)) & 1);
	}
}


void bb_cmd(int cmd) {
	//Set the NSS pin low to start an SPI transfer.
	GPIOB -> ODR &= ~(1 << 12);
	small_delay();
	bb_write_bit(0);
	bb_write_bit(0);
	bb_write_byte(cmd);
	small_delay();
	GPIOB -> ODR |= (1 << 12);
	small_delay();
}

void bb_data(int data) {
	//Set the NSS pin low to start an SPI transfer.
	GPIOB -> ODR &= ~(1 << 12);
	small_delay();
	bb_write_bit(1);
	bb_write_bit(0);
	bb_write_byte(data);
	small_delay();
	GPIOB -> ODR |= (1 << 12);
	small_delay();
}

void bb_init_oled() {
	nano_wait(1000000);
	bb_cmd(0x38); 				// set for 8-bit operation
	bb_cmd(0x08); 				// turn display off
	bb_cmd(0x01); 				// clear display
	nano_wait(2000000);  	// to wait 2 ms for the display to clear.
	bb_cmd(0x06); 				// set the display to scroll
	bb_cmd(0x02); 				// move the cursor to the home position
	bb_cmd(0x0c); 				// turn the display on
}

void bb_display1(const char* str) {
	bb_cmd(0x02); // move the cursor to the home position
	int ch;
	for (int i = 0; str[i] != '\0'; i++) {
		ch = (int) str[i];
		bb_data(ch);
	}
}
void bb_display2(const char* str) {
	// move the cursor to the lower row (offset 0x40)
	bb_cmd(0xc0);
	int ch;
	for (int i = 0; str[i] != '\0'; i++) {
		ch = (int) str[i];
		bb_data(ch);
	}
}

void setup_spi2(){
	RCC -> APB1ENR |= RCC_APB1ENR_SPI2EN;
	// set SPI2 as master mode and highest BR
	SPI2 -> CR1 |= SPI_CR1_MSTR | SPI_CR1_BR;
	SPI2 -> CR1 |= SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE;
	// set 10 bit word width, output enable, nssp.
	SPI2 -> CR2 = SPI_CR2_SSOE | SPI_CR2_NSSP | SPI_CR2_DS_3 | SPI_CR2_DS_0;
	SPI2 -> CR1 |= SPI_CR1_SPE;

	// set up GPIOB.
	RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
	// set PB12, PB13, PB15 as AF.
	GPIOB -> MODER &= ~(0b11001111 << (12 * 2));
	GPIOB -> MODER |=  (0b10001010 << (12 * 2));
	GPIOB -> AFR[1] &= ~(0xf0ff0000);
}

void spi_cmd(int cmd){
	while ((SPI2 -> SR & SPI_SR_TXE) != SPI_SR_TXE);
	SPI2 -> DR = cmd;
}

void spi_data(int data){
	while ((SPI2 -> SR & SPI_SR_TXE) != SPI_SR_TXE);
	SPI2 -> DR = (0x200 | data);
}

void spi_init_oled(){
	nano_wait(1000000);
	spi_cmd(0x38); 				// set for 8-bit operation
	spi_cmd(0x08); 				// turn display off
	spi_cmd(0x01); 				// clear display
	nano_wait(2000000);  	// to wait 2 ms for the display to clear.
	spi_cmd(0x06); 				// set the display to scroll
	spi_cmd(0x02); 				// move the cursor to the home position
	spi_cmd(0x0c); 				// turn the display on
}

void spi_display1(const char* str){
	spi_cmd(0x02); // move the cursor to the home position
	int ch;
	for (int i = 0; str[i] != '\0'; i++) {
		ch = (int) str[i];
		spi_data(ch);
	}
}

void spi_display2(const char* str){
	// move the cursor to the lower row (offset 0x40)
	spi_cmd(0xc0);
	int ch;
	for (int i = 0; str[i] != '\0'; i++) {
		ch = (int) str[i];
		spi_data(ch);
	}
}

void spi_enable_dma(const short* p){
	RCC -> AHBENR |= RCC_AHBENR_DMA1EN;
	SPI2 -> CR2 |= SPI_CR2_TXDMAEN;
	DMA1_Channel5 -> CCR &= ~ DMA_CCR_EN;
	DMA1_Channel5 -> CPAR = (uint32_t) (0x40003800 + 0x0c);
	DMA1_Channel5 -> CMAR = (uint32_t) p;
	DMA1_Channel5 -> CNDTR = (uint16_t) 34;
	DMA1_Channel5 -> CCR |= DMA_CCR_DIR;
	DMA1_Channel5 -> CCR |= DMA_CCR_MINC;
	DMA1_Channel5 -> CCR &= ~DMA_CCR_MSIZE;
	DMA1_Channel5 -> CCR |= DMA_CCR_MSIZE_0;
	DMA1_Channel5 -> CCR &= ~DMA_CCR_PSIZE;
	DMA1_Channel5 -> CCR |= DMA_CCR_PSIZE_0;
	DMA1_Channel5 -> CCR |= DMA_CCR_CIRC;
	DMA1_Channel5 -> CCR |=   DMA_CCR_EN;

	SPI2 -> CR2 |= SPI_CR2_TXEIE;
}

void setup_spi1(){
	RCC -> AHBENR |= RCC_AHBENR_GPIOAEN;
	GPIOA -> MODER &= ~(0xcf00);
	GPIOA -> MODER |= 0b10001010 << (4*2);
	GPIOA -> AFR[0] &= ~(0xf0ff0000);
	GPIOA -> MODER &= ~(0xf0);
	GPIOA -> MODER |= 0b01010000;
	RCC -> APB2ENR |= RCC_APB2ENR_SPI1EN;

	SPI1 -> CR1 &= ~(SPI_CR1_BR_0 | SPI_CR1_BR_1 | SPI_CR1_BR_2);
	SPI1 -> CR1 |= SPI_CR1_MSTR;
	SPI1 -> CR1 |= SPI_CR1_BIDIOE | SPI_CR1_BIDIMODE;
	SPI1 -> CR2 = SPI_CR2_SSOE | SPI_CR2_NSSP;
	SPI1 -> CR1 |= SPI_CR1_SPE;
}


// Write your subroutines above.

void show_counter(short buffer[])
{
	for(int i=0; i<10000; i++) {
		char line[17];
		sprintf(line,"% 16d", i);
		for(int b=0; b<16; b++)
		buffer[1+b] = line[b] | 0x200;
	}
}

void internal_clock();
void demo();
void autotest();

extern const Picture *image;

int main(void)
{
	//internal_clock();
	//demo();
	autotest();

	//setup_bb();
	//bb_init_oled();
	//bb_display1("Hello,");
	//bb_display2(login);

	//setup_spi2();
	//spi_init_oled();
	//spi_display1("Hello again,");
	//spi_display2(login);

	short buffer[34] = {
		0x02, // This word sets the cursor to the beginning of line 1.
		// Line 1 consists of spaces (0x20)
		0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
		0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
		0xc0, // This word sets the cursor to the beginning of line 2.
		// Line 2 consists of spaces (0x20)
		0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
		0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220, 0x220,
	};

	//spi_enable_dma(buffer);
	//show_counter(buffer);

	//setup_spi1();
	LCD_Init();
	LCD_Clear(BLACK);
	LCD_DrawLine(10,20,100,200, WHITE);
	LCD_DrawRectangle(10,20,100,200, GREEN);
	LCD_DrawFillRectangle(120,20,220,200, RED);
	LCD_Circle(50, 260, 50, 1, BLUE);
	LCD_DrawFillTriangle(130,130, 130,200, 190,160, YELLOW);
	LCD_DrawChar(150,155, BLACK, WHITE, 'X', 16, 1);
	LCD_DrawString(140,60,  WHITE, BLACK, "ECE 362", 16, 0);
	LCD_DrawString(140,80,  WHITE, BLACK, "has the", 16, 1);
	LCD_DrawString(130,100, BLACK, GREEN, "best toys", 16, 0);
	LCD_DrawPicture(110,220,(const Picture *)&image);
}
