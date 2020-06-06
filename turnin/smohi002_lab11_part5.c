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

unsigned char A;
unsigned char B;
const char* msg = " *       #                   #   ";
const char* end_msg = "YOU LOSE";
const char* str_output;
unsigned char j = 9;
unsigned char k = 28;
unsigned char l = 2;

enum buttonSM_States {wait_button, up, down, reset};

int buttonSMTick(int state) {
	switch (state) {
		case wait_button:
			if (A == 0x01) state = up;
			else if (A == 0x02) state = down;
			else if (A == 0x04) state = reset;
			else state = wait_button;
			break;
		case up:
			if (A) state = up;
			else state = wait_button;
			break;
		case down:
			if (A) state = down;
			else state = wait_button;
			break;
		case reset:
			if (A) state = reset;
			else state = wait_button;
			break;
		default:
			state = wait_button;
			break;
	}
	switch (state) {
		case wait_button: break;
		case up:
			l = 0x02;
			break;
		case down:
			l = 18;
			break;
		case reset:
			l = 0x02;
			break;
		default: break;
	}
	return state;
}

enum game_States {init, wait, wait2, cursor, end, end_on};

int gameSMTick(int state) {
	unsigned char output;
	switch (state) {
		case init:
			state = wait;
			break;
		case wait:
			if (A == 0x04)
				state = cursor;
			else state = wait;
			break;
		case wait2:
			if (j != l && k != l) state = cursor;
			else state = end;
			break;
		case cursor:
			state = wait2;
			break;
		case end:
			if (A == 0x04) 	{
				state = end_on;
				j = 9;
				k = 28;
			}
			else state = end;
			break;
		case end_on:
			if (A == 0x04) state = end_on;
			else state = init;
		default: 
			state = init; 
			break;
	}
	switch (state) {
		case init:
			str_output = msg;
			LCD_DisplayString(1, str_output);
			break;
		case wait: break;
		case wait2: break;
		case cursor:
			if (j == 1) j = 16;
//			else j--;
			if (k == 17) k = 32;
//			else k--;

			LCD_ClearScreen();
			LCD_Cursor(j--);
			output = 0x23;
			LCD_WriteData(output);
			
			LCD_Cursor(k--);
			output = 0x23;
			LCD_WriteData(output);
			
			LCD_Cursor(l);
			output = 0x2A;
			LCD_WriteData(output);
			break;
		case end: 
			str_output = end_msg;
			LCD_DisplayString(1, str_output);
			break;
		case end_on: break;
	}
//	PORTB = output;
	return state;
}
/*
enum display_States {Init, display_display, game_over, go};

int displaySMTick(int state) {
	unsigned char output;
	switch (state) {
		case Init:
			state = display_display;
			break;
		case display_display:
			if (j != l && k != l) state = display_display;
			else state = game_over;
			break;
		case game_over:
			if (A == 0x04) state = go;
			else state = game_over;
			break;
		case go:
                        if (A == 0x04) state = go;
                        else state = Init;
		default:
			state = Init;
			break;
	}
	switch (state) {
		case Init:
//			str_output = msg;
                        LCD_DisplayString(1, str_output);
			break;
		case display_display:
			LCD_ClearScreen();
                        LCD_Cursor(j);
                        output = 0x23;
                        LCD_WriteData(output);

                        LCD_Cursor(k);
                        output = 0x23;
                        LCD_WriteData(output);

                        LCD_Cursor(l);
                        output = 0x2A;
                        LCD_WriteData(output);
			break;
		case game_over:
			LCD_DisplayString(1, str_output);
			break;
		case go: break;
		default: break;
	}
	return state;
}
*/
int main(void) {
	unsigned char i;
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	LCD_init();

	static task task1, task2;//, task3;
	task *tasks[] = {&task1, &task2};//, &task3};
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = -1;

	task1.state = start;
	task1.period = 10;
        task1.elapsedTime = task1.period;
        task1.TickFct = &buttonSMTick;
	
	task2.state = start;
	task2.period = 500;
	task2.elapsedTime = task2.period;
	task2.TickFct = &gameSMTick;
/*
	task3.state = start;
        task3.period = 10;
        task3.elapsedTime = task3.period;
        task3.TickFct = &displaySMTick;
*/
	unsigned long GCD = tasks[0]->period;
	unsigned char j;
	for (j = 1; j < numTasks; j++) {
		GCD = findGCD(GCD, tasks[j]->period);
	}

	TimerSet(GCD);
	TimerOn();

	while (1) {
		A = ~PINA & 0x07;
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
