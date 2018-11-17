// filename ******** fixed.c ************** 
// Xinyuan (Allen) Pan && Paris Kaman
// Created Date: Sep. 6th
// Contains 4 functions needed to be implemented
// Lab 1
// TA: Saadallah
// Modified Date: Sep. 11th
// Hardware Configuration
// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected 
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO)
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
#include "ST7735.h"
#include "fixed.h"

static int32_t xMin, xMax, yMin, yMax; // the coordinate for drawing


/****************ST7735_sDecOut3***************
 converts fixed point number to LCD
 format signed 32-bit with resolution 0.01
 range -99.99 to +99.99
 Inputs:  signed 32-bit integer part of fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
 12345    " *.***"
  2345    " 2.345"  
 -8100    "-8.100"
  -102    "-0.102" 
    31    " 0.031" 
-12345    " *.***"
 */ 
void ST7735_sDecOut3(int32_t n) {
  if (n > 9999 | n < -9999) {  // out of range
     ST7735_OutString(" **.**");
	} else {
		if (n < 0) { // output - and negate if negative
			ST7735_OutChar('-');
			n = -n;
		} else {
			ST7735_OutChar(' ');
		}
		// extract each digit, +48 to get the ascii code of that digit
		ST7735_OutChar(n/1000 + 48);
		n = n - n/1000*1000;
		ST7735_OutChar(n/100 + 48);
		ST7735_OutChar('.');

		n = n - n/100*100;
		ST7735_OutChar(n/10 + 48);
	  n = n - n/10*10;
		ST7735_OutChar(n + 48);
	}
	
}

/**************ST7735_uBinOut8***************
 unsigned 32-bit binary fixed-point with a resolution of 1/256. 
 The full-scale range is from 0 to 999.99. 
 If the integer part is larger than 256000, it signifies an error. 
 The ST7735_uBinOut8 function takes an unsigned 32-bit integer part 
 of the binary fixed-point number and outputs the fixed-point value on the LCD
 Inputs:  unsigned 32-bit integer part of binary fixed-point number
 Outputs: none
 send exactly 6 characters to the LCD 
Parameter LCD display
     0	  "  0.00"
     2	  "  0.01"
    64	  "  0.25"
   100	  "  0.39"
   500	  "  1.95"
   512	  "  2.00"
  5000	  " 19.53"
 30000	  "117.19"
255997	  "999.99"
256000	  "***.**"
*/
void ST7735_uBinOut8(uint32_t n) {
	  if (n >= 256000) {  // out of range
     ST7735_OutString("***.**");
	} else {
		// since resolution is 256 = 2^8, the last 8 digits are fraction
		uint32_t intPart = n >> 8;
		uint32_t fracPart = n - (intPart << 8);
		fracPart = fracPart * 100 / 256; // times 100 because 2 decimal places are used
		
		// intPart three digits, fracPart 2 digits
		uint16_t leading0 = 1; // the boolean to track the leading 0's since we do not want to display them
		// hundreds
		if (intPart/100 == 0) {
			ST7735_OutChar(' ');
		} else {
			ST7735_OutChar(intPart/100+ 48);
			leading0 = 0;
		}
		intPart = intPart - intPart/100*100;
		// tens
			if (intPart/10 == 0 && leading0 == 1) {
			ST7735_OutChar(' ');
		} else {
			ST7735_OutChar(intPart/10+ 48);
			leading0 = 0;
		}
		intPart = intPart - intPart/10*10;
		// ones
		ST7735_OutChar(intPart + 48);
		// fraction part
		ST7735_OutChar('.');
		ST7735_OutChar(fracPart/10 + 48);
		fracPart = fracPart - fracPart/10*10;
		ST7735_OutChar(fracPart + 48);

	}
}

/**************ST7735_XYplotInit***************
 Specify the X and Y axes for an x-y scatter plot
 Draw the title and clear the plot area
 Inputs:  title  ASCII string to label the plot, null-termination
          minX   smallest X data value allowed, resolution= 0.001
          maxX   largest X data value allowed, resolution= 0.001
          minY   smallest Y data value allowed, resolution= 0.001
          maxY   largest Y data value allowed, resolution= 0.001
 Outputs: none
 assumes minX < maxX, and miny < maxY
*/
void ST7735_XYplotInit(char *title, int32_t minX, int32_t maxX, int32_t minY, int32_t maxY) {
  ST7735_PlotClear(minY, maxY); 
	ST7735_FillScreen(ST7735_BLACK);
	ST7735_SetCursor(0,0);
	ST7735_DrawString(0,0,title, ST7735_WHITE);
	xMin = minX;
	xMax = maxX;
	yMin = minY;
	yMax = maxY;
}

/**************ST7735_XYplot***************
 Plot an array of (x,y) data
 Inputs:  num    number of data points in the two arrays
          bufX   array of 32-bit fixed-point data, resolution= 0.001
          bufY   array of 32-bit fixed-point data, resolution= 0.001
 Outputs: none`
 assumes ST7735_XYplotInit has been previously called
 neglect any points outside the minX maxY minY maxY bounds
*/
void ST7735_XYplot(uint32_t num, int32_t bufX[], int32_t bufY[]) {
  for(int i=0;i<num;i++){
		if (bufX[i] > xMin && bufX[i] <xMax && bufY[i]>yMin && bufY[i]<yMax) { // if inside the boundary
			int a = (127*(bufX[i]-xMin)/(xMax-xMin)); // x-coordinate
			int b = (32 + (127*(yMax - bufY[i])/(yMax-yMin))); // y-coordinate
			ST7735_DrawPixel(a,b,ST7735_BLUE);
		}
  }
}
