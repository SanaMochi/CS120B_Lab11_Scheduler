/*	Author: sana
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #11  Exercise #4
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
const char* msg = "Congratulations!";
unsigned char j = 1;

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
      		case 'A': B = 0x11; break;
     		case 'B': B = 0x12; break;
      		case 'C': B = 0x13; break;
       		case 'D': B = 0x14; break;
      		case '*': B = 0x0E; break;
    		case '0': B = 0x00; break;
       		case '#': B = 0x0F; break;
		default: B = 0x1B; break;
	}
	return state;
}

enum display_States {init, wait, display_display, held};

int displaySMTick(int state) {
	unsigned char output;
	switch (state) {
		case init:
//			state = display_display;
			state = wait;
			break;
		case wait:
			if (B != 0x1F && B != 0x1B) state = display_display;
			else state = wait;
			break;
		case display_display: 
//			state = display_display;
			state = held;
			break;
		case held:
			if (B != 0x1F && B != 0x1B) state = held;
			else state = wait;
			break;
		default: 
			state = init; 
			break;
	}
	switch (state) {
		case init:
			LCD_DisplayString(1, msg);
			break;
		case wait: break;
		case display_display:
			if (j >= 17) j = 1;
			if (B != 0x0E && B!= 0x0F) { 
				LCD_Cursor(j++);
				output = B;
			}
			else if (B == 0x0E) {
				LCD_Cursor(j++);
				output = 0x2A;
			}
			else if (B == 0x0F) {
				LCD_Cursor(j++);
				output = 0x23;
			}
			if (B == 0x0E || B == 0x0F) {
				LCD_WriteData(output);
			}
			else {
				LCD_WriteData(output + '0');
			}
			break;
		case held: break;
	}
//	PORTB = output;
	return state;
}

int main(void) {
	unsigned char i;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;

	LCD_init();

	static task task1, task2;//, task3; //, task4;
	task *tasks[] = {&task1, &task2};//, &task3}; //, &task4};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = -1;

	task1.state = start;
	task1.period = 10;
        task1.elapsedTime = task1.period;
        task1.TickFct = &keypadButtonSMTick;
/*
	task2.state = start;
	task2.period = 10;
	task2.elapsedTime = task2.period;
	task2.TickFct = &replaceSMTick;
	
/*	task2.state = start;
	task2.period = 500;
	task2.elapsedTime= task2.period;
	task2.TickFct = &toggleLED0SMTick;
	
	task3.state = start;
        task3.period = 10;
        task3.elapsedTime= task3.period;
        task3.TickFct = &displaySMTick;
*/	
	task2.state = start;
	task2.period = 10;
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
