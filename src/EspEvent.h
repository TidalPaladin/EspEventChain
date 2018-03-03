/**
 * @file EspEvent.h
 * @author Scott Chase Waggener tidal@utexas.edu
 * @date 2/2/18
 * 
 * @description
 * Tracks a timing / callback pair for EspEventChain
 * 
 *
 * 
 */ 

#ifndef __ESP_EVENT_H__
#define __ESP_EVENT_H__

#include <Arduino.h>
#include <vector>
#include <functional>
#include <algorithm>


#ifndef ESP32
#include <Ticker.h>
#endif

/**
 * 
 * Data structure representing one event. Holds data about the callback method to run and the tick delay
 * relative to the preceeding event
 * 
 */
class EspEvent {

	public:

		typedef std::function<void()> callback_t;

    private:

		const char* _HANDLE;
		unsigned long _time_ms;
		callback_t _callback;

		

	public:

		/**
		 * @brief Default constructor
		 * 
		 * post: callback is empty, time = 0
		 * 
		 */
		EspEvent();

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

		/**
		 * @brief Tests whether the event stores a callable function
		 * 
		 * @return true if a callable function is contained, false otherwise
		 */
		operator bool() const;

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


		/**
		 * @brief Sets the time for this event relative to the event that will preceed it in
		 * the EspEventChain
		 * 
		 * @param ms The time in milliseconds, 0 < ms
		 * 
		 * @return this
		 */
		EspEvent &setCallback(callback_t &callback);

		/**
		 * @brief Gets the time property for this Event
		 * 
		 * @return Time in milliseconds, 0 < newTime_ms
		 */
		unsigned long getTime() const;

		/**
		 * @brief Gets the text handle for this event for the purpose of identification
		 * 
		 * @return "null" if no handle was set, or identifying_handle assigned at construction
		 */
		const char* getHandle() const;

		/**
		 * @brief Runs the callback for this event
		 * 
		 */
		void runEvent() const;

};

#endif