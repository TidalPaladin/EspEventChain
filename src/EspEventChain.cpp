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
	checkValidEventNum(event_num, __FILE__, __LINE__, __FUNCTION__);

	// Iterate through list, adding up getTimeOf()
	unsigned long total = 0;
	for(int pos = 0; pos < event_num; pos++){
		total += getTimeOf(pos);
	}
	return total;
} 

unsigned long EspEventChain::getTimeOf(size_t event_num) const {
	checkValidEventNum(event_num, __FILE__, __LINE__, __FUNCTION__);
	return _events.at(event_num).getTime();
}

int EspEventChain::getPositionFromHandle(const char* handle) const {
	checkValidPtr(handle, __FILE__, __LINE__, __FUNCTION__);

	citerator_t target_pos = getIteratorFromHandle(handle);
	if( target_pos != _events.cend() ) 
		return std::distance(_events.cbegin(), target_pos);
	else
		return -1;
}

EspEventChain::citerator_t EspEventChain::getIteratorFromHandle(const char* handle) const {
	checkValidPtr(handle, __FILE__, __LINE__, __FUNCTION__);

	citerator_t result = _events.cend();

	// Make sure handle is good
	if(!strcmp(handle, "null"))
		return result;

	// Look for handle in the container
	for(auto it = _events.cbegin(); it != _events.cend(); it++) {
		if( !strcmp(handle, it->getHandle())) 
			result = it;
	}
	return result;
}

void EspEventChain::changeTimeOf(size_t pos, unsigned long ms) {
	checkValidTime(ms, __FILE__, __LINE__, __FUNCTION__);
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
	checkValidEventNum(event_num, __FILE__, __LINE__, __FUNCTION__);
	
	auto insert_target = _events.begin();
	std::advance(insert_target, event_num);
	_events.insert(insert_target, event);
}

EspEvent EspEventChain::remove(size_t event_num) {
	checkValidEventNum(event_num, __FILE__, __LINE__, __FUNCTION__);

	if( isRunning() && !containsNonzeroEvent() ) {
		printErr(__FILE__, __LINE__, __FUNCTION__, "All events have time = 0");
		panic();
	}

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

	if( !containsNonzeroEvent() ) {
		printErr(__FILE__, __LINE__, __FUNCTION__, "All events have time = 0");
		panic();
	}

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
	tick.detach();
}











void EspEventChain::handleTick() {
	_currentEvent->runEvent();
	scheduleNextEvent();
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


unsigned long EspEventChain::scheduleNextEvent() {	
		
	// Move to the next callable, if we hit the end with runOnce() then abort
	if( !advanceToNextCallable() )
		return 0;

	const unsigned long delay = _currentEvent->getTime();
	if( delay == 0 ) {
		yield();
		handleTick();
		return delay;
	}

	tick.once_ms(delay, [this]() {
		this->handleTick();
	});
	yield();
	return delay;
}







/**
 * 
 * 
 * 
 * 		Helpers
 * 
 * 
 */




void EspEventChain::setCurrentEventTo(size_t event_num) {
	checkValidEventNum(event_num, __FILE__, __LINE__, __FUNCTION__);

	_currentEvent = _events.cbegin();
	 std::advance(_currentEvent, event_num);
}


void EspEventChain::construct() {
	_runOnceFlag = false;
	_started = false;
}


bool EspEventChain::containsNonzeroEvent() const { 
	for(EspEvent event : _events) {
		if(event.getTime() != 0)
			return true;
	}
	return false;
}
bool EspEventChain::validCurrentEvent() const { return !atEndOfChain() && *_currentEvent; }
bool EspEventChain::atEndOfChain() const { return _currentEvent == _events.cend(); }
