#include "EspTickerHandler.h"

unsigned long EspTickerHandler::_timeAtLastArming = 0;
const char* EspTickerHandler::TAG = "TickerWrap";


EspTickerHandler::EspTickerHandler() {

}

void EspTickerHandler::attach(uint32_t ms, const callback_t &callback) {
    assignMemberCallback(callback);
    setLastArmedTime();
    _attach_ms(ms * 1000);
}

void EspTickerHandler::attach_ms(uint32_t ms, const callback_t &callback) {
    assignMemberCallback(callback);
    setLastArmedTime();
    _attach_ms(ms);
}

void EspTickerHandler::once(uint32_t ms, const callback_t &callback) {
    assignMemberCallback(callback);
    setLastArmedTime();
    _once_ms(ms * 1000);
}

void EspTickerHandler::once_ms(uint32_t ms, const callback_t &callback) {
    assignMemberCallback(callback);
    setLastArmedTime();
    _once_ms(ms);
}




void EspTickerHandler::sHandleTick(void* ptr) {
    if(ptr == nullptr) {
       ESP_LOGE(TAG, "null pointer");
       panic();
    }

    EspTickerHandler *handler = static_cast<EspTickerHandler*>(ptr);
    handler->handleTick(); 
}

void EspTickerHandler::assignMemberCallback(const callback_t &callback) {
    if( !callback )
        panic();

    this->callback = callback;
}




void EspTickerHandler::setLastArmedTime() {
    _timeAtLastArming = millis();
}



bool EspTickerHandler::checkLastArmedTime() {
    return _timeAtLastArming != millis(); 
}
