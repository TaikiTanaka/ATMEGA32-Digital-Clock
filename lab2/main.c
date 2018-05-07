
#include <avr/io.h>
#include "avr.h"
#include "lcd.h"
#include <stdio.h>
struct timeStructure;
int getMonthDay(struct timeStructure * ts);
void updateTime(struct timeStructure * ts);

struct timeStructure{
	int sec,min,hr,month,day,year;
	int am;
	//Distinguishes between military and normal
	//0 for normal, 1 for military
	int timeType;
};

void displayTime(struct timeStructure *ts)
{
	char buf[17];
	pos_lcd(1,0);
	if(ts->timeType == 0)
	{
		if(ts->am == 1)
		{
			sprintf(buf, "%02i:%02i:%02i a.m.",ts->hr,ts->min,ts->sec);
		}
		else if (ts->am == 0)
		{
			sprintf(buf, "%02i:%02i:%02i p.m",ts->hr,ts->min,ts->sec);
		}
	}
	//means military time
	else
	{
		sprintf(buf, "%02i:%02i:%02i Mili",ts->hr,ts->min,ts->sec);
	}
	puts_lcd2(buf);
	pos_lcd(0,0);
	sprintf(buf, "%02i:%02i:%02i",ts->month,ts->day,ts->year);
	puts_lcd2(buf);
}


int is_pressed(int r, int c)
{
	//Setting Port C as N/C
	DDRC = 0;
	PORTC = 0;
	
	//Set r to strong 0
	SET_BIT(DDRC,r);
	CLR_BIT(PORTC,r);
	
	//Set col to weak 1
	CLR_BIT(DDRC,c+4);
	SET_BIT(PORTC,c+4);
	
	//Since pullup resister, if its high then return false
	wait_avr(1);
	if(GET_BIT(PINC,c+4))
	{
		return	0;
	}
	return 1;
}

int get_key()
{
	//r and c are the higher and lower bits on PORTC respectively
	//simply scans through every cell and checks if pressed
	int row,col;
	for (row=0;row<4;++row)
	{
		for (col=0;col<4;++col)
		{
			if(is_pressed(row,col))
			{
				wait_avr(300);
				return row*4 + col + 1;
			}
			//returns the index of the keypad
		}
	}
	return 0;
}

int translateKey(int k)
{
	//A -- enter programming mode
	if(k == 1)
	{return 1;}
		
	if(k == 2)
	{return 2;}
		
	if(k == 3)
	{return 3;}
		
	if(k == 4)
	{return 10;}
		
	if(k == 5)
	{return 4;}
		
	if(k==6)
	{return 5;}
		
	if(k==7)
	{return 6;}
	
	if(k==9)
	{return 7;}
		
	if(k==10)
	{return 8;}	
		
	if(k==11)
	{return 9;}
		
	if(k==14)
	{return 0;}

//If nothing is pressed
return 11;
}

void setTime(struct timeStructure * ts)
{
	char buf[17];
	pos_lcd(0,0);
	clr_lcd();
	sprintf(buf,"Set Time");
	puts_lcd2(buf);
	wait_avr(1000);
	clr_lcd();
	
	volatile int counter = 0;
	volatile int key;
	while(counter<15)
	{
		displayTime(ts);
		key = translateKey(get_key());
		if(key < 10)
		{
			switch(counter)
			{
				//month
				case 0:
					ts->month += key * 10;
					counter++;
					break;	
				case 1:
					ts->month += key;
					counter++;
					break;
				//day
				case 2:
					ts->day += key * 10;
					counter++;
					break;
				case 3:
					ts->day += key;
					counter++;
					break;
					
				//year
				case 4:
					ts->year += key * 1000;
					counter++;
					break;
				case 5:
					ts->year += key * 100;
					counter++;
					break;
				case 6:
					ts->year += key * 10;
					counter++;
					break;
				case 7:
					ts->year += key;
					counter++;
					break;
					
				//hour
				case 8:
					ts->hr += key*10;
					counter++;
					break;
				case 9:
					ts->hr += key;
					counter++;
					break;
					
				//minute
				case 10:
					ts->min += key*10;
					counter++;
					break;
				case 11:
					ts->min += key;
					counter++;
					break;
					
				//second
				case 12:
					ts->sec += key*10;
					counter++;
					break;
				case 13:
					ts->sec += key;
					counter++;
					break;
				//counter == 14 and ask for input
				default:
					if(key == 0)
					{
						ts->timeType = 0;
						ts->am = 1;
					}
					else if (key == 1)
					{
						ts->timeType = 0;
						ts->am = 0;
					}
					else
					{
						ts->timeType = 1;
					}
					counter++;
					break;
			}//end switch
		}//end if
		//displayTime(ts);
	}//end while
	clr_lcd();
	pos_lcd(0,0);
	sprintf(buf,"end input");
	puts_lcd2(buf);
	wait_avr(1000);
	clr_lcd();
}

void updateTime(struct timeStructure * ts)
{
	//Wait a second
	wait_avr(1000);
	ts->sec +=1;
	
	//First check the second
	if(ts->sec==60)
	{
		ts->sec = 0;
		ts->min+=1;
	}
	
	if(ts->min == 60)
	{
		ts->min = 0;
		ts->hr+=1;
		//If am/pm time
		if(ts->timeType == 0)
		{
			if(ts->hr == 12)
			{
				if(ts->am == 1)
				{
					ts->am = 0;
				}
				//If pm, then increase the day and make am true
				else
				{
					ts->am = 1;
					ts->day +=1;
				}
			}
			else if(ts->hr == 13)
			{
				ts->hr = 1;
			}
		}
		
		//military time
		else
		{
			if (ts->hr == 24)
			{
				ts->hr = 0;
				ts->day+=1;
			}
		}
	}
	
	int monthLength = getMonthDay(ts);
	//30 days
	if(monthLength == 30)
	{
		if(ts->day == 31)
		{
			ts->day = 1;
			ts->month+=1;
		}
	}
	
	//31 days
	else if (monthLength == 31)
	{
		if(ts->day == 32)
		{
			ts->day = 1;
			ts->month+=1;
		}
	}
	
	//28 days
	else if (monthLength == 28)
	{
		if(ts->day == 29)
		{
			ts->day = 1;
			ts->month+=1;
		}
	}
	
	if(ts->month == 13)
	{
		ts->month = 1;
		ts->year+=1;
	}
}

//Returns 1 for 30 days , 2 for 31 days, 3 for 28 days
int getMonthDay(struct timeStructure * ts)
{
	int month = ts->month;
	if(month == 2)
	{
		return 28;
	}
	// these months have 31 days -- 1,3,5,7,8,10,12
	else if(month == 1|| month == 3 || month == 5 || month ==7 || month == 8 || month ==10||month ==12)
	{
		return 31;
	}
	
	else
		return 30;
	
	//these months have 30 days -- 4,6,9,11
}

void testLED()
{
	int k;
	SET_BIT(DDRB,0);
	while (1)
	{
		k=get_key();
		for (int i=0;i<k;++i)
		{
			SET_BIT(PORTB,0);
			wait_avr(200);
			CLR_BIT(PORTB,0);
			wait_avr(200);
		}
	}
}
void clearTime(struct timeStructure * ts)
{
	ts->sec = 0;
	ts->min = 0;
	ts->hr = 0;
	ts->day = 0;
	ts->month = 0;
	ts->year = 0;
}
int main(void)
{
	//disabling jtag
	MCUCSR = (1<<JTD);
	MCUCSR = (1<<JTD);
	int initializedTime = 1;
	char buf[17];
	ini_lcd();
	ini_avr();
	int key;
	struct timeStructure ts;
	clearTime(&ts);
	while(1)
	{
		key = translateKey(get_key());
		//enter programming mode -- set the time and date
		if(key == 10)
		{
			clearTime(&ts);
			setTime(&ts);
			clr_lcd();
			initializedTime = 0;
		}
		//counter mode -- increment the timer
		else if(!initializedTime)
		{
			updateTime(&ts);
			displayTime(&ts);
		}
		else{
			pos_lcd(0,0);
			sprintf(buf,"Not initialized");
			puts_lcd2(buf);
		}
	}
}

