#ifndef UTILITIES_H
#define UTILITIES_H
#endif

#pragma once
#include "Source\ExampleAIModule.h"
#include <string>
#include <Utilities.h>

using namespace std;
using namespace BWAPI;
using namespace Filter;

//enum to activate certain strategies
enum Strats
{
	Standart,
	Zealot_Rush
};

enum amount_status
{
	NONE,
	LITTLE,
	SOME,
	PLENTY,
	FULL
};

struct buildTiming
{
	int current_sup;
	int current_maxsup;
};

//struct used in buildings queue.

//It has the building type, when to build (using "supply time units") and the amount that should be created.
struct buildingInfo
{
	UnitType building;
	int supply_timing;
	int quantity;
};
//same as above but with units instead
struct unitInfo
{
	UnitType unit;
	int supply_timing;
	int quantity;
};

class Utilities
{
public:

	bool isUnitDisabled(Unit u); // Returns true if unit is somehow disabled

	//int buildSupply(int queueAmountThreshold, amount_status supplypercent, int supplytrigger);
	Utilities();
	~Utilities();
};
