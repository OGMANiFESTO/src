/*
 * PONGArcade.c
 *
 * Created: 12/2/2019 5:05:19 PM
 * Author : manif
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include "io.c"


volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}


// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void adc_init(){
	ADMUX = (1<<REFS0);
	
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

unsigned short readadc(uint8_t ch)
{
	ch&=0b00000111;         //ANDing to limit input to 7
	ADMUX = (ADMUX & 0xf8)|ch;  //Clear last 3 bits of ADMUX, OR with ch
	ADCSRA|=(1<<ADSC);        //START CONVERSION
	while((ADCSRA)&(1<<ADSC));    //WAIT UNTIL CONVERSION IS COMPLETE
	return(ADC);        //RETURN ADC VALUE
}

/*typedef struct _task {
	unsigned char state;                  // Task's current state
	unsigned long int period;       // Task period
	unsigned long int elapsedTime;  // Time elapsed since last task tick
	int (*TickFct)(int);        // Task tick function
} task;*/

//GLOBAL VARIABLES//

unsigned short movep1 = 0;
unsigned short movep2 = 0;
unsigned char p1Down, p1Up;
unsigned char p2Down, p2Up;
unsigned short count = 0;
unsigned short count1 = 0;
unsigned short winperiod = 0;
unsigned long lcdperiod = 0;
unsigned short ballperiod = 0;
unsigned short player1p = 0;
unsigned short player2p = 0;
unsigned short i, j;
unsigned short ballyval = 0;
unsigned short ballxval = 0;

unsigned char column_sel1 = 0x00;
unsigned char column_sel2 = 0x00;
unsigned char column_sel3 = 0xF7; 

int arr[8] = {};

int arr2[8] = {};
	
//GLOBAL VARIABLES

enum PLAYER1_States {Wait, Up, Down} PLAYER1_State; 
enum PLAYER2_States {Wait2, Up2, Down2} PLAYER2_State; 
enum PLAYER3_States {UP_Left, UP_Right, DOWN_Left, DOWN_Right, Slide_right, Slide_Left, P1Score, P2Score} PLAYER3_State;// ball direction states

void TickFct_Player1()
{
	switch(PLAYER1_State)
	{
		case Wait:
			if ((p1Down == 0x00) && (p1Up == 0x00)) { PLAYER1_State = Wait; }
		
			else if ((p1Down == 0x01) && (p1Up == 0x00)) { PLAYER1_State = Up; }
		
			else if ((p1Down == 0x00) && (p1Up == 0x01)) { PLAYER1_State = Down; }
		
			else if ((p1Down == 0x01) && (p1Up == 0x01)) { PLAYER1_State = Wait; }
		
			break;

		case Up:
			if ((movep1 < 6) && (count <= 0)) { movep1 += 1; }

			if ((p1Down == 0x00) && (p1Up == 0x00)) {
				PLAYER1_State = Wait;
				count = 0;
			}
		
			else if ((p1Down == 0x01) && (p1Up == 0x00)) {
				count += 1;
				if (count >= 200) {
					count = 0;
				}
				PLAYER1_State = Up;
			}
		
			else if ((p1Down == 0x00) && (p1Up == 0x01)) {
				PLAYER1_State = Down;
				count = 0;
			}
			else if ((p1Down == 0x01) && (p1Up == 0x01)) {
				PLAYER1_State = Wait;
				count = 0;
			}
			break;

		case Down:
			if ((movep1 > 1) && (count <= 0)) {
				movep1 = movep1 - 1;
			}

			if ((p1Down == 0x00) && (p1Up == 0x00)) {
				PLAYER1_State = Wait;
				count = 0;
			}
		
			else if ((p1Down == 0x01) && (p1Up == 0x00)) {
				PLAYER1_State = Up;
				count = 0;
			}
		
			else if ((p1Down == 0x00) && (p1Up == 0x01)) {
				PLAYER1_State = Down;
				count += 1;
				if (count >= 200) {
					count = 0;
				}
			}

			else if ((p1Down == 0x01) && (p1Up == 0x01)) {
				PLAYER1_State = Wait;
				count = 0;
			}
			break;
		}
}

void TickFct_Player2()
{
	switch(PLAYER2_State) {
		case Wait2:
			if ((p2Down == 0x00) && (p2Up == 0x00)) { PLAYER2_State = Wait2; }
		
			else if ((p2Down == 0x01) && (p2Up == 0x00)) { PLAYER2_State = Up2; }
		
			else if ((p2Down == 0x00) && (p2Up == 0x01)) { PLAYER2_State = Down2; }
		
			else if ((p2Down == 0x01) && (p2Up == 0x01)) { PLAYER2_State = Wait2; }
		
			break;

		case Up2:
			if ((movep2 < 6) && (count1 == 0)) { movep2 += 1; }

			if ((p2Down == 0x00) && (p2Up == 0x00)) {
				PLAYER2_State = Wait2;
				count1 = 0;
			}
		
			else if ((p2Down == 0x01) && (p2Up == 0x00)) {
				count1 = count1 +1;
				if (count1 >= 200) {
					count1 = 0;
				}
				PLAYER2_State = Up2;
			}
		
			else if ((p2Down == 0x00) && (p2Up == 0x01)) {
				PLAYER2_State = Down2;
				count1= 0;
			}
			
			else if ((p2Down == 0x01) && (p2Up == 0x01)) {
				PLAYER2_State = Wait2;
				count1 = 0;
			}
			
			break;

		case Down2:
			if ((movep2 > 1) && (count1 <= 0)) { movep2 -= 1; }

			if ((p2Down == 0x00) && (p2Up == 0x00)) {
				PLAYER2_State = Wait2;
				count1 = 0;
			}
			
			else if ((p2Down == 0x01) && (p2Up == 0x00)) {
				PLAYER2_State = Up2;
				count1 = 0;
			}
		
			else if ((p2Down == 0x00) && (p2Up == 0x01)) {
				PLAYER2_State = Down2;
				count1 = count1 + 1;
				if (count1 >= 200) { count1 = 0; }
			}

			else if ((p2Down == 0x01) && (p2Up == 0x01)) {
				PLAYER2_State = Wait2;
			count1 = 0;
		}
		break;
	}
}

void BallMovement ()
{
	ballperiod += 1;
	if (ballperiod >= 160)
	{
		ballperiod = 0;
		switch(PLAYER3_State)
		{
			case Slide_right:
			if ((ballyval == movep2 ) && (ballxval == 6))
			{
				PLAYER3_State = UP_Left;
			}
			else if ((ballyval == movep2 + 2) && (ballxval == 6))
			{
				PLAYER3_State = DOWN_Left;
			}
			else if ((ballyval == movep2 + 1) && (ballxval == 6))
			{
				PLAYER3_State = Slide_Left;
			}
			else if ((abs(ballyval - movep2) >= 3 )  && (ballxval == 7))
			{
				PLAYER3_State = P1Score;
			}
			else {

				ballxval = ballxval + 1;
			}

			break;

			case Slide_Left:
			if ((ballyval == movep1) && (ballxval == 1))
			{
				PLAYER3_State = UP_Right;
			}
			else if ((ballyval == movep1 + 2)  && (ballxval == 1))
			{
				PLAYER3_State = DOWN_Right;
			}
			else if ((ballyval == movep1 + 1)  && (ballxval == 1))
			{
				PLAYER3_State = Slide_right;
			}
			else if ((abs(ballyval - movep1) >= 3)  && (ballxval == 1))
			{
				PLAYER3_State = P2Score;
			}
			else {

				ballxval = ballxval - 1;
			}
			break;

			case DOWN_Right:
			if ((ballyval == movep2 + 2)  && (ballxval == 6))
			{
				PLAYER3_State = DOWN_Left;
			}
			else if ((ballyval == movep2)  && (ballxval == 6))
			{
				PLAYER3_State = UP_Left;
			}

			else if ((ballyval == movep2 + 1)  && (ballxval == 6))
			{
				PLAYER3_State = Slide_Left;
			}
			else if ((abs(ballyval - movep2) >= 2 )  && (ballxval == 6))
			{
				PLAYER3_State = P1Score;
			}
			else if ((ballyval == 8)  && (ballxval >=2) && (ballxval <= 7))
			{
				PLAYER3_State = UP_Right;
			}
			else
			{
				column_sel3 = ~column_sel3;
				column_sel3 = column_sel3 << 1 ;
				column_sel3 = ~column_sel3;
				ballxval = ballxval + 1;
				ballyval = ballyval + 1;
			}

			break;

			case DOWN_Left:
			if ((ballyval == movep1 + 2) && (ballxval == 1))
			{
				PLAYER3_State = DOWN_Right;
			}
			else if ((ballyval == movep1) && (ballxval == 1))
			{
				PLAYER3_State = UP_Right;
			}

			else if ((ballyval == movep1 + 1)  && (ballxval == 1))
			{
				PLAYER3_State = Slide_right;
			}
			else if ((abs(ballyval - movep1) >= 2 ) && (ballxval == 1))
			{
				PLAYER3_State = P2Score;
			}
			else if ((ballyval == 8)  && (ballxval >=2) && (ballxval <= 7))
			{
				PLAYER3_State = UP_Left;
			}
			else
			{
				column_sel3 = ~column_sel3;
				column_sel3 = column_sel3 << 1 ;
				column_sel3 = ~column_sel3;
				ballxval = ballxval - 1;
				ballyval = ballyval + 1;
			}
			break;

			case UP_Right:
			if ((ballyval == movep2 + 2)  && (ballxval == 6))
			{
				PLAYER3_State = DOWN_Left;
			}
			else if ((ballyval == movep2)  && (ballxval == 6))
			{
				PLAYER3_State = UP_Left;
			}

			else if ((ballyval == movep2 + 1)  && (ballxval == 6))
			{
				PLAYER3_State = Slide_Left;
			}
			else if ((abs(ballyval - movep2) >= 2 )  && (ballxval == 6))
			{
				PLAYER3_State = P1Score;
			}
			else if ((ballyval == 1)  && (ballxval >=2) && (ballxval <= 7))
			{
				PLAYER3_State = DOWN_Right;
			}
			else
			{
				column_sel3 = ~column_sel3;
				column_sel3 = column_sel3 >> 1 ;
				column_sel3 = ~column_sel3;
				ballxval = ballxval + 1;
				ballyval = ballyval - 1;
			}
			break;

			case UP_Left:
			if ((ballyval == movep1 + 2)  && (ballxval == 1))
			{
				PLAYER3_State = DOWN_Right;
			}
			else if ((ballyval == movep1)  && (ballxval == 1))
			{
				PLAYER3_State = UP_Right;
			}

			else if ((ballyval == movep1 + 1)  && (ballxval == 1))
			{
				PLAYER3_State = Slide_right;
			}
			else if ((abs(ballyval - movep1) >= 2 )  && (ballxval == 1))
			{
				PLAYER3_State= P2Score;
			}
			else if ((ballyval <= 1)  && (ballxval >= 2) && (ballxval <= 7))
			{
				PLAYER3_State = DOWN_Left;
			}
			else
			{
				column_sel3 = ~column_sel3;
				column_sel3 = column_sel3 >> 1 ;
				column_sel3 = ~column_sel3;
				ballxval = ballxval - 1;
				ballyval = ballyval - 1;
			}
			break;

			case P1Score:
			player1p = player1p + 1;
			ballxval = 4;
			ballyval = 4;
			column_sel3 = 0xF7;
			PLAYER3_State = Slide_Left;
			break;

			case P2Score:
			player2p = player2p + 1;
			ballxval = 4;
			ballyval = 4;
			column_sel3 = 0xF7;
			PLAYER3_State = Slide_right;

			break;
		}
	}
}

int joystickPos1() {
	int position = PINA0;
	unsigned short val1x;
	
	val1x = readadc(0);
	
	if(val1x >= 800) { position = 1; }
	
	else if(val1x <= 80) { position = 2; }
	
	else { position = 0; }
	
	return position;
}

int joystickPos2() {
	int position;
	unsigned short val2x;
	
	val2x = readadc(1);
	
	if(val2x >= 800) { position = 1; }
	
	else if(val2x <= 80) { position = 2; }
	
	else { position = 0; }
	
	return position;
}



void Player1paddle()
{
	//joystickPos1();
	
	if (movep1 == 1) { column_sel1 = 0xF8; }
	
	else if (movep1 == 2) { column_sel1 = 0xF1; }
	
	else if (movep1 == 3) { column_sel1 = 0xE3; }
	
	else if (movep1 == 4) { column_sel1 = 0xC7; }
	
	else if (movep1 == 5) { column_sel1 = 0x8F; }
	
	else { column_sel1 = 0x1F; }
}

void Player2paddle() {

	if (movep2 == 1) { column_sel2 = 0xF8; }
	
	else if (movep2 == 2) { column_sel2 = 0xF1; }
	
	else if (movep2 == 3) { column_sel2 = 0xE3; }
	
	else if (movep2 == 4) { column_sel2 = 0xC7; }
	
	else if (movep2 == 5) { column_sel2 = 0x8F; }
	
	else { column_sel2 = 0x1F; }
	
}

void printtoports()
{
	if (j == 0)
	{
		PORTD = arr[j];
		PORTC = column_sel1;
	}
	else if ((j >= 1) && (j < 7))
	{
		if ( j == ballxval)
		{
			PORTD = arr[j];
			PORTC  = column_sel3;
		}
		else {
			PORTD = arr[j];
			PORTC  = 0xFF;
		}
	}
	else if (j == 7)
	{	PORTD = arr[j];
		PORTC = column_sel2;
		j = 0;
		return;
	}
	j += 1;
	//joystickPos1();
}
unsigned char player1_scored;
unsigned char player2_scored;
unsigned char p1Score;
unsigned char p2Score;




void main()
{	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	adc_init();
	TimerSet(1);
	TimerOn();
	LCD_init();
	LCD_DisplayString(1, "Start Pong!");

	PLAYER1_State = Wait;
	PLAYER2_State = Wait2;
	PLAYER3_State = Slide_right;

	movep1 = 3;
	movep2 = 3;
	ballyval = 4;
	ballxval = 4;
	while(1) {
		
		p1Down = ~(PINA) & 0x01;
		p1Up = ~(PINA) & 0x02;
		p2Down = ~(PINA) & 0x04;
		p2Up = ~(PINA) & 0x08;
		TickFct_Player1(); 
		TickFct_Player2(); 
		Player1paddle();

		Player2paddle();
		printtoports();
		BallMovement();
		
		while (!TimerFlag);
		TimerFlag = 0;

	}
}

