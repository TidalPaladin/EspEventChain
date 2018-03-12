#ifndef ESP32
#ifndef __ESP8266_TICKER_HANDLER_H__
#define __ESP8266_TICKER_HANDLER_H__


#include "EspTickerHandler.h"
#include <Ticker.h>
  

class TickerHandler : public EspTickerHandler {

private:

    Ticker _tick;
    unsigned long _nextExpectedCallTime;

public:

    virtual bool active() { return _tick.active(); }

    virtual void detach() { _tick.detach(); }

private:

    virtual void handleTick() {
            if(!checkLastArmedTime()) return;

        // if(ptr == nullptr) {
        //     panic();
        // }
        
        EspTickerHandler *handler = static_cast<EspTickerHandler*>(ptr);
        
        //if(!handler->callback) panic();
        
        handler->callback();
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
        _nextExpectedCallTime = millis() + ms;
        _tick.attach_ms( ms, sHandleTick, (void*)this );
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
        _nextExpectedCallTime = millis() + ms;
        _tick.once_ms( ms, sHandleTick, (void*)this );
    }

};

#endif
#endif