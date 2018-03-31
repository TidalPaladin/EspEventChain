/**
 * @file EspEventChain.h
 * @author Scott Chase Waggener tidal@utexas.edu
 * @date 2/2/18
 *
 * @description
 * Tracks a collection of timings that can be used with <Ticker.h> or another
 * scheduling class to carry out events separated by irregular intervals
 *
 *
 *
 */

#ifndef __ESP_EVENT_CHAIN_H__
#define __ESP_EVENT_CHAIN_H__

#define __ESP_EVENT_CHAIN_DEBUG_TAG__ "*EspEvent"

#ifndef __ESP_EVENT_CHAIN_DEBUG_SRC__
#define __ESP_EVENT_CHAIN_DEBUG_SRC__ Serial
#endif

#define __ESP_EVENT_CHAIN_CHECK_POS__(pos)                                     \
	if (pos < 0 || pos > _events.size()) {                                     \
		ESP_LOGE(__ESP_EVENT_CHAIN_DEBUG_TAG__,                                \
				 "Invalid index = %i - numEvents() = %i", pos,                 \
				 _events.size());                                              \
		panic();                                                               \
	}
#define __ESP_EVENT_CHAIN_CHECK_PTR__(ptr)                                     \
	if (ptr == nullptr) {                                                      \
		ESP_LOGE(__ESP_EVENT_CHAIN_DEBUG_TAG__, "Null pointer exception!");    \
		panic();                                                               \
	}
#define __ESP_EVENT_CHAIN_TRY_CALL__(f)                                        \
	if (!f) {                                                                  \
		size_t pos = std::distance(_events.begin(), _currentEvent);            \
		ESP_LOGW(__ESP_EVENT_CHAIN_DEBUG_TAG__,                                \
				 "Couldnt call event at pos = %i", pos);                       \
	} else {                                                                   \
		(f)();                                                                 \
	}

#include <Arduino.h>

#include <algorithm>
#include <functional>
#include <vector>
#include <iterator>
#include "EspDebug.h"
#include "EspEvent.h"
//#include "TickerHandler/EspTickerHandler.h"

/**
 *
 * Holds a collection of EspEvents and coordinates the periodic calls to each
 * event's callback
 *
 *
 */
class EspEventChain {
	friend class EspEventFindNextCallable;

  public:
	typedef std::vector<EspEvent> container_t;
	typedef container_t::const_iterator citerator_t;
	typedef container_t::iterator iterator_t;
	typedef EspEvent::callback_t callback_t;

  private:
	// The container of EspEvents and the corresponding iterators
	container_t _events;
	citerator_t _currentEvent;

	// TickerHandler tick;

	bool _started : 1;
	bool _runOnceFlag : 1;

  public:
	/**
	 * @brief Default constructor, nothing gets initialized
	 *
	 */
	EspEventChain();

	/**
	 * @brief Reserved space constructor, makes room for "num_events" events
	 *
	 * @param num_events The expected number of events, 0 <= num_events
	 *
	 */
	EspEventChain(size_t num_events);

	/**
	 * @brief Populate constructor, puts a variable number of event objects into
	 * the event chain
	 *
	 * @param ...events Comma separated EspEvent objects to put into the chain
	 *
	 */
	template <typename... Args>
	EspEventChain(EspEvent e1, Args... events) : _events({e1, events...}) {
		ESP_LOGI(__ESP_EVENT_CHAIN_DEBUG_TAG__,
				 "Constructing chain with %i events", sizeof...(events) + 1);
		construct();
	}

	/**
	 * @brief Destructor to ensure the chain is stopped when destroyed
	 *
	 * post: stop() called, isRunning() == false
	 */
	~EspEventChain() { stop(); }

	/**
	 * @brief Constructs an EspEvent using the supplied parameters at the end of
	 * the chain
	 *
	 * pre: Undefined behavior if the chain is running when called, may continue
	 * uninterrupted
	 *
	 * @tparam args Constructor arguments for EspEvent
	 *
	 * post: numEvents()++, event added to end of chain
	 *
	 */
	template <typename... Args> void emplace_back(Args... args) {
		_events.emplace_back(args...);
		ESP_LOGI(__ESP_EVENT_CHAIN_DEBUG_TAG__, "Event added to chain");
	}

	/**
	 * @brief Add an event to the end of the chain
	 *
	 * pre: Undefined behavior if the chain is running when called, may continue
	 * uninterrupted
	 *
	 * @param event The EspEvent object to add
	 *
	 * post: numEvents()++, event added to end of chain
	 *
	 */
	void push_back(const EspEvent &event);

	/**
	 * @brief Constructs an EspEvent using the supplied parameters at the the
	 * given position in the chain
	 *
	 * pre: Undefined behavior if the chain is running when called, may continue
	 * uninterrupted
	 *
	 * @param event_num		The position in the chain where the event should
	 * be constructed 0 <= event_num < numEvents();
	 * @tparam args 		Constructor arguments for EspEvent
	 *
	 * post: numEvents()++, event inserted at event_num, getTimeOf(event_num) =
	 * event.getTime()
	 *
	 */
	template <typename... Args> void emplace(size_t event_num, Args... args) {
		__ESP_EVENT_CHAIN_CHECK_POS__(event_num);
		auto emplace_target = _events.begin();
		std::advance(emplace_target, event_num);
		_events.emplace(emplace_target, args...);
		ESP_LOGI(__ESP_EVENT_CHAIN_DEBUG_TAG__, "Event added to chain");
	}

	/**
	 * @brief Add an event to the given position in the chain
	 *
	 * pre: Undefined behavior if the chain is running when called, may continue
	 * uninterrupted
	 *
	 * @param event_num		The position in the chain where the event should
	 * be constructed 0 <= event_num =< numEvents();
	 *
	 * @param event		The EspEvent object to add to the chain
	 *
	 * post: numEvents()++, event inserted at event_num, getTimeOf(event_num) =
	 * event.getTime()
	 *
	 */
	void insert(size_t event_num, const EspEvent &event);

	/**
	 * @brief Removes the event at the given position from the chain
	 *
	 * @param event_num		The position in the chain of the event to remove
	 * 						0 <= event_num < numEvents()
	 *
	 * post: numEvents()--
	 *
	 * @return The object that was removed
	 */
	EspEvent remove(size_t event_num);

	/**
	 * @brief Gets the number of events in the chain
	 *
	 * @return The number of events in the chain, 0 <= numEvents()
	 */
	size_t numEvents() const;

	/**
	 * @brief Change the time associated with the EspEvent at a given index
	 *
	 * @param pos           The position of the event to alter, 0 <= pos <
	 * numEvents()
	 * @param newTime_ms    The new time in milliseconds, 0 < newTime_ms
	 *
	 */
	void changeTimeOf(size_t pos, unsigned long newTime_ms);

	/**
	 * @brief Gets the time for the event at a given position
	 *
	 * @param pos The index, 0 <= pos < numEvents()
	 *
	 * @return _events.at(pos).getTime()
	 */
	unsigned long getTimeOf(size_t pos) const;

	/**
	 * @brief Attempts to look up an EspEvent in the chain using the identifying
	 * handle of the object
	 *
	 * @param handle    The handle to look up, handle != null && handle !=
	 * "null"
	 *
	 * @return  The position of the event with getHandle() == handle in the
	 * chain if it exists -1 if no match was found
	 */
	int getPositionFromHandle(const char *handle) const;

	/**
	 * @brief Attempts to look up an EspEvent in the chain using the identifying
	 * handle of the object
	 *
	 * @param handle    The handle to look up, handle != null && handle !=
	 * "null"
	 *
	 * @return  A const iterator of the event closest to begin() with
	 * getHandle()
	 * == handle if it exists container.cend() if no event was found with that
	 * handle
	 */
	citerator_t getIteratorFromHandle(const char *handle) const;

	/**
	 * @brief Starts the event chain from the beginning
	 *
	 * post:    _currentEvent positioned at the first event,
	 *          ticker armed to call first event, isRunning() == true
	 *
	 */
	void start();

	/**
	 * @brief Starts the event chain from the given event number
	 *
	 * @param event_num		The position in the chain to start from
	 * 						0 <= event_num < numEvents()
	 *
	 * post:    _currentEvent positioned at event_num,
	 *          ticker armed to call _currentEvent, isRunning() == true
	 *
	 */
	void startFrom(size_t event_num);

	/**
	 * @brief Runs the event chain from first to last one time
	 *
	 * post:    _currentEvent positioned at the first event,
	 *          ticker armed to call first event, isRunning() == true
	 *
	 */
	void runOnce();

	/**
	 * @brief Runs the event chain once starting from the given event number
	 *
	 * @param event_num		The position in the chain to start from
	 * 						0 <= event_num < numEvents()
	 *
	 * post: 	currentEvent positioned at event_num,
	 * 			ticker armed to call _currentEvent, isRunning() == true
	 *
	 */
	void runOnceStartFrom(size_t event_num);

	/**
	 * @brief Stops the event chain
	 *
	 * post: Ticker disarmed, isRunning() == false
	 *
	 */
	void stop();

	/**
	 * @brief Gets whether the event chain is running
	 *
	 * @return true if the chain is running, false otherwise
	 */
	bool isRunning() const { return _started; }

	/**
	 * @brief Gets the time required for the entire event chain to complete.
	 * Does not account for the time taken by the callbacks
	 *
	 * @return  The time in milliseconds for all events to run,
	 *          equivalent to getTime(0, numEvents() - 1)
	 */
	unsigned long getTotalTime() const;

	/**
	 * @brief Gets the time it will take for the first "index" events to run
	 *
	 * @param index The event to sum before, 0 < index < numEvents()
	 *
	 * @return Sum of getTimeOf() for events between 0 and index
	 */
	unsigned long getTotalTimeBefore(size_t index) const;

  private:
	void _start();

	/**
	 * @brief Constructor helper
	 *
	 * post: _runOnceFlag = false, _started = false
	 */
	void construct();

	/**
	 * @brief Checks whether the current event contains a callable callback
	 *
	 * @return true if valid, false otherwise
	 */
	bool validCurrentEvent() const;

	/**
	 * @brief Checks whether _currentEvent is positioned at the end of _events
	 *
	 * @return true if _currentEvent == _events.cend();
	 */
	bool atEndOfChain() const;

	/**
	 * @brief Checks whether the chain contains at least one EspEvent such that
	 * getTime() != 0
	 *
	 * @return true if getTime() != 0 for at least one event in the chain
	 */
	bool containsNonzeroEvent() const;

	/**
	 * @brief Advances the current event to the next event in the chain that is
	 * callable
	 *
	 * post:    _currentEvent > _currentEventOld if the next validCurrentEvent()
	 * follows _currentEventOld in the container _currentEvent =<
	 * _currentEventOld if there is no validCurrentEvent() between
	 * _currentEventOld and the end of the container
	 *
	 * @return
	 */
	bool advanceToNextCallable();

	/**
	 * @brief Handles each tick. Use this until we get std::function for ticker
	 *
	 * @param ptr   An EspEventChain object pointer cast to void*
	 *              ptr != null, ptr instanceof EspEventChain
	 *
	 * post: ptr cast to EspEventChain, handleTick called on casted object
	 */
	static void sHandleTick(void *ptr);

	/**
	 * @brief Member function called from handleTick that triggers the correct
	 * event
	 *
	 * post:    _currentEvent method called, _currentEvent == next valid event
	 * in chain, ticker armed to call _currentEvent
	 *
	 */
	void handleTick();

	/**
	 * @brief Sets the current event to the event at the given position in the
	 * event chain
	 *
	 * @param event_num		The position of an event in the chain,
	 * 						0 <= event_num < numEvents()
	 *
	 * post: _events.at(event_num) = _currentEvent
	 */
	void setCurrentEventTo(size_t event_num);
};

#endif