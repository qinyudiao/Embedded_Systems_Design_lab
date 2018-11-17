// ADCTestmain.c
// Runs on LM4F120/TM4C123
// Provide a function that initializes Timer0A to trigger ADC
// SS3 conversions and request an interrupt when the conversion
// is complete.

/*
List three ways you could use to initiate the ADC conversion process.
1. Triggered by the processor
2. Analog comparator
3. Continuous sampling
What is the way to know when the conversion process has been completed?
A: Once the process is completed, a bit in ADCRIS (ADC Raw Interrupt Status) will be set.
 */



// center of X-ohm potentiometer connected to PE3/AIN0
// bottom of X-ohm potentiometer connected to ground
// top of X-ohm potentiometer connected to +3.3V through X/10-ohm ohm resistor
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "ADCT0ATrigger.h"
#include "PLL.h"
#include "UART.h"
#include "ST7735.h"
#include "calib.h"
#include "fixed.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

extern volatile uint32_t ADCvalue;

uint16_t ADC_to_temp(uint32_t ADCvalue);
void print_screen(uint16_t temp, uint32_t ADCvalue);
void draw_data(uint16_t temp);


//debug code
//
// This program periodically samples ADC0 channel 0 and stores the
// result to a global variable that can be accessed with the JTAG
// debugger and viewed with the variable watch feature.


int main(void){
  PLL_Init(Bus80MHz);                      // 80 MHz system clock
  SYSCTL_RCGCGPIO_R |= 0x00000020;         // activate port F
	  UART_Init();              // initialize UART device
	  ST7735_InitR(INITR_REDTAB);
  ADC0_InitTimer0ATriggerSeq3(9, 2000000); // ADC channel 0, 40 Hz sampling
  GPIO_PORTF_DIR_R |= 0x04;                // make PF2 out (built-in LED)
  GPIO_PORTF_AFSEL_R &= ~0x04;             // disable alt funct on PF2
  GPIO_PORTF_DEN_R |= 0x04;                // enable digital I/O on PF2
                                           // configure PF2 as GPIO
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF0FF)+0x00000000;
  GPIO_PORTF_AMSEL_R = 0;                  // disable analog functionality on PF
  GPIO_PORTF_DATA_R &= ~0x04;              // turn off LED
	//ST7735_DrawString(0,3,"40 C", ST7735_YELLOW);
	//ST7735_DrawString(0,15,"10 C", ST7735_YELLOW);
	ADC0_SAC_R = ADC_SAC_AVG_64X;	
	EnableInterrupts();
  while(1){
    WaitForInterrupt();
    GPIO_PORTF_DATA_R ^= 0x04;             // toggle LED
		UART_OutUDec(ADCvalue);
		UART_OutString("\t");
	  uint16_t temp = ADC_to_temp(ADCvalue);
		print_screen(temp, ADCvalue);
		draw_data(temp);
		UART_OutString("\n\r");
  }
}

int plotX = 0;
void draw_data(uint16_t temp) {
		int plotY = 149 - ((temp - 1000) *100 / 3000);
		ST7735_DrawPixel(plotX, plotY,ST7735_BLUE);
		plotX++;
	  if (plotX == 128) {
			plotX = 0;
			ST7735_FillRect(0,50,128,100, ST7735_BLACK);
		}
}


uint16_t ADC_to_temp(uint32_t ADCvalue) {
	// if out of range, return the maximum or minimun number
	if (ADCvalue > ADCdata[TABLE_SIZE-2]) return Temperature[TABLE_SIZE-1];
	if (ADCvalue < ADCdata[1]) return Temperature[0];
	for (int i=1;i<TABLE_SIZE;i++) {
		if (ADCvalue >= ADCdata[i] && ADCvalue <= ADCdata[i+1]) {
			// Only one value need to be explicitly upconverted
			//return ADCvalue;
			//return Temperature[i];
			uint16_t temperature = (int)(ADCvalue - ADCdata[i]) * (Temperature[i+1] - Temperature[i]) / (ADCdata[i+1] - ADCdata[i]); 
			//uint16_t temperature = (int)(Temperature[i]-3100)*3 + 2200;
			//		  	UART_OutUDec(ADCvalue);
//				UART_OutString("\t");
//				UART_OutUDec(temp);
//				UART_OutString("\n\r");
			if(Temperature[i]>3000){
				if(Temperature[i] < 3640){
					temperature += (Temperature[i]-3100)*1.8 + 2100 ;
					if(temperature >4000)
						return 4000;
					return temperature;
				}
				if(Temperature[i] >= 3640){
					temperature += (Temperature[i]-3100)*1.6 + 2208 ;
					if(temperature >4000)
						return 4000;
					return temperature;
				}
			}
			return temperature + Temperature[i];
		}
	}
	return 0;  // never reached
}

void print_screen(uint16_t temp, uint32_t ADCvalue) {
				ST7735_SetCursor(0,0);
				ST7735_OutString("Temperature =");
				ST7735_sDecOut3(temp);
				ST7735_OutString("C");
						ST7735_SetCursor(0,1);
				ST7735_OutString("ADC value = ");
				ST7735_OutUDec(ADCvalue);

}

