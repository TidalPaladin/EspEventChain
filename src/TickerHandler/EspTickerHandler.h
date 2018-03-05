#ifndef __ESP_TICKER_HANDLER_H__
#define __ESP_TICKER_HANDLER_H__

#include <Arduino.h>
#include <functional>


class EspTickerHandler {

public:

    typedef std::function<void()> callback_t;

protected:

    callback_t callback;
    static unsigned long _timeAtLastArming;

public:


    EspTickerHandler();

    /**
     * @brief Starts ticking, running 'callback' after 'ms' milliseconds and
     * repeating the call with a period of 'ms' milliseconds until detach() is
     * called.
     * 
     * @param ms        The period in milliseconds between ticks, 0 < ms
     * 
     * @param callback  The std::function<void()> to call
     * 
     * post:    After ms milliseconds, callback will be called. Cycle repeats every ms
     *          milliseconds
     */
    void attach(uint32_t ms, const callback_t &callback);

    /**
     * @brief Starts ticking, running 'callback' after 'ms' milliseconds and
     * repeating the call with a period of 'ms' milliseconds until detach() is
     * called.
     * 
     * @param ms        The period in milliseconds between ticks, 0 < ms
     * 
     * @param callback  The std::function<void()> to call
     * 
     * post:    After ms milliseconds, callback will be called. Cycle repeats every ms
     *          milliseconds
     */
    void attach_ms(uint32_t ms, const callback_t &callback);

    /**
     * @brief Runs 'callback' once after 'ms' millisecionds have elapsed
     * 
     * @param ms        The period in milliseconds between ticks, 0 < ms
     * 
     * @param callback  The std::function<void()> to call
     * 
     * post: After ms milliseconds, callback will be called once
     */
    void once(uint32_t ms, const callback_t &callback);

    /**
     * @brief Runs 'callback' once after 'ms' millisecionds have elapsed
     * 
     * @param ms        The period in milliseconds between ticks, 0 < ms
     * 
     * @param callback  The std::function<void()> to call
     * 
     * post: After ms milliseconds, callback will be called once
     */
    void once_ms(uint32_t ms, const callback_t &callback);

    /**
     * @brief Gets whether the Ticker is armed
     * 
     * @return  true if Ticker has a once_ms() call to be made or has been attached
     *          with attach_ms(), false otherwise.
     */
    virtual bool active() = 0;

    /**
     * @brief Stops all periodic action
     * 
     * post: No calls of the callback are scheduled
     */
    virtual void detach() = 0;



protected:

    /**
     * @brief Static wrapper to serve as a ticker target
     * 
     * @param ptr   A EspTickerHandler pointer cast as void*,
     *              (EspTickerHandler*)ptr must be a valid cast
     * 
     * post: member callback called
     */
    static void sHandleTick(void *ptr);

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
    virtual void _attach_ms(uint32_t ms) = 0;

    /**
     * @brief Abstract method that handles the creation of a periodic method call
     * in the manner appropriate to the chip in use.
     * 
     * @param ms    The period of the function call in milliseconds, 0 < ms
     * 
     * post: Member functor will be called every 'ms' milliseconds
     * 
     */
    virtual void _once_ms(uint32_t ms) = 0;

    /**
     * @brief Helper method to assign a passed callback reference to the member
     * variable
     * 
     * @param callback  The callback to capture
     * 
     * post: this->callback = callback
     */
    void assignMemberCallback(const callback_t &callback);

    /**
     * @brief Keeps track of the time when the ticker was armed for later comparison
     * 
     * post: _timeAtLastArming = millis()
     */
    static void setLastArmedTime();

    /**
     * @brief Checks millis() against the last armed time to ensure that the clock has ticked
     * between this tick and the last
     * 
     * @return true if the clock has ticked between last event and this event
     */
    static bool checkLastArmedTime();

};

#include "Esp8266TickerHandler.h"
#include "Esp32TickerHandler.h"


#endif