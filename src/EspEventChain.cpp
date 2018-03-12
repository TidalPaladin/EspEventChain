#include "EspEventChain.h"

/**
 *
 *
 * 	Constructors
 *
 */
EspEventChain::EspEventChain() { construct(); }

EspEventChain::EspEventChain(size_t num_events) {
	_events.reserve(num_events);
	construct();
}

/**
 *
 *
 * 	Public methods
 *
 */

unsigned long EspEventChain::getTotalTime() const {
	if (numEvents() == 0) return 0;
	return getTotalTimeBefore(numEvents() - 1) + getTimeOf(numEvents() - 1);
}

unsigned long EspEventChain::getTotalTimeBefore(size_t event_num) const {
	if (event_num == 0) return 0;
	__ESP_EVENT_CHAIN_CHECK_POS__(event_num);

	// Iterate through list, adding up getTimeOf()
	unsigned long total = 0;
	for (int pos = 0; pos < event_num; pos++) {
		total += getTimeOf(pos);
	}

	return total;
}

unsigned long EspEventChain::getTimeOf(size_t event_num) const {
	__ESP_EVENT_CHAIN_CHECK_POS__(event_num);
	return _events.at(event_num).getTime();
}

int EspEventChain::getPositionFromHandle(const char *handle) const {
	__ESP_EVENT_CHAIN_CHECK_PTR__(handle);

	citerator_t target_pos = getIteratorFromHandle(handle);
	if (target_pos != _events.cend())
		return std::distance(_events.cbegin(), target_pos);
	else
		return -1;
}

EspEventChain::citerator_t
EspEventChain::getIteratorFromHandle(const char *handle) const {
	__ESP_EVENT_CHAIN_CHECK_PTR__(handle);

	citerator_t result = _events.cend();

	// Make sure handle is good
	if (!strcmp(handle, "null")) return result;

	// Look for handle in the container
	for (auto it = _events.cbegin(); it != _events.cend(); it++) {
		if (!strcmp(handle, it->getHandle())) result = it;
	}
	return result;
}

void EspEventChain::changeTimeOf(size_t pos, unsigned long ms) {
	__ESP_EVENT_CHAIN_CHECK_POS__(pos);
	_events.at(pos).setTime(ms);
	ESP_LOGI(__ESP_EVENT_CHAIN_DEBUG_TAG__,
			 "Changed time of event at index = %i to %i", pos, ms);
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
	ESP_LOGI(__ESP_EVENT_CHAIN_DEBUG_TAG__, "Event added to chain");
}

void EspEventChain::insert(size_t event_num, const EspEvent &event) {
	if (event_num == numEvents()) {
		_events.push_back(event);
		return;
	}

	__ESP_EVENT_CHAIN_CHECK_POS__(event_num);
	auto insert_target = _events.begin();
	std::advance(insert_target, event_num);
	_events.insert(insert_target, event);
	ESP_LOGI(__ESP_EVENT_CHAIN_DEBUG_TAG__, "Event added to chain");
}

EspEvent EspEventChain::remove(size_t event_num) {
	__ESP_EVENT_CHAIN_CHECK_POS__(event_num);

	if (isRunning()) {
		ESP_LOGW(__ESP_EVENT_CHAIN_DEBUG_TAG__,
				 "Called removed at pos %i / size %i while chain running",
				 event_num, numEvents());
	}

	EspEvent result = _events.at(event_num);

	auto erase_target = _events.begin();
	std::advance(erase_target, event_num);
	_events.erase(erase_target);

	ESP_LOGI(__ESP_EVENT_CHAIN_DEBUG_TAG__,
			 "Removed event at index = %i, numEvents() = %i", event_num,
			 numEvents());

	return result;
}

size_t EspEventChain::numEvents() const { return _events.size(); }

/**
 *
 *
 * 	start() variants
 *
 */

void EspEventChain::start() { startFrom(0); }

void EspEventChain::startFrom(size_t event_num) {
	ESP_LOGI(__ESP_EVENT_CHAIN_DEBUG_TAG__, "Starting chain from index = %i",
			 event_num);
	setCurrentEventTo(event_num);
	_start();
}

void EspEventChain::runOnceStartFrom(size_t event_num) {
	ESP_LOGI(__ESP_EVENT_CHAIN_DEBUG_TAG__,
			 "Set run-once flag ahead of chain start");
	_runOnceFlag = true;
	startFrom(event_num);
}

void EspEventChain::runOnce() { runOnceStartFrom(0); }

void EspEventChain::stop() {
	if (_started) {
		_started = false;
		ESP_LOGI(__ESP_EVENT_CHAIN_DEBUG_TAG__, "Stopped chain");
		while (_currentEvent != _events.cend()) {
			yield();
		}

		// #ifndef ESP32
		// 		tick.detach();
		// #endif
	}
}

void EspEventChain::_start() {
	if (_events.empty()) {
		ESP_LOGW(__ESP_EVENT_CHAIN_DEBUG_TAG__,
				 "Not starting chain because numEvents() = 0");
		return;
	}
	if (!containsNonzeroEvent()) {
		ESP_LOGW(__ESP_EVENT_CHAIN_DEBUG_TAG__,
				 "Not starting chain because all times are zero");
		return;
	}
	_started = true;

#ifdef ESP32

	// Create a task to do everything
	xTaskCreate(sHandleTick,	 // Function
				"EspEventChain", // Name
				5000,			 // Stack size in words
				(void *)this,	// Parameter
				2,				 // Task priority
				NULL			 // Task handle
	);

#else

	// Run first event manually to start cascade
	sHandleTick(this);

#endif
}

void EspEventChain::sHandleTick(void *ptr) {
	__ESP_EVENT_CHAIN_CHECK_PTR__(ptr);
	EspEventChain *cast = static_cast<EspEventChain *>(ptr);
	cast->handleTick();
}

void EspEventChain::handleTick() {
#ifdef ESP32

	UBaseType_t stack_size = uxTaskGetStackHighWaterMark(NULL);
	ESP_LOGD(__ESP_EVENT_CHAIN_DEBUG_TAG__, "Stack usage estimate: %i",
			 stack_size);

	TickType_t xLastWakeTime = xTaskGetTickCount();
	TickType_t xPeriod;

	while (_started) {

		// Run the event
		_currentEvent->runEvent();

		if (!advanceToNextCallable()) {
			ESP_LOGD(__ESP_EVENT_CHAIN_DEBUG_TAG__,
					 "No more callables to advance to");
			_runOnceFlag = false;
			stop();
		}

		if (uxTaskGetStackHighWaterMark(NULL) > stack_size) {
			stack_size = uxTaskGetStackHighWaterMark(NULL);
			ESP_LOGD(__ESP_EVENT_CHAIN_DEBUG_TAG__, "Stack usage estimate: %i",
					 stack_size);
		}

		// Delay until the next event
		xPeriod = pdMS_TO_TICKS(_currentEvent->getTime());
		if (xPeriod) {
			vTaskDelayUntil(&xLastWakeTime, xPeriod);
		} else {
			taskYIELD();
		}
	}

	// If we get here stop() was called, so task should delete itself
	// Position iterator at end() so we can force stop() to block until deletion
	_currentEvent = _events.cend();
	ESP_LOGV(__ESP_EVENT_CHAIN_DEBUG_TAG__, "Deleting task...");
	vTaskDelete(NULL);

#else

	_currentEvent->runEvent();
	// Move to the next callable, if we hit the end with runOnce() then abort
	// if( !advanceToNextCallable() )
	// 	return 0;

	// const unsigned long delay = _currentEvent->getTime();
	// if( delay == 0 ) {
	// 	handleTick();
	// 	return delay;
	// }

	// tick.once_ms(delay, [this]() {
	// 	this->handleTick();
	// });
	// return delay;

#endif
}

bool EspEventChain::advanceToNextCallable() {
	// Base case, we took a step forward and hit a valid event
	_currentEvent++;
	if (validCurrentEvent()) {
		return true;
	} else if (!atEndOfChain()) {
		return advanceToNextCallable();
	}

	ESP_LOGD(__ESP_EVENT_CHAIN_DEBUG_TAG__, "Reached end of chain");
	_currentEvent = _events.cbegin();
	return !_runOnceFlag;
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
	__ESP_EVENT_CHAIN_CHECK_POS__(event_num);

	_currentEvent = _events.cbegin();
	std::advance(_currentEvent, event_num);
}

void EspEventChain::construct() {
	_runOnceFlag = false;
	_started = false;
}

bool EspEventChain::containsNonzeroEvent() const {
	for (EspEvent event : _events) {
		if (event.getTime() != 0) return true;
	}
	return false;
}
bool EspEventChain::validCurrentEvent() const {
	return !atEndOfChain() && *_currentEvent;
}
bool EspEventChain::atEndOfChain() const {
	return _currentEvent == _events.cend();
}
