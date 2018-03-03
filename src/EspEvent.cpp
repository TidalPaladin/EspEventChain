#include "EspEvent.h"

EspEvent::EspEvent() : _time_ms(0) { }

EspEvent::EspEvent(unsigned long relative_time_ms, callback_t event, const char* identifying_handle)
:
_time_ms(relative_time_ms),
_callback(event),
_HANDLE(identifying_handle)
{
    
}

EspEvent::operator bool() const { return (bool)_callback; }
unsigned long EspEvent::getTime() const { return _time_ms; }
const char* EspEvent::getHandle() const { return _HANDLE; }

void EspEvent::runEvent() const {
	if(_callback) {
		(_callback)();
	}
}

EspEvent &EspEvent::setTime(unsigned long ms) {
	_time_ms = ms;
	return *this;
}

EspEvent &EspEvent::setCallback(const callback_t &callback) {
	_callback = callback;
	return *this;
}