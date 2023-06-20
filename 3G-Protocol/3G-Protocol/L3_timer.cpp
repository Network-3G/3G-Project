#include "mbed.h"
#include "L3_FSMevent.h"
#include "protocol_parameters.h"

static Timeout timer;
static uint8_t timerStatus = 0;

// timer event
void L3_timer_timeoutHandler(void)
{
    timerStatus = 0;
}

void L3_timer_startTimer()
{
    uint8_t waitTime = 3; 
    timer.attach(L3_timer_timeoutHandler, waitTime);
    timerStatus = 1;
}

void L3_timer_stopTimer()
{
    timer.detach();
    timerStatus = 0;
}

void L3_timer_Chat_timeoutHandler(void)
{
    L3_event_setEventFlag(Chat_Timer_Expire);
    //printf("chat timer hander\n");
    timerStatus = 0;
}

void L3_timer_Chat_Timer()
{
    uint8_t chat_waitTime = 30;
    timer.attach(L3_timer_Chat_timeoutHandler, chat_waitTime);
    timerStatus = 1;
}

uint8_t L3_timer_getTimerStatus()
{
    return timerStatus;
}
