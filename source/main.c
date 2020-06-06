/*	Author: sana
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #11  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <string.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "../header/timer.h"
#include "../header/bit.h"
#include "../header/scheduler.h"
#include "../header/keypad.h"
//#include "../header/lcd_8bit_task.h"
#include "../header/io.h"
#endif

unsigned char x;
unsigned char B;
const char* msg = "                CS120B is legend... wait for it DARY!";
unsigned char str_output[16];
unsigned char max = 15;
unsigned char j = 0;
unsigned char k = 0;
//unsigned char l* = &str_output + 16;
enum keypadButtonSM_States {start};

int keypadButtonSMTick(int state) {
	x = GetKeypadKey();
	switch (x) {
		case'\0': B = 0x1F; break;
		case '1': B = 0x01; break;
       		case '2': B = 0x02; break;
       		case '3': B = 0x03; break;
       		case '4': B = 0x04; break;
     		case '5': B = 0x05; break;
		case '6': B = 0x06; break;
		case '7': B = 0x07; break;
      		case '8': B = 0x08; break;
     		case '9': B = 0x09; break;
      		case 'A': B = 0x0A; break;
     		case 'B': B = 0x0B; break;
      		case 'C': B = 0x0C; break;
       		case 'D': B = 0x0D; break;
      		case '*': B = 0x0E; break;
    		case '0': B = 0x00; break;
       		case '#': B = 0x0F; break;
		default: B = 0x01B; break;
	}
	return state;
}

enum paginate_States {beginning, middle, end};

int paginateSMTick(int state) {
	switch (state) {
		case beginning:
			state = middle;
			break;
		case middle:
			if (k < 38) state = middle;
			else {
				state = end;
				j--; k++;
			}
			break;
		case end:
			if (k < 54 && j > 0) state = end;
			else {
				state = beginning;
				memset(str_output, 0, 16);
				j = 0;
				k = 0;
			}
			break;
		default: 	
			state = beginning;
			break;
	}
	switch (state) {
		case beginning: 
			strncat(str_output, msg, 16);
			break;
		case middle:
			memset(str_output, 0, 16);
			strncpy(str_output, msg + (k++), 16);
			break;
		case end:
			strncat(str_output, 0, 1);
			strncpy(str_output, msg + (k++), j--);
			break;
		default: break;
	}
	return state;
}

enum display_States {D_SMStart, display_display };

int displaySMTick(int state) {
//	unsigned char output;
	switch (state) {
		case D_SMStart:
			state = display_display;
			break;
		case display_display: state = display_display; break;
		default: state = display_display; break;
	}
	switch (state) {
		case display_display:
			break;
	}
//	PORTB = output;
	LCD_ClearScreen();
	LCD_DisplayString(1, str_output);
	return state;
}

int main(void) {
	unsigned char i;
//	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;

	LCD_init();

	static task task1, task2; //task3, task4;
	task *tasks[] = {&task1, &task2 };// &task3, &task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = -1;

/*	task1.state = start;
        task1.period = 10;
        task1.elapsedTime = task1.period;
        task1.TickFct = &keypadButtonSMTick;/
*/
	task1.state = start;
	task1.period = 500;
	task1.elapsedTime = task1.period;
	task1.TickFct = &paginateSMTick;
	
/*	task2.state = start;
	task2.period = 500;
	task2.elapsedTime= task2.period;
	task2.TickFct = &toggleLED0SMTick;
	
	task3.state = start;
        task3.period = 1000;
        task3.elapsedTime= task3.period;
        task3.TickFct = &toggleLED1SMTick;
*/	
	task2.state = start;
	task2.period = 500;
	task2.elapsedTime = task2.period;
	task2.TickFct = &displaySMTick;

	unsigned long GCD = tasks[0]->period;
	unsigned char j;
	for (j = 1; j < numTasks; j++) {
		GCD = findGCD(GCD, tasks[j]->period);
	}

	TimerSet(GCD);
	TimerOn();

	while (1) {
		for (i = 0; i < numTasks; i++) {
			if (tasks[i]->elapsedTime == tasks[i]->period) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += GCD;
		}
		while(!TimerFlag);
		TimerFlag = 0;
	}
    return 0;
}
