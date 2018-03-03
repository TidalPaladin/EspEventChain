#ifndef __ESP8266_TICKER_HANDLER_H__
#define __ESP8266_TICKER_HANDLER_H__

#ifndef ESP32

#include "EspTickerHandler.h"
#include <Ticker.h>
  

class TickerHandler : public EspTickerHandler {

private:

    Ticker _tick;

public:

    virtual bool active() { return _tick.active(); }

    virtual void detach() { _tick.detach(); }

private:

    

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
        _tick.once_ms( ms, sHandleTick, (void*)this );
    }

};

#endif
#endif