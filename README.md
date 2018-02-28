# EspEventChain

Provides class `EspEventChain`, an interface that simplifies scheduling of irregularly spaced `Ticker` events. Rather than relying on `once_ms` to schedule irregularly spaced events, they can be grouped under this data structure which will handle timing internally.

This class was created to handle the timing of events in [ESPLed](https://github.com/TidalPaladin/ESPLed), where blinking on and off needed to occur at unique times relative to the last change in LED state.

## Key Features
* **Track multiple events in one data structure** - 
	Multiple callbacks that would otherwise appear together as separate ticker events can now be stored inside of one data structure. Provide timing information and the callback, and `EspEventChain` will copy and store each event internally.


* **Track Event Timing Relative to Preceeding Event** - 
	Each event holds a callback method, and a time that should elapse between the preceeding event in the chain and this event before the callback is run. More details below.

* **Continuous Running / Starting, and Stopping** - 
	Start and stop methods allow for control of the event chain. Once started the chain will loop until stopped.

* **Single Ticker** - 
	A single intance of `Ticker` is used to coordinate events on ESP8266. On ESP32 no more than one thread will be created while the chain is running.


## Visualizing the Data Structure

It can be difficult to visualize the arrangement of events within the event chain, namely because each `EspEvent` stores timing data describing how long should elapse between the preceeding event and this event.

Consider the following event chain
```
[callback1; time = 1000] => [callback2; time = 20] => [callback3; time = 0]
```

`callback1` wil be run immediately upon starting the chain. After 20 milliseconds, `callback2` will run, followed immediately
by `callback3` because the third event has a time of 0. After 1000 milliseconds, the chain will loop back to the first event and call `callback1`. This repeats until `chain.stop()` is called.

## API

Documentation taken from `EspEventChain.h`.


### EspEvent

`EspEvent` is a data structure containing timing and callback data for one event. Objects of this type form the backbone of the
`EspEventChain`.

```c++
// Public callbacks provided by EspEvent
typedef std::function<void()> callback_t;
```

```c++
/**
 * @brief Default constructor
 * 
 * post: callback is empty, time = 0
 * 
 */
EspEvent();
```

```c++
/**
 * @brief Constructor
 * 
 * @param relative_time_ms      The delay in milliseconds between the preceeding event and this event
 *                              0 < relative_time_ms
 * 
 * @param event                 The void() callback to run when the event is triggered
 *                              event takes no parameters and returns void
 * 
 * @param identifying_handle    A text handle to identify this event as part of the chain
 */
EspEvent(unsigned long relative_time_ms, callback_t event, const char* identifying_handle = "null");
```

```c++
/**
 * @brief Tests whether the event stores a callable function
 * 
 * @return true if a callable function is contained, false otherwise
 */
operator bool() const;
```

```c++
/**
 * @brief Sets the time for this event relative to the event that will preceed it in
 * the EspEventChain
 * 
 * @param ms    The time in milliseconds, 0 <= ms
 *              ms = 0 for an event that will run immediately after the one preceeding it
 * 
 * @return this
 */
EspEvent &setTime(unsigned long ms);
```

```c++

/**
 * @brief Sets the time for this event relative to the event that will preceed it in
 * the EspEventChain
 * 
 * @param ms The time in milliseconds, 0 < ms
 * 
 * @return this
 */
EspEvent &setCallback(callback_t &callback);
```

```c++
/**
 * @brief Gets the time property for this Event
 * 
 * @return Time in milliseconds, 0 < newTime_ms
 */
unsigned long getTime() const;
```

```c++
/**
 * @brief Gets the text handle for this event for the purpose of identification
 * 
 * @return "null" if no handle was set, or identifying_handle assigned at construction
 */
const char* getHandle() const;
```

```c++
/**
 * @brief Runs the callback for this event
 * 
 * @return The time in milliseconds that the callback took to run
 */
unsigned long runEvent() const;

```



### EspEventChain

```c++
// Public typedefs provided by EspEventChain
typedef std::vector<EspEvent> container_t;
typedef container_t::const_iterator citerator_t;
typedef container_t::iterator iterator_t;
typedef EspEvent::callback_t callback_t;
```

```c++

/**
 * @brief Default constructor, nothing gets initialized
 * 
 */
EspEventChain();
```

```c++
/**
 * @brief Reserved space constructor, makes room for "num_events" events
 * 
 * @param num_events The expected number of events, 0 <= num_events
 * 
 */
EspEventChain(size_t num_events);
```

```c++

/**
 * @brief Populate constructor, puts a variable number of event objects into the event chain
 * 
 * @param ...events Comma separated EspEvent objects to put into the chain
 * 
 */
template<typename... Args>
EspEventChain(EspEvent e1, Args... events);
```

```c++
/**
 * @brief Add an event to the end of the chain
 * 
 * @details Constructs an event at the end of the chain via perfect forwarding of args
 * 
 * @tparam args Constructor arguments for EspEvent
 * 
 * @return numEvents() - 1, ie the position of the new event in the chain
 */
template<typename... Args>
size_t addEvent(Args... args);
```

```c++
/**
 * @brief Add an event to the end of the chain
 * 
 * @param event The EspEvent object to add
 * 
 * @return numEvents() - 1, ie the position of the new event in the chain
 */
size_t addEvent(EspEvent event);
```

```c++

/**
 * @brief Gets the number of events in the chain
 * 
 * @return 0 <= numEvents()
 */
size_t numEvents() const;
```

```c++

/**
 * @brief Change the time associated with the EspEvent at a given index
 * 
 * @param pos           The position of the event to alter, 0 <= pos < numEvents()
 * @param newTime_ms    The new time in milliseconds, 0 < newTime_ms
 * 
 */
void changeTimeOf(size_t pos, unsigned long newTime_ms);
```

```c++
/**
 * @brief Gets the time for the event at a given position
 * 
 * @param pos The index, 0 <= pos < numEvents()
 * 
 * @return _events.at(pos).getTime()
 */
unsigned long getTimeOf(size_t pos) const;
```

```c++
/**
 * @brief Attempts to look up an EspEvent in the chain using the identifying handle of the
 * object
 * 
 * @param handle    The handle to look up, handle != null && handle != "null"
 * 
 * @return  The position of the event with getHandle() == handle in the chain if it exists
 *          -1 if no match was found
 */
int getPositionFromHandle(const char* handle) const;
```

```c++

/**
 * @brief Attempts to look up an EspEvent in the chain using the identifying handle of the
 * object
 * 
 * @param handle    The handle to look up, handle != null && handle != "null"
 * 
 * @return  A const iterator of the event closest to begin() with getHandle() == handle if it exists
 *          container.cend() if no event was found with that handle
 */
citerator_t getIteratorFromHandle(const char* handle) const;
```

```c++
/**
 * @brief Starts the event chain
 * 
 * post:    reset() called, _currentEvent positioned at the first event,
 *          ticker armed to call first event, running() == true
 * 
 */
void start();
```

```c++

/**
 * @brief Stops the event chain
 * 
 * post: Ticker disarmed, running() == false
 * 
 */
void stop();
```

```c++
/**
 * @brief Gets whether the event chain is running
 * 
 * @return true if the chain is running, false otherwise
 */
bool running() const;
```

```c++

/**
 * @brief Gets the time required for the entire event chain to complete. Does not
 * account for the time taken by the callbacks
 * 
 * @return  The time in milliseconds for all events to run, 
 *          equivalent to getTime(0, numEvents() - 1)
 */
unsigned long totalTime() const;
```

```c++
/**
 * @brief Gets the time it will take for the first "index" events to run
 * 
 * @param index The event to sum before, 0 < index < numEvents()
 *
 * @return Sum of getTimeOf() for events between 0 and index
 */
unsigned long totalTimeBefore(size_t index) const;
```


