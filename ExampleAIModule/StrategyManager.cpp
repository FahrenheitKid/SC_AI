#include "StrategyManager.h"

StrategyManager::StrategyManager()
{
	currentStrategy = Zealot_Rush;
}

void StrategyManager::init(ProductionManager p)
{
	prManager_ptr = &p;

	race = Broodwar->self()->getRace();
	if (race.getID() == BWAPI::Races::Protoss)
	{
		doProtossStrat = true;

		//Broodwar->sendText("teste in");
	}

	p.SetRace(race);
	//Broodwar->sendText("teste AAAfora");
}

void StrategyManager::update()
{
}

StrategyManager::~StrategyManager()
{
}