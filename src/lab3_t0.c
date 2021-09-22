#include "fsl_common.h"
#include "3140_accel.h"
#include "3140_serial.h"
#include "3140_i2c.h"
#include <stdio.h>
#include "3140_concur.h"
#include "utils.h"
#include "shared_structs.h"

static char string[100];
int b2 = 0; //whether button 2 is pressed
int b3 = 0; //whether button 3 is pressed
int count= 0;// total number of processes created
int c2 = 0; //counter for twist
int go = 0; //game over check
int twarr[15]; //array for twist
int frt = 10000000; //fastest reaction time
int lastT = 0; //stores arrival time to check reaction time
int totalcount = 0; //keeps count over games to check avg reaction time
int totaltime = 0; //keeps time over games to check avg reaction time

int twist(void) {
	/* detects when the board has been twisted using an
	average of the previous 15 acceldat values */
	int t = 0;
	for (int i=0; i<15; i++) {
			t += twarr[i];
		}	
	t /= 15;
	if (t < 800) {
		return 1;
	} else return 0;
}

void p1 (void) {
	/* p1 is the process for twist, which is a blue light*/
	LEDBlue_On();
	SRAWDATA accelDat;
	ACCEL_getAccelDat(&accelDat);
	//this loop continues while the process is running
	//until time runs out, a button is pressed or twist has happened
	while(!process_deadline_miss && !twist() && !b2 && !b3){
		ACCEL_getAccelDat(&accelDat);
		twarr[c2 % 15] = abs(accelDat.z);
		c2++;
	}
	//if twist has happened, gets player data and flashes green light, which means correct
	if(twist()) {
		int trt = getabstime(&current_time) - lastT;
		if (trt < frt) {
			frt = trt;
		}
		totaltime += trt;
		totalcount++;
		current_process->deadline->sec += 100;
		LEDBlue_Toggle();
		LEDGreen_Toggle();
		delay();
		delay();
		LEDGreen_Toggle();
		count++;
	} else if(process_deadline_miss || b2 || b3){
		//if you did the wrong thing or ran out of time, enters failstate
		LEDBlue_Toggle();
		LEDRed_On();
		uart_putString("You lost on twist! \n\r");
		go = 1;
	} else {
		LEDCyan_On(); //should never happen; purely for debugging
		while(1);
	}
}

void p2 (void) {
	/* p2 is the process for button 2, which is a yellow light*/
	LEDYellow_On();
	SRAWDATA accelDat;
	ACCEL_getAccelDat(&accelDat);
	//this loop continues while the process is running
	//until time runs out, a button is pressed or twist has happened
	while(!process_deadline_miss && !b2 && !b3 && !twist()){
		ACCEL_getAccelDat(&accelDat);
		twarr[c2 % 15] = abs(accelDat.z);
		c2++;
	}
	if (process_deadline_miss || b3 || twist()){
		//if you did the wrong thing or ran out of time, enters failstate
		LEDRed_On();
		uart_putString("You lost on press button 2! \n\r");
		go = 1;
	} else if (b2){
		//if button 2 has been pressed, gets player data and flashes green light, which means correct
		int trt = getabstime(&current_time) - lastT;
		if (trt < frt) {
			frt = trt;
		}
		totaltime += trt;
		totalcount++;
		current_process->deadline->sec += 100;
		LEDYellow_Toggle();
		LEDGreen_Toggle();
		delay();
		delay();
		LEDGreen_Toggle();
		count++;
	} else {
		LEDCyan_On(); //should never happen; purely for debugging
		while(1);
	}
}

void p3 (void) {
	/* p3 is the process for button 3, which is a purple light*/
	LEDPurple_On();
	SRAWDATA accelDat;
	ACCEL_getAccelDat(&accelDat);
	//this loop continues while the process is running
	//until time runs out, a button is pressed or twist has happened
	while(!process_deadline_miss && !b3 && ! b2 && !twist()){
		ACCEL_getAccelDat(&accelDat);
		twarr[c2 % 15] = abs(accelDat.z);
		c2++;
	}
	if (process_deadline_miss || b2 || twist()){
		//if you did the wrong thing or ran out of time, enters failstate
		LEDRed_On();
		uart_putString("You lost on press button 3! \n\r");
		go = 1;
	} else if (b3){
		//if button 3 has been pressed, gets player data and flashes green light, which means correct
		int trt = getabstime(&current_time) - lastT;
		if (trt < frt) {
			frt = trt;
		}
		totaltime += trt;
		totalcount++;
		current_process->deadline->sec += 100;
		LEDPurple_Toggle();
		b3 = 0;
		LEDGreen_Toggle();
		delay();
		delay();
		LEDGreen_Toggle();
		count++;
	} else {
		LEDCyan_On(); //should never happen; purly for debugging
		while(1);
	}
}

void init (void) {
	/* initial state, waits for button press */
	LEDWhite_Toggle();
	while(!b2&!b3){
		delay();
	}
	b2=0;
	b3 = 0;
	LEDWhite_Toggle();
}

void PORTA_IRQHandler(void) {
	/* IRQ handler for button 3 */
	b3 = 1;
	PORTA->PCR[4] |= PORT_PCR_IRQC(1001);
	PORTA->PCR[4] &= ~PORT_PCR_MUX(110); //this works
}

void PORTC_IRQHandler(void) {
	/* IRQ handler for button 2 */
	b2 = 1;
	PORTC->PCR[6] |= PORT_PCR_IRQC(1001);
}

realtime_t gettime() {
	/* calculates next deadline time */
	realtime_t t = {3, 00}; //sets the deadline; would do it as a global but that breaks things
	int tempt = getabstime(&t);
	int c3 = 0;
	while(tempt > 600 && c3 < count) { //minimum time of 600ms
		c3++;
		tempt *= 0.95; //decrements next deadline time by 5% each time to make game challenging
	}
	t.sec = tempt / 1000;
	t.msec = tempt % 1000;
	sprintf(string, "Time to complete next task: %ds, %dms \n\r",t.sec, t.msec);
	uart_putString(string);
	return t;
}

int random_process (void) {
	/* picks next process by using current time as a seed time */
	realtime_t t = gettime();
	int next_process = (current_time.msec) % 3; //needs to be an int between 0 and 3 to select a process
	int r = 0;
	lastT = getabstime(&current_time);
	if(next_process == 0) {
		r = process_rt_create(p1,100, &current_time, &t); //needs to have arrival time be current time
	}
	else if(next_process == 1) {
		r = process_rt_create(p2,100, &current_time, &t);
	}
	else if(next_process ==2) {
		r = process_rt_create(p3,100, &current_time, &t);
	}
	else LEDCyan_On(); //should never happen; purly for debugging
	return r;
}

void reseta (void) {
	/* resets values to initial values between tasks*/
	b2 = 0; 
	b3 = 0;
	c2 = 0;
	for (int i=0; i<15; i++) {
		twarr[i] = 2000;
	}
}

int main (void){	
	//most of this is from accelerometer tutorial
	int clock;

	clock=SystemCoreClock;

	LED_Initialize();
	Butt_Init();

	I2C_ReleaseBus(); // Force Restart the bus, this is done via GPIO commands
									// and not via the onboard i2c module

	I2C_Init();				// Initialize I2C bus and Uart Serial Communications
	uart_init();

	uint8_t a = ACCEL_ReadWhoAmI();
	uint8_t b = ACCEL_getDefaultConfig();

	char string[100];
	
	uart_putString("\n\rHi! Welcome to MicroBop!\n\r");
	uart_putString("When the game starts, you will be given a task via LED color\n\r");
	uart_putString("If you see blue, twist the board\n\r");
	uart_putString("If you see yellow, press button 2\n\r");
	uart_putString("If you see purple, press button 3\n\r");
	uart_putString("When you correctly complete a task, a green light will blink\n\r");
	uart_putString("You will begin with 3 seconds to complete the first task\n\r");
	uart_putString("Each task, you will lose 5% of your allotted time until you have only 600 ms.\n\r");
	uart_putString("If you do the wrong thing or take too long, you lose!\n\r");
	uart_putString("When you lose, red blinking lights will indicate your score.\n\r");
	//red lights let you know the score and play the game without PuTTY
	uart_putString("Press any button to start game, good luck!\n\r");
	
	//creates initial process
	if(process_create(init,100) < 0){
		return -1;
	}
	process_start();
	
	//while player hasn't lost, keep creating new process
	while(!go){
		reseta();
		if (random_process() < 0) {
			return -1;
		} else{
			process_begin();
		}
	}
	
	//fail state below
	sprintf(string, "Your score is: %d!\n\r", count);
	uart_putString(string);
	
	//if player got score of 0, no reason to tell them their times
	if (!count) {
		uart_putString("Try again! This time, read the directions in the README, or via PuTTY!");
	} else {
		sprintf(string, "Your fastest reaction time was: %dms!\n\r", frt);
		uart_putString(string);
		int temprt = totaltime / totalcount;
		sprintf(string, "Your average reaction time was: %dms!*\n\r", temprt);
		//avg reaction time doesn't include failed task bc that would skew reaction 
		//time, making it a useless statistic
		uart_putString(string);
		sprintf(string, "Your total time was: %ds, %dms!\n\r", totaltime / 1000, totaltime % 1000);
		uart_putString(string);
	}
	//blinks out red light for score
	while(count>1) {
			shortdelay();
			LEDRed_Toggle();
			shortdelay();
			LEDRed_Toggle();
			count--;
	}
	
	while (1) ;

	return 0;
}
