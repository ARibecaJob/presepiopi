// button.h
#ifndef BUTTON_H
#define BUTTON_H

int Button_Init(void);
float Button_GetPressDuration(void);

#endif // BUTTON_H

#ifdef BUTTON_IMPLEMENTATION

#define EVENT_QUEUE_SIZE 10
volatile float press_durations[EVENT_QUEUE_SIZE] = {0};
volatile uint8_t queue_head = 0, queue_tail = 0;

const int BUTTON_GPIO__ = 17;
const int BUTTON_DEBOUNCE_US__ = 10000;

volatile uint32_t buttonPressStartTicks__ = 0;

static void Button_Callback__(int gpio, int level, uint32_t tick, void* user)
{
    (void)gpio;
    (void)user;

    if ( 1 == level )
        buttonPressStartTicks__ = tick;
    else if ( buttonPressStartTicks__ )
	{
		uint32_t duration_us = tick - buttonPressStartTicks__;
		buttonPressStartTicks__ = 0;
		
		press_durations[queue_head] = (float)duration_us / 1e6f;
		queue_head = (queue_head + 1) % EVENT_QUEUE_SIZE;
	}
}

int Button_Init(void)
{
	int ret = ! gpioSetMode(BUTTON_GPIO__, PI_INPUT);
	if ( ret ) ret = ! gpioSetPullUpDown(BUTTON_GPIO__, PI_PUD_DOWN);
	if ( ret ) ret = ! gpioGlitchFilter(BUTTON_GPIO__, BUTTON_DEBOUNCE_US__);
	if ( ret ) ret = ! gpioSetAlertFuncEx(BUTTON_GPIO__, Button_Callback__, NULL);
	return ret;
}

float Button_GetPressDuration(void)
{
	float duration = 0.f;
	if ( queue_head != queue_tail )
	{
		duration = press_durations[queue_tail];
		queue_tail = (queue_tail + 1) % EVENT_QUEUE_SIZE;
	}
	return duration;
}

#endif // BUTTON_IMPLEMENTATION
