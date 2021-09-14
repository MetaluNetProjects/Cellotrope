/*********************************************************************
 *               analog example for Versa2.0
 *	Analog capture on connectors K1, K2, K3 and K5. 
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>
//#include <analog.h>
//#include <switch.h>
#include <dcmotor.h>
#include <dmx.h>

t_delay mainDelay;
DCMOTOR_DECLARE(A);
unsigned char lampSteps = 24;
unsigned char lampWindow = 26;
int lampVal = 1023;
int stepOffset = 0;
const unsigned int maxPos = 1000;
char updateDMX = 0;
long int intpos, intposold;
int nextStepPos;
unsigned char intstep, intstepold;
char inton;
char doPulse = 0;
unsigned char pulseMs = 6;
unsigned char stopNextStep = 0;
char forced;
// Timer macros
#define TIMER 5
#include <timer.h>

#define TIMER_INIT() do{\
	TIMER_CON = 0; \
	TIMER_PS0 = 1;				/* 	prescaler 8 (->2MHz at 64MHz)	*/\
	TIMER_PS1 = 1;				/* 	...								*/\
	TIMER_16BIT = 1;			/* 	16bits							*/\
	TIMER_IP = 1			;	/* 	high/low priority interrupt 	*/\
	TIMER_ON = 1;				/* 	stop timer						*/\
	TIMER_IF = 0;				/*  clear flag						*/\
	TIMER_IE = 1;				/* 	enable timer interrupt			*/\
} while(0)

#define	TimerCountUS(T) (0xffff - ((T * FOSC) / 32000000)) //us ; 
#define InitTimerUS(T) do{TIMER_H = TimerCountUS(T) / 256; TIMER_L = TimerCountUS(T) % 256; TIMER_IF = 0;} while(0)

void highInterrupts()
{
	if(TIMER_IF) {
		DCMOTOR_CAPTURE_SERVICE(A);

		if(lampSteps > 0) {
			intpos = DCMOTOR_GETPOS(A);
			if(inton) { /*analogWrite(LAMP1, 1023)*/ inton--; if(inton == 0) digitalSet(LAMP1);}
			if(intpos != intposold) {
				intposold = intpos;
				/*if(lampSteps > 0) {
					intstep = (intpos * lampSteps) / 1000;
					if(intstep != intstepold) {
						//analogWrite(LAMP1, 1023 - lampVal);
						digitalClear(LAMP1);
						//else analogWrite(LAMP1, 1023);
						intstepold = intstep;
						inton = lampWindow;
					}
				}*/
				if(intpos < 0) intpos += maxPos;
				if(intpos == nextStepPos) {
					inton = lampWindow;
					digitalClear(LAMP1);
					doPulse = 1;
				}
			}
		}
		InitTimerUS(100);
	}
}

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED); 	// set the LED pin mode to digital out
	digitalClear(LED);		// clear the LED
	delayStart(mainDelay, 5000); 	// init the mainDelay to 5 ms

/*
//----------- Analog setup ----------------
	analogInit();		// init analog module
	
	analogSelect(0,ANALOG1);	// assign connector K1 to analog channel 0

//----------- Switch setup ----------------
	switchInit();		// init switch module
	switchSelect(0,SW1);	// assign connector K1 to analog channel 0
*/

	//pinModeAnalogOut(LAMP1);
	pinModeDigitalOut(LAMP1);
	dcmotorInit(A);
	DCMOTOR(A).Setting.onlyPositive = 0;
	TIMER_INIT();
	InitTimerUS(100);
	
	DMXInit();        // init DMX master module
	DMXSet(1, 255);
}

long int position, lastPosition;
char step, oldstep;

void StrobeSet(char on)
{
	DMXSet(2, on != 0 ? 0 : 6);
	do {
		fraiseService();
	} while(!DMXService());
}

void StrobePulse(unsigned char ms)
{
	t_delay delay;
	StrobeSet(1);
	delayStart(delay, ms * 1000);
	while(!delayFinished(delay)) fraiseService();
	StrobeSet(0);
}

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
//	analogService();	// analog management routine
//	switchService();	// analog management routine
	if(updateDMX) DMXService();	// DXM management routine

	/*if(doPulse) {
		StrobePulse(pulseMs);
		doPulse = 0;
	}*/
	
	__critical { position = DCMOTOR_GETPOS(A);}
	if(position < 0) position += maxPos;
	
	if(position > maxPos) printf("C E %ld\n", position);
	if(lampSteps > 0) {
		step = (position * lampSteps) / maxPos;
		if(step != oldstep) {
			oldstep = step;
			if(DCMOTOR(A).VolVars.direction) nextStepPos = ((long)((step + 1) % lampSteps) * maxPos) / lampSteps;
			else nextStepPos = ((long)((step + lampSteps) % lampSteps) * maxPos) / lampSteps;
			if(!forced) StrobePulse(pulseMs);
			if(stopNextStep) DCMOTOR(A).Vars.PWMConsign = 0;
			printf("C N %d %ld\n", nextStepPos, position);
		}
	}
	
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 2000); 	// re-init mainDelay
		//analogSend();					// send analog channels that changed
		//switchSend();					// send switch channels that changed
		DCMOTOR_COMPUTE(A,SYM);
		if(position != lastPosition) {
			lastPosition = position;
			//printf("C P %ld\n", position);
		}
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
	else if(c=='E') { 	// echo text (send it back to host)
		printf("C");
		c = fraiseGetLen(); 			// get length of current packet
		while(c--) printf("%c",fraiseGetChar());// send each received byte
		putchar('\n');				// end of line
	}	
}

void fraiseReceive() // receive raw
{
	unsigned char c, v;
	unsigned int i;
	
	c = fraiseGetChar();
	switch(c) {
		case 10 : //direct lamp control 
			i = fraiseGetInt();
			analogWrite(LAMP1, i);
			break;
		case 11 : //direct lamp control 
			c = fraiseGetChar();
			digitalWrite(LAMP1, c != 0);
			break;
		case 20 : lampVal = fraiseGetInt(); break;
		case 21 : lampSteps = fraiseGetChar(); break;
		case 22 : lampWindow = fraiseGetChar(); break;
		case 23 : stepOffset = fraiseGetInt(); break;
		case 30 : i = fraiseGetInt(); v = fraiseGetChar(); DMXSet(i, v); break; // if first byte is 30 then get DMX channel (int) and value (char)
		case 31 : v = fraiseGetChar(); StrobeSet(v); forced = v;break;
		case 32 : v = fraiseGetChar(); StrobePulse(v); break;
		case 33 : pulseMs = fraiseGetChar(); break;
		case 34 : updateDMX = (fraiseGetChar() != 0); break;
		case 120 : DCMOTOR_INPUT(A) ; break;
		case 121 : stopNextStep = (fraiseGetChar() != 0); break;
	}
}


