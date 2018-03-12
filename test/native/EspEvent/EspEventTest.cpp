#ifdef UNIT_TEST

#include "EspEvent.h"
#include "unity.h"

EspEvent e1;
EspEvent e2(10, []() {});
const char *HANDLE = "handle";
EspEvent e3(20, []() {}, HANDLE);

void empty_constructor() {

	TEST_ASSERT_EQUAL_MESSAGE(0, e1.getTime(), "Empty ctor time = 0");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("null", e1.getHandle(),
									 "Empty ctor handle");
	TEST_ASSERT_FALSE_MESSAGE(e1, "Empty event ctor not callable");
}

void filled_constructor_default_handle() {
	TEST_ASSERT_EQUAL_MESSAGE(10, e2.getTime(), "Correct ctor time set");
	TEST_ASSERT_TRUE_MESSAGE(e2, "Nonempty ctor callable");
}

void filled_constructor_custom_handle() {
	TEST_ASSERT_EQUAL_MESSAGE(20, e3.getTime(), "Correct ctor time set");
	TEST_ASSERT_EQUAL_MESSAGE("handle", e3.getHandle(), "Correct ctor handle");
	TEST_ASSERT_TRUE_MESSAGE(e3, "Nonempty ctor callable");
}

void getTime() {
	const char *msg1 = "getTime()";
	TEST_ASSERT_EQUAL_MESSAGE(0, e1.getTime(), msg1);
	TEST_ASSERT_EQUAL_MESSAGE(10, e2.getTime(), msg1);
	TEST_ASSERT_EQUAL_MESSAGE(20, e3.getTime(), msg1);
}

void getHandle() {
	const char *msg2 = "getHandle()";
	TEST_ASSERT_EQUAL_STRING_MESSAGE("null", e1.getHandle(), msg2);
	TEST_ASSERT_EQUAL_MESSAGE(HANDLE, e3.getHandle(), msg2);
}

void setTime() {

	const char *msg = "setTime()";
	const int E1_SET_TIME = 100;
	const int E2_SET_TIME = 200;
	const int E3_SET_TIME = 300;

	e1.setTime(E1_SET_TIME);
	e2.setTime(E2_SET_TIME);
	e3.setTime(E3_SET_TIME);

	TEST_ASSERT_EQUAL_MESSAGE(E1_SET_TIME, e1.getTime(), msg);
	TEST_ASSERT_EQUAL_MESSAGE(E2_SET_TIME, e2.getTime(), msg);
	TEST_ASSERT_EQUAL_MESSAGE(E3_SET_TIME, e3.getTime(), msg);
}

void setCallback() {

	const char *msg = "setCallback()";
	bool e1_callback_ran = false;
	bool e2_callback_ran = false;
	bool e3_callback_ran = false;
	EspEvent::callback_t E1_SET_CALLBACK = [&]() { e1_callback_ran = true; };
	EspEvent::callback_t E2_SET_CALLBACK = [&]() { e2_callback_ran = true; };
	EspEvent::callback_t E3_SET_CALLBACK = [&]() { e3_callback_ran = true; };

	e1.setCallback(E1_SET_CALLBACK);
	e2.setCallback(E2_SET_CALLBACK);
	e3.setCallback(E3_SET_CALLBACK);

	e1.runEvent();
	e2.runEvent();
	e3.runEvent();
	TEST_ASSERT_TRUE_MESSAGE(e1_callback_ran, msg);
	TEST_ASSERT_TRUE_MESSAGE(e2_callback_ran, msg);
	TEST_ASSERT_TRUE_MESSAGE(e3_callback_ran, msg);
}

int main(int argc, char **argv) {
	UNITY_BEGIN();
	RUN_TEST(empty_constructor);
	RUN_TEST(filled_constructor_default_handle);
	RUN_TEST(filled_constructor_custom_handle);
	RUN_TEST(getTime);
	RUN_TEST(getHandle);
	RUN_TEST(setTime);
	RUN_TEST(setCallback);
	UNITY_END();
	return 0;
}

#endif
