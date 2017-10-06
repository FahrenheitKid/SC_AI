#include "StrategyManager.h"



StrategyManager::StrategyManager()
{
}

void StrategyManager::init()
{

	race = Broodwar->self()->getRace();
	if(race.getID() == BWAPI::Races::Protoss)
	{ 
		doProtossStrat = true;

		//Broodwar->sendText("teste in");
	}

	//Broodwar->sendText("teste AAAfora");
}


StrategyManager::~StrategyManager()
{
}
