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

enum amount_status
{
	NONE,
	LITTLE,
	SOME,
	PLENTY,
	FULL
};

class Utilities
{
public:

	bool isUnitDisabled(Unit u); // Returns true if unit is somehow disabled

	//int buildSupply(int queueAmountThreshold, amount_status supplypercent, int supplytrigger);
	Utilities();
	~Utilities();
};
