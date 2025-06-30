// main.c
#include <pigpio.h>
#include <stdio.h>

#define BUTTON_IMPLEMENTATION
#include "../../src/button.h"

int main(void)
{
	int ret = 1;

	if ( gpioInitialise() < 0)
		puts("unable to init pigpio!");
	else
	{
		if ( ! Button_Init() )
			puts("unable to init button!");
		else
		{
			printf("press button...");
			fflush(stdout);
			
			while ( 1 )
			{
				float f = Button_GetPressDuration();
				if ( f )
				{
					printf("duration: %.2fs\n", f);
					fflush(stdout);
				}
				
				gpioDelay(10000);
			}
			
			ret = 0;
		}
		
		gpioTerminate();
	}
	
	return ret;
}
