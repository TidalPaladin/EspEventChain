#include <Arduino.h>
#include "EspEventChain.h"
#include "unity.h"

#ifdef UNIT_TEST

void eventChainTickTimeHelper(unsigned long t1, unsigned long t2) {
	const unsigned long DELAY_MARGIN_ERR = 2;
	const unsigned int NUM_LOOPS = 3;
	unsigned long wait = NUM_LOOPS * (t1 + t2) + 10;
	unsigned long count = 0;

	std::function<void()> f = [&]() {
		static unsigned long lastEvent = millis();
		unsigned long current_time = count > 0 ? millis() : lastEvent;
		unsigned long elapsed_time = current_time - lastEvent;

		unsigned long expected_time = (count % 2 == 0 ? t1 : t2);
		if (count == 0) expected_time = 0;
		TEST_ASSERT_INT_WITHIN(DELAY_MARGIN_ERR, expected_time, elapsed_time);
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

	unsigned long expected_ticks =
		chain.getTotalTime() * NUM_LOOPS / (t1 + t2) * 2 + 1;
	TEST_ASSERT_INT_WITHIN_MESSAGE(1, expected_ticks, count,
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

void run_once_start_from() {
	bool tickedOnce = true;
	int count = 0;
	EspEvent e1(20, [&]() {
		count++;
		tickedOnce = false;
	});
	EspEvent e2(20, [&]() { count++; });
	EspEventChain chain(e1, e1, e1, e2, e2);

	// Start from event index 3, tickedOnce should stay false
	chain.runOnceStartFrom(3);
	delay(chain.getTotalTime() * 3);
	chain.stop();

	TEST_ASSERT_EQUAL_MESSAGE(2, count, "Correct number of ticks");
	TEST_ASSERT_EQUAL_MESSAGE(true, tickedOnce,
							  "Starting from correct position");
}

void run_once() {
	int count = 0;
	EspEvent e1(20, [&]() { count++; });
	EspEvent e2(20, [&]() { count++; });
	EspEventChain chain(e1, e1, e1, e2, e2);

	// Start from event index 3, tickedOnce should stay false
	chain.runOnce();
	delay(chain.getTotalTime() * 3);
	chain.stop();

	TEST_ASSERT_EQUAL_MESSAGE(5, count, "Correct number of ticks");
}

void setup() {
	delay(2000);
	UNITY_BEGIN();
	RUN_TEST(simple_tick);
	RUN_TEST(complex_tick);
	RUN_TEST(run_once_start_from);
	RUN_TEST(run_once);

	UNITY_END();
}

void loop() {}

#endif