#pragma once

#include "Source\ExampleAIModule.h"
#include <string>
#include <Utilities.h>
#include "ProductionManager.h"
#include <vector>
using namespace std;
using namespace BWAPI;
class CombatManager
{

private:
	ProductionManager *prManager_ptr;
	amount_status defense_status;
	int attackArmy_DPS;
	int defenseArmy_DPS;
	int swatArmy_DPS;

	vector <BWAPI::Unit> allArmy;
	vector <BWAPI::Unit> attackArmy;
	vector<BWAPI::Unitset> attackSets;
	vector <BWAPI::Unit> defenseArmy;
	vector <BWAPI::Unit> swatArmy;

	bool zealotRush;
	
public:
	CombatManager();
	void init(ProductionManager prM);
	void updateArmyQuantity();

	void update();

	bool pushAttackArmy(Unit u);
	BWAPI::Unit getClosestEnemyNexus();
	~CombatManager();
	
	//ualbertabot utility functions
	void SmartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target);
	void SmartAttackMove(BWAPI::Unit attacker, const BWAPI::Position & targetPosition);
	void SmartMove(BWAPI::Unit attacker, const BWAPI::Position & targetPosition);
	
	std::vector<BWAPI::Unit> getAttackArmy() const { return attackArmy; }
	void setAttackArmy(std::vector<BWAPI::Unit> val) { attackArmy = val; }
	std::vector<BWAPI::Unitset> getAttackSets() const { return attackSets; }
	void setAttackSets(std::vector<BWAPI::Unitset> val) { attackSets = val; }
};
