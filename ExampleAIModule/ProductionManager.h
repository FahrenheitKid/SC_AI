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
	int max_supply;
	int supply_percentage;

	BWAPI::Race race;

	amount_status minerals_status;
	amount_status gas_status;
	amount_status supply_status;

	vector <UnitType> UniversalBuildQueue;

public:

	Utilities util;
	std::string hello;
	ProductionManager();
	void updateResources();

	//build a supply unit
	int buildSupply(int queueAmountThreshold);

	bool isReservedMineralsBiggerThan(int amount);
	bool isReservedGasBiggerThan(int amount);

	// returns true if There are more overall minerals than the reserved ones
	bool isThereAvailableMinerals();
	// returns true if There are more overall gas than the reserved ones
	bool isThereAvailableGas();

	//reserve the unit's mineral and gas amount
	bool reserveUnitPrice(UnitType u);

	//remove the unit's mineral and gas amount reserved resources
	bool dereserveUnitPrice(UnitType u);

	void update();

	// makes all idle nexus build workers
	void makeIdleNexusBuildWorkers();

	int getSpeedtest() { return speedtest; };

	/*get and sets functions*/

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
	int getReserved_minerals() const { return reserved_minerals; }
	void setReserved_minerals(int val) { reserved_minerals = val; }
	int getGas() const { return gas; }
	void setGas(int val) { gas = val; }

	~ProductionManager();
};
