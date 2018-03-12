
#include <Arduino.h>
#include "EspDebug.h"
#include "EspEventChain.h"
#include "TestHelper.h"

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

void eventChainTickTimeHelper(TestHelper &test, uint32_t t1, uint32_t t2) {
	const uint32_t DELAY_MARGIN_ERR = 2;
	unsigned long wait = 3 * (t1 + t2);
	unsigned long count = 0;

	unsigned long lastEvent;
	unsigned long elapsedTime;

	EspEvent e1(t1, [&]() {
		unsigned long current_time = count > 0 ? millis() : lastEvent;
		unsigned long elapsed_time = current_time - lastEvent;
		test.printResultRange(t1, elapsed_time, 5);
		count++;
		lastEvent = current_time;
	});

	EspEvent e2(t2, [&]() {
		unsigned long current_time = millis();
		unsigned long elapsed_time = current_time - lastEvent;
		test.printResultRange(t2, elapsed_time, 5);
		count++;
		lastEvent = current_time;
	});
	EspEventChain chain(e1, e2);

	Serial.printf("Tick1: %i, Tick2: %i\n", t1, t2);

	lastEvent = millis();
	chain.start();
	delay(wait + DELAY_MARGIN_ERR);
	chain.stop();
}

bool eventChainTest1() {
	TestHelper test("EspEventChain", "constructors");

	const uint8_t fixed_size = 10;
	std::function<void()> f = []() {

	};
	unsigned long t1 = 20, t2 = 30;

	EspEventChain chain1;
	EspEventChain chain2(100);
	EspEventChain chain3(EspEvent(t1, f), EspEvent(t2, f));

	test.printResult(0, chain1.numEvents());
	test.printResult(0, chain2.numEvents());
	test.printResult(2, chain3.numEvents());

	test.printResult(t1, chain3.getTimeOf(0));
	test.printResult(t2, chain3.getTimeOf(1));

	return test.printResult();
}

bool eventChainTest2() {
	TestHelper test("EspEventChain", "getters");

	unsigned long t1 = 200;
	unsigned long t2 = 75;

	EspEvent e1(t1, [&]() {});
	EspEvent e2(t2, [&]() {});
	EspEventChain chain(e1, e2);

	test.printResult(2, (int)chain.numEvents());
	test.printResult(t1 + t2, chain.getTotalTime());
	test.printResult(t1, chain.getTotalTimeBefore(1));

	chain.push_back(e1);
	test.printResult(3, chain.numEvents());
	test.printResult(2 * t1 + t2, chain.getTotalTime());
	test.printResult(t1, chain.getTimeOf(2));

	return test.printResult();
}

bool eventChainTest2s5() {
	TestHelper test("EspEventChain", "sipmlest tick test");

	EspEvent e1(50, [&]() { Serial.println("Tick"); });
	EspEventChain chain(e1);

	chain.start();
	delay(2000);
	chain.stop();

	return test.printResult();
}

bool eventChainTest3() {
	TestHelper test("EspEventChain", "Ticking properly");
	unsigned long t1 = 100, t2 = 75, wait = (t1 + t2) * 3;
	eventChainTickTimeHelper(test, t1, t2);
	return test.printResult();
}

bool eventChainTest4() {
	TestHelper test("ESPEventChain()", "events stored by copy, not reference");

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
	test.printResult(true, ticked);
	return test.printResult();
}

bool eventChainTest5() {
	TestHelper test("ESPEventChain()", "time=0 events");
	unsigned long t1 = 0, t2 = 420;
	eventChainTickTimeHelper(test, t1, t2);
	return test.printResult();
}

bool eventChainTest6() {
	TestHelper test("ESPEventChain()", "speed test");
	unsigned long t1 = 0, t2 = 30;
	eventChainTickTimeHelper(test, t1, t2);
	return test.printResult();
}

bool eventChainTest7() {
	TestHelper test("insert() and push_back()", "fixed test");
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
		test.printResult(EXPECTED_TIMES[i], chain.getTimeOf(i));
	}

	return test.printResult();
}

bool eventChainTest8() {
	TestHelper test("emplace() and emplace_back()", "fixed test");
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
		test.printResult(EXPECTED_TIMES[i], chain.getTimeOf(i));
	}

	return test.printResult();
}

bool eventChainTest9() {
	TestHelper test("remove()", "fixed test");
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
		test.printResult(EXPECTED_TIMES[i], removed_event.getTime());
		test.printResult(TEST_SIZE - i - 1, chain.numEvents());
	}

	return test.printResult();
}

bool eventChainTest10p1() {
	TestHelper test("runOnce()", "fixed test");
	unsigned long t1 = 15, t2 = 10;

	unsigned long count = 0;

	EspEvent e1(t1, [&]() { count++; });
	EspEvent e2(t2, [&]() { count++; });

	EspEventChain chain(e1, e2, e1, e1, e2, e2);
	const unsigned long EXPECTED_COUNT = chain.numEvents();
	chain.runOnce();

	delay(chain.getTotalTime() * 3);
	test.printResult(EXPECTED_COUNT, count);
	return test.printResult();
}

bool eventChainTest10() {
	TestHelper test("runOnceStartFrom()", "fixed test");
	unsigned long t1 = 15, t2 = 10;

	unsigned long count = 0;

	EspEvent e1(t1, [&]() { count += 2; });
	EspEvent e2(t2, [&]() { count += 3; });

	EspEventChain chain(e1, e2, e1, e1, e2, e2);
	const size_t START_FROM = 2;
	const unsigned long EXPECTED_COUNT = 2 + 2 + 3 + 3;
	chain.runOnceStartFrom(START_FROM);

	delay(chain.getTotalTime() * 3);
	test.printResult(EXPECTED_COUNT, count);
	return test.printResult();
}

bool eventChainTest14() {
	TestHelper test("start()", "overlapping timings test");
	unsigned long t1 = 5, t2 = 5;

	const uint32_t LOOP_STOP = F_CPU / 1000 * t1;

	unsigned long count = 0;

	EspEvent e1(t1, LONG_CALLBACK);
	EspEvent e2(t2, LONG_CALLBACK);

	EspEventChain chain(e1, e2);

	chain.start();
	testLongDelay((t1 + t2) * 500);
	chain.stop();
	return test.printResult();
}

bool eventChainTest11() {
	TestHelper test("start()", "fast ticking");
	Serial.println("This will take a little");
	unsigned long t1 = 5, t2 = 5;

	unsigned long count = 0;

	EspEvent e1(t1, [&]() { count += 2; });
	EspEvent e2(t2, [&]() { count += 3; });

	EspEventChain chain(e1, e2, e1, e1, e2, e2);

	chain.start();
	testLongDelay(chain.getTotalTime() * 100);
	chain.stop();

	return test.printResult();
}

void setup() {
	Serial.begin(921600);
	Serial.println("\n\n");
	Serial.println("Beginning tests\n");
	delay(1000);

	eventChainTest1();
	eventChainTest2();
	eventChainTest2s5();
	eventChainTest3();
	eventChainTest4();
	eventChainTest5();
	eventChainTest6();
	eventChainTest7();
	eventChainTest8();
	eventChainTest9();
	eventChainTest10p1();
	eventChainTest10();
	eventChainTest14();
	eventChainTest11();

	TestHelper::end();
}

void loop() { delay(10); }