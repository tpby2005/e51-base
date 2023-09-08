#include <stdio.h>
#include <stdbool.h>
#include "definitions.h"

#define MAX_PRINT_LEN 400

static volatile bool isRTCExpired = false;
static volatile bool isUSARTTxComplete = true;
static uint8_t uartTxBuffer[MAX_PRINT_LEN] = {0};

static void rtcEventHandler (RTC_TIMER32_INT_MASK intCause, uintptr_t context) {
    if (intCause & RTC_MODE0_INTENSET_CMP0_Msk) {            
        isRTCExpired    = true;
    }
}
static void usartDmaChannelHandler(DMAC_TRANSFER_EVENT event, uintptr_t contextHandle) {
    if (event == DMAC_TRANSFER_EVENT_COMPLETE) {
        isUSARTTxComplete = true;
    }
}

int main(void) {
    SYS_Initialize(NULL);

    DMAC_ChannelCallbackRegister(DMAC_CHANNEL_0, usartDmaChannelHandler, 0);
    RTC_Timer32CallbackRegister(rtcEventHandler, 0);
    RTC_Timer32Compare0Set(128);
    RTC_Timer32CounterSet(0);
    RTC_Timer32Start();

    while(true) {
        LED0_Toggle();

        isRTCExpired = false;
        isUSARTTxComplete = false;

        snprintf((char *)uartTxBuffer, MAX_PRINT_LEN, "Hello World!\r\n");
        DMAC_ChannelTransfer(DMAC_CHANNEL_0, uartTxBuffer, (const void *)&(SERCOM5_REGS->USART_INT.SERCOM_DATA), strlen((const char *)uartTxBuffer));

        while((isRTCExpired == false) || (isUSARTTxComplete == false));
    }
}