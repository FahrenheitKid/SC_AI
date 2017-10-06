#pragma once
#include "Source\ExampleAIModule.h"
#include <string>
#include <Utilities.h>
using namespace std;
using namespace BWAPI;

class StrategyManager
{

private:
	BWAPI::Race race;

	bool doProtossStrat = false;
	
public:
	StrategyManager();

	void init();
	Race getRace() { return race; };
	void setRace(Race r) { race = r; };
	~StrategyManager();
};

