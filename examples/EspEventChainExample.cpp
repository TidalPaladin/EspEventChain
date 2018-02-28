#include <Arduino.h>
#include "EspEventChain.h"





void runEventChain() {
		Serial.println("About to start event chain");

	/* The times for the first and second event */
	unsigned long t1 = 100, t2 = 75;


	unsigned long wait = 3*(t1+t2) + 2;	
	unsigned long lastEvent;
	unsigned long elapsedTime;

	/**
	 * Create 3 events that print time information
	 * 
	 */

	EspEvent e1(t1, [&]() {
		unsigned long current_time = millis();
		unsigned long elapsed_time = current_time - lastEvent;
		Serial.printf("Tick: %i\n", elapsed_time);
		lastEvent = current_time;
	});

	EspEvent e2(t2, [&]() {
		unsigned long current_time = millis();
		unsigned long elapsed_time = current_time - lastEvent;
		Serial.printf("Tock: %i\n", elapsed_time);
		lastEvent = current_time;
	});

	EspEvent e3(0, [&]() {
		unsigned long current_time = millis();
		unsigned long elapsed_time = current_time - lastEvent;
		Serial.printf("Zero: %i\n", elapsed_time);
		lastEvent = current_time;
	});


	/* Create an event chain composed of the events we created */
	EspEventChain chain(e1, e2, e3);

	/* Start the chain */
	lastEvent = millis();
	chain.start();
	delay( wait );
	chain.stop();
	Serial.println("Stop");
}











void setup() {
	Serial.begin(115200);
	Serial.println("\n\n");
	delay(1000);

	runEventChain();
}


void loop() {
	delay(1);
}