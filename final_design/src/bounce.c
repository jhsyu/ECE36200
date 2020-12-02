
#include "stm32f0xx.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h"

#define TIM1_BDTR_MOE           (1 << 15)
#define TIM1_CCMR2_OC4PE    (1 << 11)
#define TIM1_CCER_CC1E      (1 << 0 )
#define TIM1_CCER_CC2E      (1 << 4 )
#define TIM1_CCER_CC3E      (1 << 8 )
#define TIM1_CCER_CC4E      (1 << 12)


void setup_tim17()
{
  // Set this to invoke the ISR 100 times per second.
  RCC -> APB2ENR |= RCC_APB2ENR_TIM17EN;
  // set ARR and PSC value.
  TIM17 -> PSC = 800 - 1;
  TIM17 -> ARR = 600 - 1;
  // write UIE bit.
  TIM17 -> DIER |= TIM_DIER_UIE;
  TIM17 -> CR1 |= TIM_CR1_CEN;
  NVIC -> ISER[0] |= (1 << TIM17_IRQn);
}

void setup_portb()
{
  // Enable Port B.
  RCC -> AHBENR |= RCC_AHBENR_GPIOBEN;
  // Set PB0-PB3 for output.
  GPIOB -> MODER &= ~(0xff);
  GPIOB -> MODER |=  (0x55);
  // Set PB4-PB7 for input and enable pull-down resistors.
  GPIOB -> MODER &= ~(0xff00);
  GPIOB -> PUPDR &= ~(0xff00);
  GPIOB -> PUPDR |=  (0xaa00);

  // Turn on the output for the lower row. (PB0)
  GPIOB -> ODR |= 1 << 3;

}

char check_key()
{
  char rtv = 0;
  int key = ((GPIOB -> IDR) >> 4) & 0xf;
  // If the '*' key is pressed (key == 1), return '*'
  if ((key & 1) == 1) rtv = '*';
  else if ((key & 2) == 2) rtv = '0';
  else if ((key & 4) == 4) rtv = '#';
  // If the 'D' key is pressed (key == 8), return 'D'
  else if ((key & 8) == 8) rtv = 'D';
  // Otherwise, return 0
  return rtv;
}

void setup_spi1()
{
  // Use setup_spi1() from lab 8.
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

//============================================================================
// segment display functions
//============================================================================
char offset = 0; // decide whihch segment display to light. (0-7)
char display[8]; // the data buffer for 8 segment display.
int score = 0;
int high = 0;

int pause = 0;
int dead = 1;
char str[20];
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
  ['p'] = 0x73,
  ['t'] = 0x78
};

void setup_portc() {
  RCC -> AHBENR |= RCC_AHBENR_GPIOCEN;
  // set PC0 - PC10 as output.
  for (int i = 0; i < 11; i++) {
    GPIOC -> MODER &= ~(0b11 << (2*i));
    GPIOC -> MODER |=  (0b01 << (2*i));
  }

}

void show_digit(){
	int off = offset & 7;
	GPIOC -> ODR = (off << 8) | display[off];
}

void segdisp_inil() {
  memset(display, 0, 8);
  for (int i = 0; i < 8; i++) {
    display[i] = font['0'];
  }
  display[6] = font['p'];
  display[7] = font['t'];
}

void segdisp_update() {
  char buffer[6];
  itoa(score, buffer, 10);
  for (int i = 0; i < 6; i++) {
    display[i] = font[buffer[i]];
  }
}

void TIM16_IRQHandler(){
	TIM16 -> SR &= !(TIM_SR_UIF);
  segdisp_update();
	show_digit();
  offset = (offset + 1) & 0x7; // count 0 ... 7 and repeat
}

void setup_tim16(){
	// enable RCC clock for TIM16.
	RCC -> APB2ENR |=  RCC_APB2ENR_TIM16EN;
	// set ARR and PSC value.
	TIM16 -> PSC = 80 - 1;
	TIM16 -> ARR = 60 - 1;
	// write UIE bit.
	TIM16 -> DIER |= TIM_DIER_UIE;
	TIM16 -> CR1 |= TIM_CR1_CEN;
	NVIC -> ISER[0] |= (1 << TIM16_IRQn);
}


//============================================================================
// OLED display functions
//============================================================================
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


//============================================================================
// TIM1: PWM output.
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
// RGB LED functions
//============================================================================
void setrgb(int r, int g, int b){

	int arr = TIM1 -> ARR;
	r = ((r >> 4) * 10 + (r & 0xf)) * (arr + 1) / 100;
	g = ((g >> 4) * 10 + (g & 0xf)) * (arr + 1) / 100;
	b = ((b >> 4) * 10 + (b & 0xf)) * (arr + 1) / 100;

	TIM1 -> CCR1 = (arr + 1) - r;
	TIM1 -> CCR2 = (arr + 1) - g;
	TIM1 -> CCR3 = (arr + 1) - b;
}
int red = 0;
int green = 45;
int blue = 95;
int step_r = 1;
int step_g = 1;
int step_b = 1;

void TIM15_IRQHandler(){
	TIM15 -> SR &= !(TIM_SR_UIF);
  red += step_r;
  green += step_g;
  blue += step_b;
  if (red == 99) {
    step_r = -1;
    red += step_r;
  }
  if (red == 1) {
    step_r = 1;
    red += step_r;
  }

  if (green == 99) {
    step_g = -1;
    green += step_g;
  }
  if (green == 1) {
    step_g = 1;
    green += step_g;
  }

  if (blue == 99) {
    step_b = -1;
    blue += step_b;
  }
  if (blue == 5) {
    step_b = 1;
    blue += step_b;
  }

  setrgb(red, green, blue);
}

void setup_tim15(){
	// enable RCC clock for TIM15.
	RCC -> APB2ENR |=  RCC_APB2ENR_TIM15EN;
	// set ARR and PSC value.
	TIM15 -> PSC = 8000 - 1;
	TIM15 -> ARR = 300 - 1;
	// write UIE bit.
	TIM15 -> DIER |= TIM_DIER_UIE;
	TIM15 -> CR1 |= TIM_CR1_CEN;
	NVIC -> ISER[0] |= (1 << TIM15_IRQn);
}




// Copy a subset of a large source picture into a smaller destination.
// sx,sy are the offset into the source picture.
void pic_subset(Picture *dst, const Picture *src, int sx, int sy)
{
    int dw = dst->width;
    int dh = dst->height;
    if (dw + sx > src->width)
        for(;;)
            ;
    if (dh + sy > src->height)
        for(;;)
            ;
    for(int y=0; y<dh; y++)
        for(int x=0; x<dw; x++)
            dst->pix2[dw * y + x] = src->pix2[src->width * (y+sy) + x + sx];
}

// Overlay a picture onto a destination picture.
// xoffset,yoffset are the offset into the destination picture that the
// source picture is placed.
// Any pixel in the source that is the 'transparent' color will not be
// copied.  This defines a color in the source that can be used as a
// transparent color.
void pic_overlay(Picture *dst, int xoffset, int yoffset, const Picture *src, int transparent)
{
    for(int y=0; y<src->height; y++) {
        int dy = y+yoffset;
        if (dy < 0 || dy >= dst->height)
            continue;
        for(int x=0; x<src->width; x++) {
            int dx = x+xoffset;
            if (dx < 0 || dx >= dst->width)
                continue;
            unsigned short int p = src->pix2[y*src->width + x];
            if (p != transparent)
                dst->pix2[dy*dst->width + dx] = p;
        }
    }
}

// Called after a bounce, update the x,y velocity components in a
// pseudo random way.  (+/- 1)
void perturb(int *vx, int *vy)
{
    if (*vx > 0) {
        *vx += (random()%3) - 1;
        if (*vx >= 3)
            *vx = 2;
        if (*vx == 0)
            *vx = 1;
    } else {
        *vx += (random()%3) - 1;
        if (*vx <= -3)
            *vx = -2;
        if (*vx == 0)
            *vx = -1;
    }
    if (*vy > 0) {
        *vy += (random()%3) - 1;
        if (*vy >= 3)
            *vy = 2;
        if (*vy == 0)
            *vy = 1;
    } else {
        *vy += (random()%3) - 1;
        if (*vy <= -3)
            *vy = -2;
        if (*vy == 0)
            *vy = -1;
    }
}

extern const Picture background; // A 240x320 background image
extern const Picture ball; // A 19x19 purple ball with white boundaries
extern const Picture paddle; // A 59x5 paddle

const int border = 20;
int xmin; // Farthest to the left the center of the ball can go
int xmax; // Farthest to the right the center of the ball can go
int ymin; // Farthest to the top the center of the ball can go
int ymax; // Farthest to the bottom the center of the ball can go
int x,y; // Center of ball
int vx,vy; // Velocity components of ball

int px; // Center of paddle offset
int newpx; // New center of paddle

// This C macro will create an array of Picture elements.
// Really, you'll just use it as a pointer to a single Picture
// element with an internal pix2[] array large enough to hold
// an image of the specified size.
// BE CAREFUL HOW LARGE OF A PICTURE YOU TRY TO CREATE:
// A 100x100 picture uses 20000 bytes.  You have 32768 bytes of SRAM.
#define TempPicturePtr(name,width,height) Picture name[(width)*(height)/6+2] = { {width,height,2} }

// Create a 29x29 object to hold the ball plus padding
TempPicturePtr(object,29,29);

//============================================================================
// Timer 17 ISR()
// The Timer 17 ISR: calculate the bouncing of the ball.
// Parameters: none
//============================================================================
void TIM17_IRQHandler(void)
{
    TIM17->SR &= ~TIM_SR_UIF;
    char key = check_key();
    if (key == '#') {
      /* reset the game*/
      x = (xmin+xmax)/2; // Center of ball
      y = ymin;
      vx = 0; // Velocity components of ball
      vy = 1;
      dead = 0;
      score = 0;
      spi_display1("Welcome!         ");
      sprintf(str, "High: %d", high);
      spi_display2(str);
    }
    if (dead == 1) return;
    if (key == '*')
        newpx -= 1;
    else if (key == 'D')
        newpx += 1;
    if (newpx - paddle.width/2 <= border || newpx + paddle.width/2 >= 240-border)
        newpx = px;
    if (newpx != px) {
        px = newpx;
        // Create a temporary object two pixels wider than the paddle.
        // Copy the background into it.
        // Overlay the paddle image into the center.
        // Draw the result.
        //
        // As long as the paddle moves no more than one pixel to the left or right
        // it will clear the previous parts of the paddle from the screen.
        TempPicturePtr(tmp,61,5);
        pic_subset(tmp, &background, px-tmp->width/2, background.height-border-tmp->height); // Copy the background
        pic_overlay(tmp, 1, 0, &paddle, -1);
        LCD_DrawPicture(px-tmp->width/2, background.height-border-tmp->height, tmp);
    }
    x += vx;
    y += vy;
    if (x <= xmin) {
        // Ball hit the left wall.
        vx = - vx;
        if (x < xmin)
            x += vx;
        perturb(&vx,&vy);
    }
    if (x >= xmax) {
        // Ball hit the right wall.
        vx = -vx;
        if (x > xmax)
            x += vx;
        perturb(&vx,&vy);
    }
    if (y <= ymin) {
        // Ball hit the top wall.
        vy = - vy;
        if (y < ymin)
            y += vy;
        perturb(&vx,&vy);
    }
    if (y >= ymax - paddle.height &&
        x >= (px - paddle.width/2) &&
        x <= (px + paddle.width/2)) {
        // The ball has hit the paddle.  Bounce.
        if (!dead) {
          score += 1;
          if (high < score) {
            high = score;
            sprintf(str, "High: %d        ", high);
            spi_display1("Congratulations!");
            spi_display2(str);
          }
        }
        int pmax = ymax - paddle.height;
        vy = -vy;
        if (y > pmax)
            y += vy;
    }
    else if (y >= ymax) {
        // The ball has hit the bottom wall.  Set velocity of ball to 0,0.
        vx = 0;
        vy = 0;
        dead = 1;
        spi_display1("press # to start");

    }

    TempPicturePtr(tmp,29,29); // Create a temporary 29x29 image.
    pic_subset(tmp, &background, x-tmp->width/2, y-tmp->height/2); // Copy the background
    pic_overlay(tmp, 0,0, object, 0xffff); // Overlay the object
    pic_overlay(tmp, (px-paddle.width/2) - (x-tmp->width/2),
            (background.height-border-paddle.height) - (y-tmp->height/2),
            &paddle, 0xffff); // Draw the paddle into the image
    LCD_DrawPicture(x-tmp->width/2,y-tmp->height/2, tmp); // Re-draw it to the screen
    // The object has a 5-pixel border around it that holds the background
    // image.  As long as the object does not move more than 5 pixels (in any
    // direction) from it's previous location, it will clear the old object.
}

int main(void)
{
    setup_portb();
    setup_portc();
    setup_spi1();
    setup_spi2();
    LCD_Init();
    spi_init_oled();
    spi_display1("press # to start");
    sprintf(str, "High: %d        ", high);
    spi_display2(str);

    // Draw the background.
    LCD_DrawPicture(0,0,&background);

    // Set all pixels in the object to white.
    for(int i=0; i<29*29; i++)
        object->pix2[i] = 0xffff;

    // Center the 19x19 ball into center of the 29x29 object.
    // Now, the 19x19 ball has 5-pixel white borders in all directions.
    pic_overlay(object,5,5,&ball,0xffff);

    // Initialize the game state.
    xmin = border + ball.width/2;
    xmax = background.width - border - ball.width/2;
    ymin = border + ball.width/2;
    ymax = background.height - border - ball.height/2;
    x = (xmin+xmax)/2; // Center of ball
    y = ymin;
    vx = 0; // Velocity components of ball
    vy = 1;

    px = -1; // Center of paddle offset (invalid initial value to force update)
    newpx = (xmax+xmin)/2; // New center of paddle
    segdisp_inil();
    setup_tim1();
    setup_tim15();
    setup_tim16();
    setup_tim17();
}
