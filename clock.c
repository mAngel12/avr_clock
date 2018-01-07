#include <avr/io.h>
#include <util/delay.h>
#include <string.h>

#define LCD_DDR DDRA
#define LCD_PORT PORTA
#define LCD_RS 0
#define LCD_EN 1
#define LCD_DB4 4
#define LCD_DB5 5
#define LCD_DB6 6
#define LCD_DB7 7

volatile int status = 0;

volatile int hours = 0;
volatile int minutes = 0;
volatile int seconds = 0;
volatile char hoursChar[] = "HH";
volatile char minutesChar[] = "MM";
volatile char secondsChar[] = "SS";
volatile char text[] = "HH:MM:SS";

volatile int stopwatch = 0;
volatile int stopwatch_minutes = 0;
volatile int stopwatch_seconds = 0;
volatile char stopwatch_minutesChar[] = "MM";
volatile char stopwatch_secondsChar[] = "SS";

volatile int world_time_status = 0;

volatile int timer = 0;
volatile int timer_minutes = 0;
volatile int timer_seconds = 0;
volatile char timer_minutesChar[] = "MM";
volatile char timer_secondsChar[] = "SS";

void LCD_Send(char byte)
{
	LCD_PORT |= _BV(LCD_EN);
	LCD_PORT = (byte & 0xF0) | (LCD_PORT & 0x0F);
	LCD_PORT &= ~(_BV(LCD_EN));
	asm volatile("NOP");
	LCD_PORT |= _BV(LCD_EN);
	LCD_PORT = ((byte & 0x0F) << 4) | (LCD_PORT & 0x0F);
	LCD_PORT &= ~(_BV(LCD_EN));
	_delay_us(40);
}

void LCD_Clear()
{
	LCD_PORT &= ~(_BV(LCD_RS));
	LCD_Send(1);
	LCD_PORT |= _BV(LCD_RS);
	_delay_ms(1.64);
}

void LCD_Enable()
{
	LCD_DDR = (0xF0) | (_BV(LCD_RS)) | (_BV(LCD_EN));
	LCD_PORT = 0;

	LCD_PORT &= ~(_BV(LCD_RS));
	LCD_Send(0b00101000);
	LCD_PORT |= _BV(LCD_RS);

	LCD_PORT &= ~(_BV(LCD_RS));
	LCD_Send(0b00000110);
	LCD_PORT |= _BV(LCD_RS);

	LCD_PORT &= ~(_BV(LCD_RS));
	LCD_Send(0b00001100);
	LCD_PORT |= _BV(LCD_RS);

	LCD_Clear();
}

void LCD_Text(char *text)
{
	unsigned char i = 0, t = 0;

	while(text[i])
	{
		LCD_Send(text[i]);

		if(i > 14 && t == 0)
		{
			LCD_PORT &= ~(_BV(LCD_RS));
			LCD_Send(0b10101000);
			LCD_PORT |= _BV(LCD_RS);
			t = 1;
		}
	i++;
    }
}

void delay(int ms)
{
	for(int i = 0; i <ms; i++)
	{
		TCNT0 = 255;
		TCCR0 |= (1 << CS00) | (1 << CS02);
 
		while(!(TIFR & _BV(TOV0)))
		{ }

		TCCR0 &= ~(1 << CS00) | ~(1 << CS02);
		TIFR |= (1 << TOV0);
	}
}

void detectButtonClick()
{
	PORTC = 0xEF;
	asm("nop");

	if(bit_is_clear(PINC,0))
	{
		if(status == 0)
		{
			status = 1;
		}
		else if(status == 1)
		{
			status = 0;
		}
		else if(status == 2)
		{
			stopwatch = 1;
		}
		else if(status == 3)
		{
			if(world_time_status == 0)
			{
				status = 0;
			}
		}
		else if(status == 4)
		{
			if(timer == 0)
			{
				timer = 1;
			}
			else if(timer == 1)
			{
				timer = 0;
			}
		}
	}
	if(bit_is_clear(PINC,1))
	{
		if(status == 0)
		{
			status = 0;
			hours = hours + 1;
			if (hours >= 24)
			{
				hours = 0;
			}
		}
		else if(status == 1)
		{
			status = 2;
		}
		else if(status == 2)
		{
			stopwatch = 0;
		}
		else if(status == 3)
		{
			if(world_time_status == 0)
			{
				world_time_status = 1;
			}
		}
		else if(status == 4)
		{
			if(timer == 0)
			{
				timer_minutes = timer_minutes + 1;
				if(timer_minutes >= 60)
				{
					timer_minutes == 0;
				}
			}
		}
	}
	if(bit_is_clear(PINC,2))
	{
		if(status == 0)
		{
			status = 0;
			minutes = minutes + 1;
			if (minutes >= 60)
			{
				minutes = 0;
			}
		}
		else if(status == 1)
		{
			status = 3;
		}
		else if(status == 2)
		{
			stopwatch = 2;
		}
		else if(status == 3)
		{
			if(world_time_status == 0)
			{
				world_time_status = 2;
			}
		}
		else if(status == 4)
		{
			if(timer == 0)
			{
				timer_seconds = timer_seconds + 1;
				if(timer_seconds >= 60)
				{
					timer_seconds == 0;
				}
			}
		}
	}
	if(bit_is_clear(PINC,3))
	{
		if(status == 0)
		{
			status = 0;
			seconds = 0;
		}
		else if(status == 1)
		{
			status = 4;
		}
		else if(status == 2)
		{
			status = 0;
			stopwatch = 0;
		}
		else if(status == 3)
		{
			status = 0;
			stopwatch = 0;
			world_time_status = 0;
		}
		else if(status == 4)
		{
			status = 0;
			timer = 0;
			timer_minutes = 0;
			timer_seconds = 0;
		}
	}
	DDRC = 0xF0;
}

void controlActualStatus()
{
	if(status == 0)
	{
		LCD_Text(text);
	}
    else if(status == 1)
	{
		showMenu();
	}
	else if(status == 2)
	{
		useStopwatch();
	}
	else if(status == 3)
	{
		useTimeInTheWorld();
	}
	else if(status == 4)
	{
		useTimer();
	}
}

void generateActualTime()
{
	sprintf(secondsChar, "%i", seconds);
	sprintf(minutesChar, "%i", minutes);
	sprintf(hoursChar, "%i", hours);

	if(hours > 9)
	{
		strcpy(text, hoursChar);
	}
	else
	{
		strcpy(text, "0");
		strcat(text, hoursChar);
	}

	strcat(text, ":");

	if(minutes > 9)
	{
		strcat(text, minutesChar);
	}
	else
	{
		strcat(text, "0");
		strcat(text, minutesChar);
	}

	strcat(text, ":");

	if(seconds > 9)
	{
		strcat(text, secondsChar);
	}
	else
	{
		strcat(text, "0");
		strcat(text, secondsChar);
	}
}

void setTime()
{
	delay(1000);

	LCD_Clear();

	seconds = seconds + 1;
	if(seconds > 59)
	{
		minutes = minutes + 1;
		seconds = 0;
		if(minutes > 59)
		{
			hours = hours + 1;
			minutes = 0;
			if(hours == 24)
			{
				hours = 0;
			}
		}
	}
}

void showMenu()
{
	char menu_text[] = "MENU 1ZEGAR 2STOPER 3CZAS 4MINUT";
	LCD_Text(menu_text);
}

void useStopwatch()
{
	char stopwatch_text[] = "Stoper: ";

	if(stopwatch == 0) { }
	else if(stopwatch == 1)
	{
		stopwatch_seconds = stopwatch_seconds + 1;
		if(stopwatch_seconds > 59)
		{
			stopwatch_minutes = stopwatch_minutes + 1;
			stopwatch_seconds = 0;
			if(stopwatch_minutes > 59)
			{
				stopwatch_minutes = 0;
			}
		}
	}
	else if(stopwatch == 2)
	{
		stopwatch_seconds = 0;
		stopwatch_minutes = 0;
		stopwatch = 0;
	}

	sprintf(stopwatch_secondsChar, "%i", stopwatch_seconds);
	sprintf(stopwatch_minutesChar, "%i", stopwatch_minutes);

	if(stopwatch_minutes > 9)
	{
		strcat(stopwatch_text, stopwatch_minutesChar);
	}
	else
	{
		strcat(stopwatch_text, "0");
		strcat(stopwatch_text, stopwatch_minutesChar);
	}

	strcat(stopwatch_text, ":");

	if(stopwatch_seconds > 9)
	{
		strcat(stopwatch_text, stopwatch_secondsChar);
	}
	else
	{
		strcat(stopwatch_text, "0");
		strcat(stopwatch_text, stopwatch_secondsChar);
	}

	LCD_Text(stopwatch_text);
}

void useTimeInTheWorld()
{
	int other_hours;
	char other_hoursChar[] = "00";

	if(world_time_status == 0)
	{
		char world_time_menu[] = "Czas Swiat: 1Wawa 2Paryz 3NY";
		LCD_Text(world_time_menu);
	}
	else if(world_time_status == 1)
	{
		int paris_hours;
		strcpy(text, "Paris: ");
		if(hours < 1)
		{
			paris_hours = 24 + hours - 1;
		}
		else
		{
			paris_hours = hours - 1;
		}
		other_hours = paris_hours;
	}
	else if(world_time_status == 2)
	{
		int ny_hours;
		strcpy(text, "New York: ");
		if(hours < 6)
		{
			ny_hours = 24 + hours - 6;
		}
		else
		{
			ny_hours = hours - 1;
		}
		other_hours = ny_hours;
	}
	if (world_time_status == 1 || world_time_status == 2)
	{
		sprintf(other_hoursChar, "%i", other_hours);
		if(other_hours > 9)
		{
			strcat(text, other_hoursChar);
		}
		else
		{
			strcat(text, "0");
	        strcat(text, other_hoursChar);
		}

		strcat(text, ":");

		if(minutes > 9)
		{
			strcat(text, minutesChar);
		}
		else
		{
			strcat(text, "0");
		       	strcat(text, minutesChar);
		}

		strcat(text, ":");

		if(seconds > 9)
		{
			strcat(text, secondsChar);
		}
		else
		{
			strcat(text, "0");
			strcat(text, secondsChar);
		}

		LCD_Text(text);
	}
}

void useTimer()
{
	char timer_text[] = "Minutnik: ";

	if(timer == 0)
	{
		
	}
	else if(timer == 1)
	{
		timer_seconds = timer_seconds - 1;
		if(timer_seconds < 0)
		{
			timer_minutes = timer_minutes - 1;
			timer_seconds = 59;
		}
		if(timer_minutes < 0 && timer_seconds == 59)
		{
			timer = 2;
			timer_minutes = 0;
		}
	}

	sprintf(timer_secondsChar, "%i", timer_seconds);
	sprintf(timer_minutesChar, "%i", timer_minutes);

	if(timer_minutes > 9)
	{
		strcat(timer_text, timer_minutesChar);
	}
	else
	{
		strcat(timer_text, "0");
		strcat(timer_text, timer_minutesChar);
	}

	strcat(timer_text, ":");

	if(timer_seconds > 9)
	{
		strcat(timer_text, timer_secondsChar);
	}
	else
	{
		strcat(timer_text, "0");
		strcat(timer_text, timer_secondsChar);
	}
	if(timer != 2)
	{
		LCD_Text(timer_text);
	}
	else
	{
		char timer_boom = "Koniec czasu! Czas minal! Hello!";
		LCD_Text(timer_boom);
	}
}

int main()
{
	LCD_Enable();

	DDRC = 0xF0;

	while(1)
	{
		detectButtonClick();

		generateActualTime();

		controlActualStatus();

        setTime();
	}
}
