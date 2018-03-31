#include <Arduino.h>
#include <Arduino.h>
#include "EspEventChain.h"

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

void push_back()
{

	EspEventChain chain1c(e1);
	EspEventChain chain2c(e1, e2);
	EspEventChain chain3c(e1, e2, e3);
	EspEventChain chain4c(e1, e2, e3, e4);

	chain1c.push_back(e1);
	chain2c.push_back(e1);
	chain3c.push_back(e1);
	chain4c.push_back(e1);
}

void emplace()
{

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
}

void setup()
{
	push_back();
	emplace();
}

void loop() {}
