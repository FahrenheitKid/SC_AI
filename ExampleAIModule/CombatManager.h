#ifndef COMBAT_H_
#define COMBAT_H_


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




	bool zealotRush;
	

public:

	int attackTroopSize;
	vector <BWAPI::Unit> allArmy; // all offensive units (except workers)
	vector <BWAPI::Unit> attackArmy; //units set as attack army


	vector<BWAPI::Unitset> attackSets; // sets from the attack army (like troops)
	vector <BWAPI::Unit> defenseArmy; // units set as defense army

	vector <BWAPI::Unit> swatArmy; // a special army if needed to make a specific attack or so

	Position enemyBasePosition; // last know enemy base position

	Position lastEnemySeenPosition; // last know enemy base position

	CombatManager();
	void init(ProductionManager prM);
	void updateArmyQuantity();

	void update();


	void makeIdleArmyAttack(int idle_amount);

	bool pushAttackArmy(Unit u);

	BWAPI::Unit getClosestEnemyNexus();
	BWAPI::Unit getClosestEnemy(Unit u);
	~CombatManager();

	//ualbertabot utility functions
	void SmartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target);
	void SmartAttackMove(BWAPI::Unit attacker, BWAPI::Position targetPosition);
	void SmartMove(BWAPI::Unit attacker, const BWAPI::Position targetPosition);
	// end of utility


	std::vector<BWAPI::Unit> getAttackArmy() { return attackArmy; }
	void setAttackArmy(std::vector<BWAPI::Unit> val) { attackArmy = val; }
	std::vector<BWAPI::Unitset> getAttackSets() const { return attackSets; }
	void setAttackSets(std::vector<BWAPI::Unitset> val) { attackSets = val; }
	std::vector<BWAPI::Unit> getAllArmy() const { return allArmy; }
	void setAllArmy(std::vector<BWAPI::Unit> val) { allArmy = val; }

	bool isThisUnitInThatVector(Unit u, std::vector<BWAPI::Unit> val);
	bool isThisUnitInThatVector(Unitset u, std::vector<BWAPI::Unitset> val);
	bool isThisUnitInThatUnisetVector(Unit u, std::vector<BWAPI::Unitset> val);

};
#endif