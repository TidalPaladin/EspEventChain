#include <Arduino.h>
#include <Arduino.h>
#include "EspDebug.h"
#include "EspEventChain.h"

#ifndef UNIT_TEST

uint32_t long_event_length = 100000;
const EspEvent::callback_t LONG_CALLBACK = [&]() {
	for (int i = 0; i < ::long_event_length; i++) {
		ESP.wdtFeed();
	}
};

void testLongDelay(uint32_t ms) {
#ifdef ESP32
	vTaskDelay(pdMS_TO_TICKS(ms));
#else
	delay(ms);
#endif
}

void testYield() {
#ifndef ESP32
	yield();
#else
	taskYIELD();
#endif
}

bool eventChainTickTimeHelper(uint32_t t1, uint32_t t2) {
	const uint32_t DELAY_MARGIN_ERR = 2;
	unsigned long wait = 3 * (t1 + t2);
	unsigned long count = 0;

	unsigned long lastEvent;
	unsigned long elapsedTime;

	EspEvent e1(t1, [&]() {
		unsigned long current_time = count > 0 ? millis() : lastEvent;
		unsigned long elapsed_time = current_time - lastEvent;
		TEST_ASSERT_INT_WITHIN(5, t1, elapsed_time);
		count++;
		lastEvent = current_time;
	});

	EspEvent e2(t2, [&]() {
		unsigned long current_time = millis();
		unsigned long elapsed_time = current_time - lastEvent;
		TEST_ASSERT_INT_WITHIN(5, t2, elapsed_time);
		count++;
		lastEvent = current_time;
	});
	EspEventChain chain(e1, e2);

	Serial.printf("Tick1: %i, Tick2: %i\n", t1, t2);

	lastEvent = millis();
	chain.start();
	delay(wait + DELAY_MARGIN_ERR);
	chain.stop();
	return true;
}

bool eventChainTest1() {
	const uint8_t fixed_size = 10;
	std::function<void()> f = []() {

	};
	unsigned long t1 = 20, t2 = 30;

	EspEventChain chain1;
	EspEventChain chain2(100);
	EspEventChain chain3(EspEvent(t1, f), EspEvent(t2, f));

	TEST_ASSERT_EQUAL(0, chain1.numEvents());
	TEST_ASSERT_EQUAL(0, chain2.numEvents());
	TEST_ASSERT_EQUAL(2, chain3.numEvents());

	TEST_ASSERT_EQUAL(t1, chain3.getTimeOf(0));
	TEST_ASSERT_EQUAL(t2, chain3.getTimeOf(1));

	return true;
}

bool eventChainTest2() {

	unsigned long t1 = 200;
	unsigned long t2 = 75;

	EspEvent e1(t1, [&]() {});
	EspEvent e2(t2, [&]() {});
	EspEventChain chain(e1, e2);

	TEST_ASSERT_EQUAL(2, (int)chain.numEvents());
	TEST_ASSERT_EQUAL(t1 + t2, chain.totalTime());
	TEST_ASSERT_EQUAL(t1, chain.totalTimeBefore(1));

	chain.push_back(e1);
	TEST_ASSERT_EQUAL(3, chain.numEvents());
	TEST_ASSERT_EQUAL(2 * t1 + t2, chain.totalTime());
	TEST_ASSERT_EQUAL(t1, chain.getTimeOf(2));

	return true;
}

bool eventChainTest2s5() {

	EspEvent e1(50, [&]() { Serial.println("Tick"); });
	EspEventChain chain(e1);

	chain.start();
	delay(2000);
	chain.stop();

	return true;
}

bool eventChainTest3() {
	unsigned long t1 = 100, t2 = 75, wait = (t1 + t2) * 3;
	eventChainTickTimeHelper(test, t1, t2);
	return true;
}

bool eventChainTest4() {

	bool ticked = false;
	unsigned long wait = 30;

	EspEvent *e1 = new EspEvent(30, [&]() {
		ticked = true;
		Serial.println("Tick from deleted object, copy was successful!");
	});

	EspEventChain chain(*e1);
	delete e1;
	chain.start();
	delay(2 * wait);

	Serial.println(
		"If we made it this far without an exception, copy was made");
	TEST_ASSERT_EQUAL(true, ticked);
	return true;
}

bool eventChainTest5() {
	unsigned long t1 = 0, t2 = 420;
	eventChainTickTimeHelper(test, t1, t2);
	return true;
}

bool eventChainTest6() {
	unsigned long t1 = 0, t2 = 30;
	eventChainTickTimeHelper(test, t1, t2);
	return true;
}

bool eventChainTest7() {
	unsigned long t1 = 0, t2 = 10;

	EspEvent e1(t1, []() {

	});
	EspEvent e2(t2, []() {

	});

	EspEventChain chain;
	chain.push_back(e1);
	chain.insert(0, e2);
	chain.push_back(e1);
	chain.insert(chain.numEvents() - 1, e2);

	const unsigned long EXPECTED_TIMES[] = {t2, t1, t2, t1};
	const size_t TEST_SIZE = sizeof(EXPECTED_TIMES) / sizeof(EXPECTED_TIMES[0]);

	for (size_t i = 0; i < TEST_SIZE; i++) {
		TEST_ASSERT_EQUAL(EXPECTED_TIMES[i], chain.getTimeOf(i));
	}

	return true;
}

bool eventChainTest8() {
	unsigned long t1 = 0, t2 = 10;

	std::function<void()> dummy = []() {

	};

	EspEventChain chain;
	chain.emplace_back(t1, dummy);
	chain.emplace(0, t2, dummy);
	chain.emplace_back(t1, dummy);
	chain.emplace(chain.numEvents() - 1, t2, dummy);

	const unsigned long EXPECTED_TIMES[] = {t2, t1, t2, t1};
	const size_t TEST_SIZE = sizeof(EXPECTED_TIMES) / sizeof(EXPECTED_TIMES[0]);

	for (size_t i = 0; i < TEST_SIZE; i++) {
		TEST_ASSERT_EQUAL(EXPECTED_TIMES[i], chain.getTimeOf(i));
	}

	return true;
}

bool eventChainTest9() {
	unsigned long t1 = 0, t2 = 10;

	EspEvent e1(t1, []() {

	});
	EspEvent e2(t2, []() {

	});

	EspEventChain chain(e1, e2, e1, e1, e2, e2);

	// e1 -> e1 -> e2, e1 e2, e2
	const size_t REMOVE_POSITIONS[] = {2, 0, 3, 1, 1, 0};
	const unsigned long EXPECTED_TIMES[] = {t1, t1, t2, t1, t2, t2};
	const size_t TEST_SIZE = sizeof(EXPECTED_TIMES) / sizeof(EXPECTED_TIMES[0]);

	for (size_t i = 0; i < TEST_SIZE; i++) {
		EspEvent removed_event = chain.remove(REMOVE_POSITIONS[i]);
		TEST_ASSERT_EQUAL(EXPECTED_TIMES[i], removed_event.getTime());
		TEST_ASSERT_EQUAL(TEST_SIZE - i - 1, chain.numEvents());
	}

	return true;
}

bool eventChainTest10p1() {
	unsigned long t1 = 15, t2 = 10;

	unsigned long count = 0;

	EspEvent e1(t1, [&]() { count++; });
	EspEvent e2(t2, [&]() { count++; });

	EspEventChain chain(e1, e2, e1, e1, e2, e2);
	const unsigned long EXPECTED_COUNT = chain.numEvents();
	chain.runOnce();

	delay(chain.totalTime() * 3);
	TEST_ASSERT_EQUAL(EXPECTED_COUNT, count);
	return true;
}

bool eventChainTest10() {
	unsigned long t1 = 15, t2 = 10;

	unsigned long count = 0;

	EspEvent e1(t1, [&]() { count += 2; });
	EspEvent e2(t2, [&]() { count += 3; });

	EspEventChain chain(e1, e2, e1, e1, e2, e2);
	const size_t START_FROM = 2;
	const unsigned long EXPECTED_COUNT = 2 + 2 + 3 + 3;
	chain.runOnceStartFrom(START_FROM);

	delay(chain.totalTime() * 3);
	TEST_ASSERT_EQUAL(EXPECTED_COUNT, count);
	return true;
}

bool eventChainTest14() {
	unsigned long t1 = 5, t2 = 5;

	const uint32_t LOOP_STOP = F_CPU / 1000 * t1;

	unsigned long count = 0;

	EspEvent e1(t1, LONG_CALLBACK);
	EspEvent e2(t2, LONG_CALLBACK);

	EspEventChain chain(e1, e2);

	chain.start();
	testLongDelay((t1 + t2) * 500);
	chain.stop();
	return true;
}

bool eventChainTest11() {
	Serial.println("This will take a little");
	unsigned long t1 = 5, t2 = 5;

	unsigned long count = 0;

	EspEvent e1(t1, [&]() { count += 2; });
	EspEvent e2(t2, [&]() { count += 3; });

	EspEventChain chain(e1, e2, e1, e1, e2, e2);

	chain.start();
	testLongDelay(chain.totalTime() * 100);
	chain.stop();

	return true;
}

void setup() {
	// NOTE!!! Wait for >2 secs
	// if board doesn't support software reset via Serial.DTR/RTS
	delay(2000);

	UNITY_BEGIN();
	RUN_TEST(eventChainTest1);
	RUN_TEST(eventChainTest2);
	RUN_TEST(eventChainTest2s5);
	RUN_TEST(eventChainTest3);
	UNITY_END();

	// eventChainTest1();
	// eventChainTest2();
	// eventChainTest2s5();
	// eventChainTest3();
	// eventChainTest4();
	// eventChainTest5();
	// eventChainTest6();
	// eventChainTest7();
	// eventChainTest8();
	// eventChainTest9();
	// eventChainTest10p1();
	// eventChainTest10();
	// eventChainTest14();
	// eventChainTest11();

	// TestHelper::end();
}

void loop() {
	digitalWrite(13, HIGH);
	delay(100);
	digitalWrite(13, LOW);
	delay(500);
}

#endif