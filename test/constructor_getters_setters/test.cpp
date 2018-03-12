#include <Arduino.h>
#include <Arduino.h>
#include "EspDebug.h"
#include "EspEventChain.h"
#include "unity.h"

#ifdef UNIT_TEST

unsigned long t1 = 10;

EspEvent e1(t1, [&]() {});
EspEvent e2(t1 * 2, [&]() {});
EspEvent e3(t1 * 3, [&]() {});
EspEvent e4(t1 * 4, [&]() {});

EspEventChain chain1(e1);
EspEventChain chain2(e1, e2);
EspEventChain chain3(e1, e2, e3);
EspEventChain chain4(e1, e2, e3, e4);
EspEventChain cases[] = {chain1, chain2, chain3, chain4};

void empty_initial_values() {
	const uint8_t fixed_size = 100;
	EspEventChain chain1;
	EspEventChain chain2(fixed_size);

	TEST_ASSERT_EQUAL_MESSAGE(0, chain1.numEvents(),
							  "Initial numEvents() != 0 at construction");
	TEST_ASSERT_EQUAL_MESSAGE(0, chain2.numEvents(),
							  "Initial numEvents() != 0 at construction");
	;
}

void filled_initial_values() {
	std::function<void()> f = []() {};
	const unsigned long EVENT_TIME = 10;
	EspEventChain chain1(EspEvent(EVENT_TIME, f), EspEvent(EVENT_TIME, f));
	const uint32_t EVENTS_IN_CHAIN = 2;

	TEST_ASSERT_EQUAL_MESSAGE(
		EVENTS_IN_CHAIN, chain1.numEvents(),
		"Initial numEvents() not correct at construction of populated list");

	TEST_ASSERT_EQUAL_MESSAGE(EVENTS_IN_CHAIN * EVENT_TIME,
							  chain1.getTotalTime(),
							  "getTotalTime() != 0 at construction");
}

void numEvents() {
	TEST_ASSERT_EQUAL_MESSAGE(1, chain1.numEvents(), "Failed for event 1");
	TEST_ASSERT_EQUAL_MESSAGE(2, chain2.numEvents(), "Failed for event 2");
	TEST_ASSERT_EQUAL_MESSAGE(3, chain3.numEvents(), "Failed for event 3");
	TEST_ASSERT_EQUAL_MESSAGE(4, chain4.numEvents(), "Failed for event 4");
}

void getTotalTime() {
	TEST_ASSERT_EQUAL_MESSAGE(10, chain1.getTotalTime(), "Failed for event 1");
	TEST_ASSERT_EQUAL_MESSAGE(30, chain2.getTotalTime(), "Failed for event 2");
	TEST_ASSERT_EQUAL_MESSAGE(60, chain3.getTotalTime(), "Failed for event 3");
	TEST_ASSERT_EQUAL_MESSAGE(100, chain4.getTotalTime(), "Failed for event 4");
}

void getTotalTimeBeforeLast() {
	TEST_ASSERT_EQUAL_MESSAGE(10, chain1.getTotalTimeBefore(chain1.numEvents()),
							  "Failed for event 1");
	TEST_ASSERT_EQUAL_MESSAGE(30, chain2.getTotalTimeBefore(chain2.numEvents()),
							  "Failed for event 2");
	TEST_ASSERT_EQUAL_MESSAGE(60, chain3.getTotalTimeBefore(chain3.numEvents()),
							  "Failed for event 3");
	TEST_ASSERT_EQUAL_MESSAGE(100,
							  chain4.getTotalTimeBefore(chain4.numEvents()),
							  "Failed for event 4");
}

void getTotalTimeBeforeFirst() {
	const char *msg = "getTotalTimeBefore(0)";

	TEST_ASSERT_EQUAL_MESSAGE(0, chain1.getTotalTimeBefore(0), msg);
	TEST_ASSERT_EQUAL_MESSAGE(0, chain2.getTotalTimeBefore(0), msg);
	TEST_ASSERT_EQUAL_MESSAGE(0, chain3.getTotalTimeBefore(0), msg);
	TEST_ASSERT_EQUAL_MESSAGE(0, chain4.getTotalTimeBefore(0), msg);
}

void getTotalTimeBefore() {
	const char *msg = "getTotalTimeBefore( numEvents() - 1 )";

	TEST_ASSERT_EQUAL_MESSAGE(10, chain2.getTotalTimeBefore(1), msg);
	TEST_ASSERT_EQUAL_MESSAGE(30, chain3.getTotalTimeBefore(2), msg);
	TEST_ASSERT_EQUAL_MESSAGE(60, chain4.getTotalTimeBefore(3), msg);
}

void getTimeOf() {
	const char *msg = "getTimeOf() on last event";
	TEST_ASSERT_EQUAL_MESSAGE(10, chain1.getTimeOf(0), msg);
	TEST_ASSERT_EQUAL_MESSAGE(20, chain2.getTimeOf(1), msg);
	TEST_ASSERT_EQUAL_MESSAGE(30, chain3.getTimeOf(2), msg);
	TEST_ASSERT_EQUAL_MESSAGE(40, chain4.getTimeOf(3), msg);
}

void isRunning() {
	TEST_ASSERT_EQUAL_MESSAGE(false, chain1.isRunning(),
							  "Not running before start()");
	chain1.start();
	TEST_ASSERT_EQUAL_MESSAGE(true, chain1.isRunning(),
							  "Running after start()");
	chain1.stop();
	delay(25);
}

void emplace_back() {

	EspEventChain chain1c(e1);
	EspEventChain chain2c(e1, e2);
	EspEventChain chain3c(e1, e2, e3);
	EspEventChain chain4c(e1, e2, e3, e4);

	chain1c.emplace_back(t1, []() {});
	chain2c.emplace_back(t1, []() {});
	chain3c.emplace_back(t1, []() {});
	chain4c.emplace_back(t1, []() {});

	const char *msg1 = "numEvents()++ after emplace_back";
	TEST_ASSERT_EQUAL_MESSAGE(chain1.numEvents() + 1, chain1c.numEvents(),
							  msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain2.numEvents() + 1, chain2c.numEvents(),
							  msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain3.numEvents() + 1, chain3c.numEvents(),
							  msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain4.numEvents() + 1, chain4c.numEvents(),
							  msg1);

	const char *msg2 =
		"getTotalTime() += new time after new time after emplace_back";
	TEST_ASSERT_EQUAL_MESSAGE(chain1.getTotalTime() + 10,
							  chain1c.getTotalTime(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(chain2.getTotalTime() + 10,
							  chain2c.getTotalTime(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(chain3.getTotalTime() + 10,
							  chain3c.getTotalTime(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(chain4.getTotalTime() + 10,
							  chain4c.getTotalTime(), msg2);

	const char *msg3 = "getTimeOf(numEvents() - 1) == new time after new time "
					   "after emplace_back";
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain1c.getTimeOf(chain1c.numEvents() - 1),
							  msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain2c.getTimeOf(chain2c.numEvents() - 1),
							  msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain3c.getTimeOf(chain3c.numEvents() - 1),
							  msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain4c.getTimeOf(chain4c.numEvents() - 1),
							  msg3);
}

void push_back() {

	EspEventChain chain1c(e1);
	EspEventChain chain2c(e1, e2);
	EspEventChain chain3c(e1, e2, e3);
	EspEventChain chain4c(e1, e2, e3, e4);

	chain1c.push_back(e1);
	chain2c.push_back(e1);
	chain3c.push_back(e1);
	chain4c.push_back(e1);

	const char *msg1 = "numEvents()++ after push_back";
	TEST_ASSERT_EQUAL_MESSAGE(chain1.numEvents() + 1, chain1c.numEvents(),
							  msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain2.numEvents() + 1, chain2c.numEvents(),
							  msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain3.numEvents() + 1, chain3c.numEvents(),
							  msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain4.numEvents() + 1, chain4c.numEvents(),
							  msg1);

	const char *msg2 =
		"getTotalTime() += new time after new time after push_back";
	TEST_ASSERT_EQUAL_MESSAGE(chain1.getTotalTime() + 10,
							  chain1c.getTotalTime(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(chain2.getTotalTime() + 10,
							  chain2c.getTotalTime(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(chain3.getTotalTime() + 10,
							  chain3c.getTotalTime(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(chain4.getTotalTime() + 10,
							  chain4c.getTotalTime(), msg2);

	const char *msg3 = "getTimeOf(numEvents() - 1) == new time after push_back";
	TEST_ASSERT_EQUAL_MESSAGE(e1.getTime(),
							  chain1c.getTimeOf(chain1c.numEvents() - 1), msg3);
	TEST_ASSERT_EQUAL_MESSAGE(e1.getTime(),
							  chain2c.getTimeOf(chain2c.numEvents() - 1), msg3);
	TEST_ASSERT_EQUAL_MESSAGE(e1.getTime(),
							  chain3c.getTimeOf(chain3c.numEvents() - 1), msg3);
	TEST_ASSERT_EQUAL_MESSAGE(e1.getTime(),
							  chain4c.getTimeOf(chain4c.numEvents() - 1), msg3);
}

void emplace() {

	EspEventChain chain1c(e1);
	EspEventChain chain2c(e1, e2);
	EspEventChain chain3c(e1, e2, e3);
	EspEventChain chain4c(e1, e2, e3, e4);
	const int INDEX = 0;

	chain1c.emplace(INDEX, t1, []() {});
	chain2c.emplace(INDEX, t1, []() {});
	chain3c.emplace(INDEX, t1, []() {});
	chain4c.emplace(INDEX, t1, []() {});

	chain1c.emplace(chain1c.numEvents(), t1, []() {});
	chain2c.emplace(chain2c.numEvents(), t1, []() {});
	chain3c.emplace(chain3c.numEvents(), t1, []() {});
	chain4c.emplace(chain4c.numEvents(), t1, []() {});

	unsigned long size_increase = 2;

	const char *msg1 = "numEvents()++ after emplace";
	TEST_ASSERT_EQUAL_MESSAGE(chain1.numEvents() + size_increase,
							  chain1c.numEvents(), msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain2.numEvents() + size_increase,
							  chain2c.numEvents(), msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain3.numEvents() + size_increase,
							  chain3c.numEvents(), msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain4.numEvents() + size_increase,
							  chain4c.numEvents(), msg1);

	const char *msg2 =
		"getTotalTime() += new time after new time after emplace";
	TEST_ASSERT_EQUAL_MESSAGE(chain1.getTotalTime() + 10 * size_increase,
							  chain1c.getTotalTime(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(chain2.getTotalTime() + 10 * size_increase,
							  chain2c.getTotalTime(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(chain3.getTotalTime() + 10 * size_increase,
							  chain3c.getTotalTime(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(chain4.getTotalTime() + 10 * size_increase,
							  chain4c.getTotalTime(), msg2);

	const char *msg3 = "getTimeOf(index) == new time after emplace";
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain1c.getTimeOf(INDEX), msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain2c.getTimeOf(INDEX), msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain3c.getTimeOf(INDEX), msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain4c.getTimeOf(INDEX), msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain1c.getTimeOf(chain1c.numEvents() - 1),
							  msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain2c.getTimeOf(chain2c.numEvents() - 1),
							  msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain3c.getTimeOf(chain3c.numEvents() - 1),
							  msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain4c.getTimeOf(chain4c.numEvents() - 1),
							  msg3);
}

void insert() {
	EspEventChain chain1c(e1);
	EspEventChain chain2c(e1, e2);
	EspEventChain chain3c(e1, e2, e3);
	EspEventChain chain4c(e1, e2, e3, e4);
	const int INDEX = 0;

	chain1c.insert(INDEX, e1);
	chain2c.insert(INDEX, e1);
	chain3c.insert(INDEX, e1);
	chain4c.insert(INDEX, e1);

	chain1c.insert(chain1c.numEvents(), e1);
	chain2c.insert(chain2c.numEvents(), e1);
	chain3c.insert(chain3c.numEvents(), e1);
	chain4c.insert(chain4c.numEvents(), e1);

	unsigned long size_increase = 2;

	const char *msg1 = "numEvents()++ after insert";
	TEST_ASSERT_EQUAL_MESSAGE(chain1.numEvents() + size_increase,
							  chain1c.numEvents(), msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain2.numEvents() + size_increase,
							  chain2c.numEvents(), msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain3.numEvents() + size_increase,
							  chain3c.numEvents(), msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain4.numEvents() + size_increase,
							  chain4c.numEvents(), msg1);

	const char *msg2 = "getTotalTime() += new time after new time after insert";
	TEST_ASSERT_EQUAL_MESSAGE(chain1.getTotalTime() + 10 * size_increase,
							  chain1c.getTotalTime(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(chain2.getTotalTime() + 10 * size_increase,
							  chain2c.getTotalTime(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(chain3.getTotalTime() + 10 * size_increase,
							  chain3c.getTotalTime(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(chain4.getTotalTime() + 10 * size_increase,
							  chain4c.getTotalTime(), msg2);

	const char *msg3 = "getTimeOf(index) == new time after insert";
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain1c.getTimeOf(INDEX), msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain2c.getTimeOf(INDEX), msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain3c.getTimeOf(INDEX), msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain4c.getTimeOf(INDEX), msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain1c.getTimeOf(chain1c.numEvents() - 1),
							  msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain2c.getTimeOf(chain2c.numEvents() - 1),
							  msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain3c.getTimeOf(chain3c.numEvents() - 1),
							  msg3);
	TEST_ASSERT_EQUAL_MESSAGE(t1, chain4c.getTimeOf(chain4c.numEvents() - 1),
							  msg3);
}

void remove() {
	EspEventChain chain1c(e1, e1, e1);
	EspEventChain chain2c(e1, e2, e1, e1);
	EspEventChain chain3c(e1, e2, e3, e1, e1);
	EspEventChain chain4c(e1, e2, e3, e4, e1, e1);

	const int INDEX = 0;

	const char *msg0 = "Return value of remove()";
	TEST_ASSERT_EQUAL_MESSAGE(e1, chain1c.remove(INDEX), msg0);
	TEST_ASSERT_EQUAL_MESSAGE(e2, chain2c.remove(INDEX), msg0);
	TEST_ASSERT_EQUAL_MESSAGE(e3, chain3c.remove(INDEX), msg0);
	TEST_ASSERT_EQUAL_MESSAGE(e4, chain4c.remove(INDEX), msg0);
	TEST_ASSERT_EQUAL_MESSAGE(e1, chain1c.remove(chain1c.numEvents() - 1),
							  msg0);
	TEST_ASSERT_EQUAL_MESSAGE(e1, chain2c.remove(chain2c.numEvents() - 1),
							  msg0);
	TEST_ASSERT_EQUAL_MESSAGE(e1, chain3c.remove(chain3c.numEvents() - 1),
							  msg0);
	TEST_ASSERT_EQUAL_MESSAGE(e1, chain4c.remove(chain4c.numEvents() - 1),
							  msg0);

	const char *msg1 = "numEvents()-- after remove()";
	// 2x remove returns size to original
	TEST_ASSERT_EQUAL_MESSAGE(chain1.numEvents(), chain1c.numEvents(), msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain2.numEvents(), chain2c.numEvents(), msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain3.numEvents(), chain3c.numEvents(), msg1);
	TEST_ASSERT_EQUAL_MESSAGE(chain4.numEvents(), chain4c.numEvents(), msg1);
}

void changeTimeOf() {
	EspEventChain chain1c(e1, e1, e1);
	EspEventChain chain2c(e1, e2, e1, e1);
	EspEventChain chain3c(e1, e2, e3, e1, e1);
	EspEventChain chain4c(e1, e2, e3, e4, e1, e1);

	const int INDEX = 0;

	chain1c.changeTimeOf(INDEX, 1234);
	chain2c.changeTimeOf(INDEX, 1234);
	chain3c.changeTimeOf(INDEX, 1234);
	chain4c.changeTimeOf(INDEX, 1234);
	chain1c.changeTimeOf(chain1c.numEvents() - 1, 1234);
	chain2c.changeTimeOf(chain2c.numEvents() - 1, 1234);
	chain3c.changeTimeOf(chain3c.numEvents() - 1, 1234);
	chain4c.changeTimeOf(chain4c.numEvents() - 1, 1234);

	const char *msg0 = "Updated value after changeTimeOf()";
	TEST_ASSERT_EQUAL_MESSAGE(1234, chain1c.getTimeOf(INDEX), msg0);
	TEST_ASSERT_EQUAL_MESSAGE(1234, chain2c.getTimeOf(INDEX), msg0);
	TEST_ASSERT_EQUAL_MESSAGE(1234, chain3c.getTimeOf(INDEX), msg0);
	TEST_ASSERT_EQUAL_MESSAGE(1234, chain4c.getTimeOf(INDEX), msg0);
	TEST_ASSERT_EQUAL_MESSAGE(1234, chain1c.getTimeOf(chain1c.numEvents() - 1),
							  msg0);
	TEST_ASSERT_EQUAL_MESSAGE(1234, chain2c.getTimeOf(chain2c.numEvents() - 1),
							  msg0);
	TEST_ASSERT_EQUAL_MESSAGE(1234, chain3c.getTimeOf(chain3c.numEvents() - 1),
							  msg0);
	TEST_ASSERT_EQUAL_MESSAGE(1234, chain4c.getTimeOf(chain4c.numEvents() - 1),
							  msg0);
}

void setup() {
	delay(2000);
	UNITY_BEGIN();
	RUN_TEST(empty_initial_values);
	RUN_TEST(filled_initial_values);
	RUN_TEST(numEvents);
	RUN_TEST(getTotalTime);
	RUN_TEST(getTotalTimeBeforeLast);
	RUN_TEST(getTotalTimeBeforeFirst);
	RUN_TEST(getTotalTimeBefore);
	RUN_TEST(getTimeOf);
	RUN_TEST(emplace);
	RUN_TEST(emplace_back);
	RUN_TEST(insert);
	RUN_TEST(push_back);
	RUN_TEST(remove);
	RUN_TEST(changeTimeOf);

	UNITY_END();
}

void loop() {}

#endif
