#include "EspEventChain.h"

/**
 * 
 * 
 * 	Constructors
 * 
 */
EspEventChain::EspEventChain() { construct(); }

EspEventChain::EspEventChain(size_t num_events)
{
	_events.reserve(num_events);
	construct();
}





/**
 * 
 * 
 * 	Public methods
 * 
 */


unsigned long EspEventChain::totalTime() const {
	return totalTimeBefore( numEvents()-1 ) + getTimeOf(numEvents()-1);  
}

unsigned long EspEventChain::totalTimeBefore(size_t event_num) const {

	if( !checkValidEventNum(event_num) ) 
		panic();

	// Time before event 0 is 0
	else if(event_num == 0)
		return 0;

	// Iterate through list, adding up getTimeOf()
	unsigned long total = 0;
	for(int pos = 0; pos < event_num; pos++){
		total += getTimeOf(pos);
	}
	return total;
} 

unsigned long EspEventChain::getTimeOf(size_t event_num) const {
	if( !checkValidEventNum(event_num) )
		panic();

	return _events.at(event_num).getTime();
}

int EspEventChain::getPositionFromHandle(const char* handle) const {
	citerator_t target_pos = getIteratorFromHandle(handle);
	if( target_pos != _events.cend() ) 
		return std::distance(_events.cbegin(), target_pos);
	else
		return -1;
}

EspEventChain::citerator_t EspEventChain::getIteratorFromHandle(const char* handle) const {
	citerator_t result = _events.cend();

	// Make sure handle is good
	if(handle == nullptr || !strcmp(handle, "null"))
		return result;

	// Look for handle in the container
	for(auto it = _events.cbegin(); it != _events.cend(); it++) {
		if( !strcmp(handle, it->getHandle())) 
			result = it;
	}
	return result;
}

void EspEventChain::changeTimeOf(size_t pos, unsigned long ms) {
	if( pos >= numEvents() ) panic();
	_events.at(pos).setTime(ms);
}




/**
 * 
 * 
 * 
 * 		Event insertion / removal
 * 
 * 
 */


void EspEventChain::push_back(const EspEvent &event) {
	_events.push_back(event);
}

void EspEventChain::insert(size_t event_num, const EspEvent &event) {
	if( !checkValidEventNum(event_num) )
		panic();
	
	auto insert_target = _events.begin();
	std::advance(insert_target, event_num);
	_events.insert(insert_target, event);
}

EspEvent EspEventChain::remove(size_t event_num) {
	if( !checkValidEventNum(event_num) )
		panic();

	EspEvent result = _events.at(event_num); 

	auto erase_target = _events.begin(); 
	std::advance(erase_target, event_num);
	_events.erase(erase_target);

	return result;
}




size_t EspEventChain::numEvents() const { return _events.size(); }

 

/**
 * 
 * 
 * 	start() variants
 * 
 */

void EspEventChain::start() {
	startFrom(0);
}

void EspEventChain::startFrom(size_t event_num) {
	setCurrentEventTo(event_num);
	handleTick();
	_started = true;
}

void EspEventChain::runOnceStartFrom(size_t event_num) {
	_runOnceFlag = true;
	startFrom(event_num);
}

void EspEventChain::runOnce() {
	runOnceStartFrom(0);
}


void EspEventChain::stop() {

#ifdef ESP32
	vTaskDelete(_taskHandle);
#else
	_tick.detach();
#endif
	_started = false;

}









/**
 * 
 * 	Ticker interface methods
 * 
 * 
 */

void EspEventChain::sHandleTick(void *ptr) {
	EspEventChain *pChain = (EspEventChain*)ptr;
	if(pChain == nullptr) panic();

	pChain->handleTick();
}

void EspEventChain::handleTick() {
	unsigned long time_for_callback = _currentEvent->runEvent();
	unsigned long delay = scheduleNextEvent(time_for_callback);

#ifdef ESP32
	preventTaskEnd(delay);
#endif
}


bool EspEventChain::advanceToNextCallable() {

	// First advance to the next event in the chain
	const citerator_t INITIAL_POS = _currentEvent++;

	if( atEndOfChain() ){

		// Reset to beginning of chain
		setCurrentEventTo(0);

		// If we were only supposed to run once we're done
		if( _runOnceFlag ) {
			stop();
			_runOnceFlag = false;
			return false;
		}

	}

	// If we made it here, we've reset to the start of the chain
	// Look for the next valid current event from the start of the chain
	while( !validCurrentEvent() && !atEndOfChain() ) {
		_currentEvent++;
	}

	// We have either found another valid event, or _currentEvent remained the same before
	// the call to this method
	return true;
}


unsigned long EspEventChain::scheduleNextEvent(unsigned long offset_ms) {
	if( !advanceToNextCallable() )
		return 0;

	unsigned long delay = _currentEvent->getTime();


	// Handle time=0 events that should be run immediately, no scheduling
	while(delay == 0) {
		_currentEvent->runEvent();
		advanceToNextCallable();
		delay = _currentEvent->getTime();
	}


	delay -= ( offset_ms <= delay ? offset_ms : delay); 



// Schedule event in appropriate manner
#ifdef ESP32
	
	xTaskCreate(
		sHandleTick,    // Function
		"EventChain",     // Name
		1000,           // Stack size in words
		(void*)this,    // Parameter
		1,              // Task priority
		&_taskHandle    // Task handle
	);
  
#else

   _tick.once_ms(delay, sHandleTick, (void*)this);

#endif   

	return delay;
}


#ifdef ESP32
void EspEventChain::preventTaskEnd(unsigned long howLong_ms) {

	TickType_t xLastWakeTime = xTaskGetTickCount(); // Initialize for delayUntil

	while(true){
		const TickType_t xFrequency = howLong_ms / portTICK_PERIOD_MS;
		vTaskDelayUntil(&xLastWakeTime, xFrequency);
	}

}
#endif

void EspEventChain::setCurrentEventTo(size_t event_num) {
	if( !checkValidEventNum(event_num) )
		panic();

	_currentEvent = _events.cbegin();
	 std::advance(_currentEvent, event_num);
}

bool EspEventChain::checkValidEventNum(size_t event_num) const {
	if(event_num >= _events.size()) {
		Serial.println("EspEventChain - checkValidEventNum failed");
		return false;
	}
	return true;
}

void EspEventChain::construct() {
	_runOnceFlag = false;
	_started = false;
}






bool EspEventChain::validCurrentEvent() const { return !atEndOfChain() && *_currentEvent; }
bool EspEventChain::atEndOfChain() const { return _currentEvent == _events.cend(); }
