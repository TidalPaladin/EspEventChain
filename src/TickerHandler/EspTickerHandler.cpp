#include "EspTickerHandler.h"


EspTickerHandler::EspTickerHandler() {

}

void EspTickerHandler::attach(uint32_t ms, const callback_t &callback) {
    assignMemberCallback(callback);

    _attach_ms(ms * 1000);
}

void EspTickerHandler::attach_ms(uint32_t ms, const callback_t &callback) {
    assignMemberCallback(callback);

    _attach_ms(ms);
}

void EspTickerHandler::once(uint32_t ms, const callback_t &callback) {
    assignMemberCallback(callback);

    _once_ms(ms * 1000);
}

void EspTickerHandler::once_ms(uint32_t ms, const callback_t &callback) {
    assignMemberCallback(callback);

    _once_ms(ms);
}




void EspTickerHandler::sHandleTick(void* ptr) {
    if(ptr == nullptr) {
        panic();
    }

    EspTickerHandler *handler = (EspTickerHandler*)(ptr);
    handler->callback();
}

void EspTickerHandler::assignMemberCallback(const callback_t &callback) {
    if( !callback )
        panic();

    this->callback = callback;
}