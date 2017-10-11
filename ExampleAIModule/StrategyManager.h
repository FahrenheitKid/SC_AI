#pragma once
#include "Source\ExampleAIModule.h"
#include <string>
#include <ProductionManager.h>
#include <Utilities.h>
using namespace std;
using namespace BWAPI;

class StrategyManager
{
private:
	BWAPI::Race race;

	bool doProtossStrat = false;
	Strats currentStrategy;
	ProductionManager *prManager_ptr;

public:
	StrategyManager();

	//void init();
	void init(ProductionManager p);
	Race getRace() { return race; };
	void setRace(Race r) { race = r; };

	void update();

	~StrategyManager();
	Strats getCurrentStrategy() const { return currentStrategy; }
	void setCurrentStrategy(Strats val) { currentStrategy = val; }
};
