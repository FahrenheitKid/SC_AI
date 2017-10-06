#pragma once
#include "Source\ExampleAIModule.h"
#include <string>
#include <Utilities.h>

using namespace std;
using namespace BWAPI;

class ProductionManager
{

private:

	int speedtest;
	int gas;
	int minerals;
	int supply;
	int max_supply;

	amount_status minerals_status;
	amount_status gas_status;
	amount_status supply_status;
	

	
public:
	std::string hello;
	ProductionManager();
	void updateResources();
	void update();
	int getSpeedtest() { return speedtest; };
	~ProductionManager();
};

