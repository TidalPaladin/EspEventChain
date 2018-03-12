#include <Arduino.h>
#include "EspDebug.h"
#include "EspEventChain.h"
#include "unity.h"

#ifdef UNIT_TEST

void eventChainTickTimeHelper(unsigned long t1, unsigned long t2) {
	const unsigned long DELAY_MARGIN_ERR = 2;
	final unsigned int NUM_LOOPS = 3;
	unsigned long wait = NUM_LOOPS * (t1 + t2) + 10;
	unsigned long count = 0;

	std::function<void()> f = [&]() {
		static unsigned long lastEvent = millis();
		unsigned long current_time = count > 0 ? millis() : lastEvent;
		unsigned long elapsed_time = current_time - lastEvent;
		TEST_ASSERT_INT_WITHIN(DELAY_MARGIN_ERR, t1, elapsed_time);
		count++;
		lastEvent = current_time;
	};

	EspEvent e1(t1, f);
	EspEvent e2(t2, f);
	EspEventChain chain(e1, e2);

	TEST_ASSERT_EQUAL_MESSAGE(t1, chain.getTimeOf(0),
							  "getTimeOf() before test");
	TEST_ASSERT_EQUAL_MESSAGE(t2, chain.getTimeOf(1),
							  "getTimeOf() before test");

	chain.start();
	TEST_ASSERT_EQUAL_MESSAGE(true, chain.isRunning(),
							  "isRunning() == true after start()");

	delay(wait);
	chain.stop();
	TEST_ASSERT_EQUAL_MESSAGE(false, chain.isRunning(),
							  "isRunning() == false after stop()");
	TEST_ASSERT_INT_WITHIN_MESSAGE(1, chain.getTotalTime() * NUM_LOOPSf, count,
								   "number of ticks on stop");
}

void simple_tick() {

	bool tickedOnce = false;
	EspEvent e1(50, [&]() { tickedOnce = true; });
	EspEventChain chain(e1);

	chain.start();
	delay(300);
	chain.stop();

	TEST_ASSERT_EQUAL_MESSAGE(true, tickedOnce, "No tick!");
}

void complex_tick() { eventChainTickTimeHelper(100, 200); }

void setup() {
	delay(2000);
	UNITY_BEGIN();
	RUN_TEST(empty_initial_values);
	RUN_TEST(filled_initial_values);

	UNITY_END();
}

void loop() {}

#endif