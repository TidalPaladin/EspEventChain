#ifdef ESP32
#ifndef __ESP32_TICKER_HANDLER_H__
#define __ESP32_TICKER_HANDLER_H__


#include "EspTickerHandler.h"


class TickerHandler : public EspTickerHandler {

public:

    TickerHandler() : attached(false) {}

    virtual bool active() { return attached; }

    virtual void detach() { 
        attached = false;
        if(_taskHandle != NULL)
            vTaskDelete(_taskHandle); 
    }

private:

    TaskHandle_t _taskHandle = NULL;
    unsigned long tickTime = 0;
    bool attached : 1;

private:

    virtual void handleTick() {

        do {
            delayTask(tickTime);
            callback ? (callback)() : detach();
        } 
        while(active());

        vTaskDelete(NULL);
    }

    /**
     * @brief Abstract method that handles the creation of a periodic method call
     * in the manner appropriate to the chip in use.
     * 
     * @param ms    The period of the function call in milliseconds, 0 < ms
     * 
     * post: Member functor will be called every 'ms' milliseconds
     * 
     */
    virtual void _attach_ms(uint32_t ms) {
        tickTime = ms;
        attached = true;
        createTask();
    }

    /**
     * @brief Abstract method that handles the creation of a periodic method call
     * in the manner appropriate to the chip in use.
     * 
     * @param ms    The period of the function call in milliseconds, 0 < ms
     * 
     * post: Member functor will be called every 'ms' milliseconds
     * 
     */
    virtual void _once_ms(uint32_t ms) {
        tickTime = ms;
        attached = false;
        createTask();
    }

    /**
     * @brief A while loop that runs endlessly until a given time has elapsed. Needed to prevent premature task
     * end on ESP32
     * 
     * @param ms    The time in milliseconds to loop for, 0 < ms
     * 
     * post: loop will run for 'ms' milliseconds
     * 
     */
    static void delayTask(uint32_t ms) {
        if(ms == 0)
            return;

        TickType_t xLastWakeTime = xTaskGetTickCount(); // Initialize for delayUntil

        while(true){
            const TickType_t xFrequency = ms / portTICK_PERIOD_MS;
            vTaskDelayUntil(&xLastWakeTime, xFrequency);
        }

    }

    void createTask(size_t stack_size = 1000, uint32_t priority = 1) {

            xTaskCreate(
                sHandleTick,    // Function
                "TickHandler32",     // Name
                stack_size,           // Stack size in words
                (void*)this,    // Parameter
                1,              // Task priority
                &_taskHandle    // Task handle
            );
    }
    

};

#endif
#endif