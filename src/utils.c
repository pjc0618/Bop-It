#include <MK64F12.h>
#include "utils.h"

/*----------------------------------------------------------------------------
  Function that initializes Buttons
 *----------------------------------------------------------------------------*/
void Butt_Init(void) {
	SIM->SCGC5   |= (1 << 9) | (1 <<  11);  /* Enable Clock to Port A & C */
	NVIC_EnableIRQ(PORTA_IRQn); //Enables interrupts on port A
	NVIC_EnableIRQ(PORTC_IRQn); //Enables interrupts on port C
	
	PORTA->PCR[4] = PORT_PCR_MUX(001); //Set up PTA4 as GPIO
	//PORTA->PCR[4] &= ~PORT_PCR_MUX(110); //this works
	PORTC->PCR[6] |= PORT_PCR_MUX(001); //Set up PTC6 as GPIO
	
	PTA->PDDR &= ~(1 << 4); //Enables port A as Input
	PTC->PDDR &= ~(1 << 6); //Enables port C as Input
	
	PORTA->PCR[4] |= PORT_PCR_IRQC(1001);
	PORTC->PCR[6] |= PORT_PCR_IRQC(1001);
}

/*----------------------------------------------------------------------------
  Function that initializes LEDs
 *----------------------------------------------------------------------------*/
void LED_Initialize(void) {

  SIM->SCGC5    |= (1 << 10) | (1 <<  13);  /* Enable Clock to Port B & E */ 
  PORTB->PCR[22] = (1 << 8);               /* Pin PTB22 is GPIO */
  PORTB->PCR[21] = (1 << 8);                /* Pin PTB21 is GPIO */
  PORTE->PCR[26] = (1 << 8);                /* Pin PTE26  is GPIO */
  
  PTB->PDOR = (1 << 21 | 1 << 22 );          /* switch Red/Green LED off  */
  PTB->PDDR = (1 << 21 | 1 << 22 );          /* enable PTB18/19 as Output */

  PTE->PDOR = 1 << 26;            /* switch Blue LED off  */
  PTE->PDDR = 1 << 26;            /* enable PTE26 as Output */
}

/*----------------------------------------------------------------------------
  Function that toggles the red LED
 *----------------------------------------------------------------------------*/

void LEDRed_Toggle (void) {
	PTB->PTOR = 1 << 22; 	   /* Red LED Toggle */
}

/*----------------------------------------------------------------------------
  Function that toggles the blue LED
 *----------------------------------------------------------------------------*/
void LEDBlue_Toggle (void) {
	PTB->PTOR = 1 << 21; 	   /* Blue LED Toggle */
}

/*----------------------------------------------------------------------------
  Function that toggles the green LED
 *----------------------------------------------------------------------------*/
void LEDGreen_Toggle (void) {
	PTE->PTOR = 1 << 26; 	   /* Green LED Toggle */
}

/*----------------------------------------------------------------------------
  Function that turns on Red LED & all the others off
 *----------------------------------------------------------------------------*/
void LEDRed_On (void) {
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTB->PCOR   = 1 << 22;   /* Red LED On*/
  PTB->PSOR   = 1 << 21;   /* Blue LED Off*/
  PTE->PSOR   = 1 << 26;   /* Green LED Off*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that turns on Green LED & all the others off
 *----------------------------------------------------------------------------*/
void LEDGreen_On (void) {
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTB->PSOR   = 1 << 21;   /* Blue LED Off*/
  PTE->PCOR   = 1 << 26;   /* Green LED On*/
  PTB->PSOR   = 1 << 22;   /* Red LED Off*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that turns on Blue LED & all the others off
 *----------------------------------------------------------------------------*/
void LEDBlue_On (void) {
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTE->PSOR   = 1 << 26;   /* Green LED Off*/
  PTB->PSOR   = 1 << 22;   /* Red LED Off*/
  PTB->PCOR   = 1 << 21;   /* Blue LED On*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that turns on Blue & Red LED for purple & turns off Green
 *----------------------------------------------------------------------------*/
void LEDPurple_On (void) {
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTB->PCOR   = 1 << 22;   /* Red LED On*/
  PTE->PSOR   = 1 << 26;   /* Green LED Off*/
  PTB->PCOR   = 1 << 21;   /* Blue LED On*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that turns on Blue & Green LED for cyan & turns off red
 *----------------------------------------------------------------------------*/
void LEDCyan_On (void) {
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTB->PSOR   = 1 << 22;   /* Red LED Off*/
  PTE->PCOR   = 1 << 26;   /* Green LED On*/
   PTB->PCOR   = 1 << 21;   /* Blue LED On*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that turns on Red & Green LED for yellow & turns off blue
 *----------------------------------------------------------------------------*/
void LEDYellow_On (void) {
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTB->PCOR   = 1 << 22;   /* Red LED On*/
  PTE->PCOR   = 1 << 26;   /* Green LED On*/
  PTB->PSOR   = 1 << 21;   /* Blue LED Off*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that turns on all LEDs for white
 *----------------------------------------------------------------------------*/
void LEDWhite_On (void) {
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTB->PCOR   = 1 << 22;   /* Red LED On*/
  PTE->PCOR   = 1 << 26;   /* Green LED On*/
  PTB->PCOR   = 1 << 21;   /* Blue LED On*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that turns all LEDs off
 *----------------------------------------------------------------------------*/
void LED_Off (void) {	
	// Save and disable interrupts (for atomic LED change)
	uint32_t m;
	m = __get_PRIMASK();
	__disable_irq();
	
  PTB->PSOR   = 1 << 22;   /* Green LED Off*/
  PTB->PSOR   = 1 << 21;   /* Red LED Off*/
  PTE->PSOR   = 1 << 26;   /* Blue LED Off*/
	
	// Restore interrupts
	__set_PRIMASK(m);
}

/*----------------------------------------------------------------------------
  Function that toggles the red and green LEDs
 *----------------------------------------------------------------------------*/
void LEDYellow_Toggle (void){
	LEDRed_Toggle();
	LEDGreen_Toggle();
}

/*----------------------------------------------------------------------------
  Function that toggles the red and blue LEDs
 *----------------------------------------------------------------------------*/
void LEDPurple_Toggle (void){
	LEDRed_Toggle();
	LEDBlue_Toggle();
}

/*----------------------------------------------------------------------------
  Function that toggles the green and blue LEDs
 *----------------------------------------------------------------------------*/
void LEDCyan_Toggle (void){
	LEDGreen_Toggle();
	LEDBlue_Toggle();
}

/*----------------------------------------------------------------------------
  Function that toggles the red, green, and blue LEDs
 *----------------------------------------------------------------------------*/
void LEDWhite_Toggle (void){
	LEDRed_Toggle();
	LEDBlue_Toggle();
	LEDGreen_Toggle();
}

/*----------------------------------------------------------------------------
  Function that cycles 1000000 times to create a delay
 *----------------------------------------------------------------------------*/
void delay(void){
	int j;
	for(j=0; j<1000000; j++);
}

/*----------------------------------------------------------------------------
  Function that cycles 500000 times to create a delay
 *----------------------------------------------------------------------------*/
void shortdelay(void){
	int j;
	for(j=0; j<500000; j++);
}
