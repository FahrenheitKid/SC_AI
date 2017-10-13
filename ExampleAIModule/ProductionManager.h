#ifndef PRODUCTION_H
#define PRODUCTION_H

#pragma once
#include "Source\ExampleAIModule.h"
#include <string>
#include <Utilities.h>
#include <vector>

using namespace std;
using namespace BWAPI;

class ProductionManager
{
private:

	int speedtest;
	int gas;
	int reserved_gas;
	int minerals;
	int reserved_minerals;
	int supply;
	int reserved_supply;
	int max_supply;
	int supply_percentage;
	bool hold_worker_production;
	BWAPI::Race race;

	bool zealotRush;
	//overall status of resources
	amount_status minerals_status;
	amount_status gas_status;
	amount_status supply_status;

	vector <UnitType> UniversalBuildQueue;

	//buildings and units queue
	vector <buildingInfo> buildingsQueue;
	vector <buildingInfo> buildingsQueueBackup;
	vector <unitInfo> unitsQueue;

	//The next unit that we should create
	buildingInfo nextBuidling;
	unitInfo nextUnit;

	Unit scout;

public:

	Utilities util;
	std::string hello;
	Position lastEnemyBaseLocation;
	bool isScounting;
	bool holdScouting;

	ProductionManager();

	//make the same actions as the constructor
	void constructorInit();

	~ProductionManager();

	void updateResources();

	//build a supply unit
	int buildSupply(int queueAmountThreshold);

	bool isReservedMineralsBiggerThan(int amount);
	bool isReservedGasBiggerThan(int amount);
	bool isReservedSupplyBiggerThan(int amount);

	// returns true if There are more overall minerals than the reserved ones
	bool isThereAvailableMinerals();
	// returns true if There are more overall gas than the reserved ones
	bool isThereAvailableGas();

	// returns true if There are more overall supply  than the reserved ones
	bool isThereAvailableSupply();

	//returns the amount available for use e.g. pure value - reserved ones
	unsigned int getAvailableSupply() const;
	unsigned int getAvailableMinerals() const;
	unsigned int getAvailableGas() const;

	//returns if a unit is disabled somehow or is constructing/being constructed
	bool isUnitDisabled(Unit u);

	//add the standart build order to the queue
	void pushStandartBuildingQueue();

	void moveScouts();

	//make a cybernetics core make dragoon upgrade
	bool makeUpgradeAt(BWAPI::UpgradeType upgType, int supply_timing);

	bool obtainNextUpgrade(BWAPI::UpgradeType upgType);
	// all these are utility functions from ualbertabot !!
	bool enemyWorkerInRadius();
	BWAPI::Unit closestEnemyWorker();
	void smartMove(BWAPI::Unit  attacker, BWAPI::Position targetPosition);
	// ualbertabot ends here

	// returns true if there is available resources to build a unit
	bool isThereAvailableResourcesFor(UnitType u);

	//returns true if there is available resources for a set amount
	bool isThereAvailableResourcesFor(int mineral_cost, int gas_cost, int supply_cost);
	bool isThereAvailableResourcesFor(UpgradeType t);

	bool isThereAvailableResourcesFor(TechType t);

	//reserve the unit's mineral and gas amount
	bool reserveUnitPrice(UnitType u);

	//remove the unit's mineral and gas amount reserved resources
	bool dereserveUnitPrice(UnitType u);

	void update();

	BWAPI::Unit getClosestEnemyNexus();
	//choose a worker and send it to scout;
	bool sendScout();

	void updateScoutStatus();

	//set zealot rush strategy build orders
	bool setZealotRushQueues();

	bool setSneakTemplarsQueues();

	// makes all idle nexus build workers
	void makeIdleNexusBuildWorkers();

	//function that checks the queue order for units with infinite (-1) quantity and build them
	void makeBuildingsBuildEndless();

	//returns how many buildings we own that cna build a specific unit
	int howManyUnitsCanMake(UnitType unitToBuild);

	//makes all idle workers work
	void makeAllIdlesWork(int refineries_amount);

	//makes a idle or farming worker go build a specific building/unit
	bool makeWorkerBuild(UnitType unitToBuild, int queueAmountThreshold);

	//order the first building that can build the unit with empty space in training queue to build it
	bool makeSomeBuildingBuild(UnitType unitToBuild, int queueAmountThreshold);

	//searches for vespene geyser and builds a refinery there
	void searchAndBuildRefinery();

	//follows the building order list, creating the buildings when the time comes
	void followBuildingOrder();

	//follows the units order list, creating the units when the time comes
	void followUnitOrder();

	//rebuilds initial buildings if they got destroyed
	void rebuildBackup();

	int getRefineriesAmount();
	/*get and sets functions*/
	int getSpeedtest() { return speedtest; };

	bool isScout(Unit u);

	amount_status getGasStatus() const { return gas_status; }
	void setGasStatus(amount_status val) { gas_status = val; }
	amount_status getMineralStatus() const { return minerals_status; }
	void setMineralStatus(amount_status val) { minerals_status = val; }
	amount_status getSupplyStatus() const { return supply_status; }
	void setSupplyStatus(amount_status val) { supply_status = val; }
	int getSupply_percentage() const { return supply_percentage; }
	void setSupply_percentage(int val) { supply_percentage = val; }
	BWAPI::Race getRace() const { return race; }
	void SetRace(BWAPI::Race val) { race = val; }
	int getMinerals() const { return minerals; }
	void setMinerals(int val) { minerals = val; }
	int getReservedMinerals() const { return reserved_minerals; }
	void setReservedMinerals(int val) { reserved_minerals = val; }
	int getGas() const { return gas; }
	void setGas(int val) { gas = val; }

	int getReservedGas() const { return reserved_gas; }
	void setReservedGas(int val) { reserved_gas = val; }
	int getSupply() const { return supply; }
	void setSupply(int val) { supply = val; }
	int getReservedSupply() const { return reserved_supply; }
	void setReservedSupply(int val) { reserved_supply = val; }
	int getMaxSupply() const { return max_supply; }
	void setMaxSupply(int val) { max_supply = val; }

	//function to immediately adjust reserved resources in case they are bigger than the actual resources
	void contigencyReservedResources();

	//decrements the first building find in the the building queue
	bool decrementFirstBuildingInQueueOrder(Unit u);
	//decrements the first unit find in the the building queue
	bool decrementFirstUnitInQueueOrder(Unit u);

	//decrements the first unit/building in the queue
	bool decrementFirstInQueueOrder(Unit u);

	//check if a unit is in queue order
	bool isUnitInQueueOrder(Unit u);

	buildingInfo getNextBuidling() const { return nextBuidling; }
	void setNextBuidling(buildingInfo val) { nextBuidling = val; }
	vector <buildingInfo> getBuildingsQueue() const { return buildingsQueue; }
	void setBuildingsQueue(vector <buildingInfo> val) { buildingsQueue = val; }
	BWAPI::Unit getScout() const { return scout; }
	void setScout(BWAPI::Unit val) { scout = val; isScounting = true; }
	void resetScout() { scout = nullptr; isScounting = false; };
	bool isZealotRush() { return zealotRush; }
	BWAPI::Unit getPossibleScout(UnitType type);
	BWAPI::Position getLastEnemyBaseLocation() const { return lastEnemyBaseLocation; }
	void setLastEnemyBaseLocation(BWAPI::Position val) { lastEnemyBaseLocation = val; }
};
#endif