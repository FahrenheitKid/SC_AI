#include "ProductionManager.h"

ProductionManager::ProductionManager()
{
	speedtest = 1;
	hello = "Hello World";
	ProductionManager::setGas(0);
	setReservedGas(0);
	ProductionManager::minerals = 0;
	reserved_minerals = 0;
	setSupply(4);
	setMaxSupply(9);
	supply_percentage = 0;
	hold_worker_production = false;
	zealotRush = false;

	if (!race)
		race = Broodwar->self()->getRace();

	minerals_status = NONE;
	gas_status = NONE;
	supply_status = NONE;
	/*

	*/

	//CANT USE THIS LINE OTHERWISE BREAKS THE DLL
	//if (setZealotRushQueues()) Broodwar << "QUEUE SETADA " << endl;
}

void ProductionManager::constructorInit()
{
	lastEnemyBaseLocation = Position::Point(0, 0);

	speedtest = 1;
	hello = "Hello World";
	ProductionManager::setGas(0);
	setReservedGas(0);
	ProductionManager::minerals = 0;
	reserved_minerals = 0;
	setSupply(4);
	setMaxSupply(9);
	supply_percentage = 0;
	hold_worker_production = false;
	isScounting = false;
	holdScouting = false;
	zealotRush = false;
	if (!race)
		race = Broodwar->self()->getRace();

	minerals_status = NONE;
	gas_status = NONE;
	supply_status = NONE;
	/*

	*/

	/*
	if (setSneakTemplarsQueues()) Broodwar << "QUEUE  TEMPLAR SETADA " << endl;
	else Broodwar << "QUEUE ERROR " << endl;
	*/

	if (setZealotRushQueues()) Broodwar << "QUEUE  zealot SETADA " << endl;
	else Broodwar << "QUEUE ERROR " << endl;

	buildingsQueueBackup = buildingsQueue;
}

void ProductionManager::updateResources()
{
	if (!Broodwar->self()) return;
	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return;
	if (Broodwar->self()->getUnits().size() <= 0) return;

	//int val = Broodwar->self()->getRace()->getWorker()->getType().mineralPrice();
	//mineral sorting

	setMinerals(Broodwar->self()->minerals());
	setSupply(Broodwar->self()->supplyUsed() / 2);
	setMaxSupply(Broodwar->self()->supplyTotal() / 2);
	setGas(Broodwar->self()->gas());

	{
		if (getMinerals() == 0)
		{
			setMineralStatus(NONE);
		}
		else if (getMinerals() > 0 && getMinerals() <= 300)
		{
			setMineralStatus(LITTLE);
		}
		else if (getMinerals() > 300 && getMinerals() <= 1000)
		{
			setMineralStatus(SOME);
		}
		else if (getMinerals() > 1000 && getMinerals() <= 2500)
		{
			setMineralStatus(PLENTY);
		}
		else if (getMinerals() > 2500 && getMinerals() <= 70000)
		{
			setMineralStatus(FULL);
		}
	}

	//gas sorting
	{
		if (getGas() == 0)
		{
			setGasStatus(NONE);
		}
		else if (getGas() > 0 && getGas() <= 400)
		{
			setGasStatus(LITTLE);
		}
		else if (getGas() > 400 && getGas() <= 1000)
		{
			setGasStatus(SOME);
		}
		else if (getGas() > 1000 && getGas() <= 2500)
		{
			setGasStatus(PLENTY);
		}
		else if (getGas() > 2500 && getGas() <= 70000)
		{
			setGasStatus(FULL);
		}
	}

	//supply sorting
	{
		///max_supply -- 100 %
		//supply --- x

		// x max_supply == 100 supply
		// x == (100 supply) / max_supply

		if (getSupply() == 0 || getMaxSupply() == 0)
			supply_percentage = 0;
		else
			supply_percentage = 100 * getSupply() / getMaxSupply();

		if (supply_percentage <= 0)
		{
			setSupplyStatus(NONE);
		}
		else if (supply_percentage > 0 && supply_percentage <= 30)
		{
			setSupplyStatus(LITTLE);
		}
		else if (supply_percentage > 30 && supply_percentage <= 80)
		{
			setSupplyStatus(SOME);
		}
		else if (supply_percentage > 80 && supply_percentage < 100)
		{
			setSupplyStatus(PLENTY);
		}
		else if (supply_percentage >= 100)
		{
			setSupplyStatus(FULL);
		}
	}
}

//function to build a supply unit. First parameter is how many already incomplete supplies is the limit to order another one
// if the limit is surpassed, the order is ignored to avoid waste of resources
int ProductionManager::buildSupply(int queueAmountThreshold)
{
	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return 0;
	if (max_supply >= 200) return 0;

	if (!isThereAvailableResourcesFor(race.getSupplyProvider())) return 0;

	// Iterate through all the units that we own
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (!u->exists()) continue;

		Position pos = u->getPosition();
		Error lastErr = Broodwar->getLastError();
		Broodwar->registerEvent([pos, lastErr](Game*) { Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
			nullptr,    // condition
			Broodwar->getLatencyFrames());  // frames to run

											// Retrieve the supply provider type in the case that we have run out of supplies
		UnitType supplyProviderType = u->getType().getRace().getSupplyProvider();
		static int lastChecked = 0;

		// If we are supply blocked and haven't tried constructing more recently
		if ((supply_status == PLENTY || supply_status == FULL || supply_status == SOME || (getMaxSupply() - getSupply() <= 2)) &&
			lastChecked + 200 < Broodwar->getFrameCount() &&
			Broodwar->self()->incompleteUnitCount(supplyProviderType) <= queueAmountThreshold)
		{
			lastChecked = Broodwar->getFrameCount();

			// Retrieve a unit that is capable of constructing the supply needed
			Unit supplyBuilder = u->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
				(IsIdle || IsGatheringMinerals) && IsOwned);
			// If a unit was found
			if (supplyBuilder && !isUnitDisabled(supplyBuilder))
			{
				if (supplyProviderType.isBuilding())
				{
					TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, supplyBuilder->getTilePosition());

					bool isTooClose = false;
					//check if position is too close of another building

					/*
					for (auto &b : Broodwar->self()->getUnits())
					{
					if (!b->exists() || !b || !targetBuildLocation) continue;

					if (!b->getType().isBuilding()) continue;

					if (targetBuildLocation.getApproxDistance(b->getTilePosition()) <= 10)
					isTooClose = true;
					}

					*/
					if (targetBuildLocation && !isTooClose)
					{
						// Register an event that draws the target build location
						Broodwar->registerEvent([targetBuildLocation, supplyProviderType](Game*)
						{
							Broodwar->drawBoxMap(Position(targetBuildLocation),
								Position(targetBuildLocation + supplyProviderType.tileSize()),
								Colors::Blue);
						},
							nullptr,  // condition
							supplyProviderType.buildTime() + 100);  // frames to run

																	// Order the builder to construct the supply structure
																	//Broodwar << "ENTROU" << endl;

						if (supplyBuilder->build(supplyProviderType, targetBuildLocation))
						{
							if (isThereAvailableResourcesFor(supplyProviderType))
								reserveUnitPrice(supplyProviderType);
							//Broodwar << "Reservou UM probe (100)" << endl;
						}
					}
				}
				else
				{
					// Train the supply provider (Overlord) if the provider is not a structure
					return 1;
					supplyBuilder->train(supplyProviderType);
				}
			} // closure: supplyBuilder is valid
		} // closure: insufficient supply
	}

	return 0;
}

bool ProductionManager::isReservedMineralsBiggerThan(int amount)
{
	return getReservedMinerals() > amount;
}

bool ProductionManager::isReservedGasBiggerThan(int amount)
{
	return getReservedGas() > amount;
}

bool ProductionManager::isReservedSupplyBiggerThan(int amount)
{
	return getReservedSupply() > amount;
}

bool ProductionManager::isThereAvailableMinerals()
{
	return !isReservedMineralsBiggerThan(getMinerals());
}

bool ProductionManager::isThereAvailableGas()
{
	return !isReservedGasBiggerThan(getGas());
}

bool ProductionManager::isThereAvailableSupply()
{
	return !isReservedSupplyBiggerThan(getMaxSupply() - getSupply());
}

bool ProductionManager::isThereAvailableResourcesFor(UnitType u)
{
	if (!u) return false;
	//if (!isThereAvailableGas() || !isThereAvailableMinerals() || !isThereAvailableSupply()) return false;

	if ((u.mineralPrice() <= getAvailableMinerals()) && (u.gasPrice() <= getAvailableGas()) && (u.supplyRequired() / 2 <= getAvailableSupply()))
	{
		//setReserved_minerals(getReserved_minerals() + u.mineralPrice());
		//reserved_gas += u.gasPrice();
		return true;
	}
	else return false;
}

bool ProductionManager::isThereAvailableResourcesFor(int mineral_cost, int gas_cost, int supply_cost)
{
	//if (!u) return false;
	//if (!isThereAvailableGas() || !isThereAvailableMinerals() || !isThereAvailableSupply()) return false;

	if ((mineral_cost <= getAvailableMinerals()) && (gas_cost <= getAvailableGas()) && (supply_cost <= getAvailableSupply()))
	{
		//setReserved_minerals(getReserved_minerals() + u.mineralPrice());
		//reserved_gas += u.gasPrice();
		return true;
	}
	else return false;
}

bool ProductionManager::isThereAvailableResourcesFor(UpgradeType t)
{
	if ((t.mineralPrice() <= getAvailableMinerals()) && (t.gasPrice() <= getAvailableGas()))
	{
		//setReserved_minerals(getReserved_minerals() + u.mineralPrice());
		//reserved_gas += u.gasPrice();
		return true;
	}
	else return false;
}

bool ProductionManager::isThereAvailableResourcesFor(TechType t)
{
	if ((t.mineralPrice() <= getAvailableMinerals()) && (t.gasPrice() <= getAvailableGas()))
	{
		//setReserved_minerals(getReserved_minerals() + u.mineralPrice());
		//reserved_gas += u.gasPrice();
		return true;
	}
	else return false;
}
bool ProductionManager::reserveUnitPrice(UnitType u)
{
	if (!u) return false;

	if (!isThereAvailableResourcesFor(u)) return false;

	if (!isThereAvailableGas() || !isThereAvailableMinerals() || !isThereAvailableSupply()) return false;

	if ((u.mineralPrice() <= getAvailableMinerals()) && (u.gasPrice() <= getAvailableGas()) && (u.supplyRequired() <= getAvailableSupply()))
	{
		if (isThereAvailableResourcesFor(u))
		{
			setReservedMinerals(getReservedMinerals() + u.mineralPrice());
			setReservedGas(getReservedGas() + u.gasPrice());
			setReservedSupply(getReservedSupply() + u.supplyRequired());
			return true;
		}
		else return false;
	}
	else return false;
}

bool ProductionManager::dereserveUnitPrice(UnitType u)
{
	if (!u) return false;

	if ((getReservedMinerals() >= u.mineralPrice()) && (getReservedGas() >= u.gasPrice()) && (getReservedSupply() >= u.supplyRequired()))
	{
		setReservedMinerals(getReservedMinerals() - u.mineralPrice());
		setReservedGas(getReservedGas() - u.gasPrice());
		setReservedSupply(getReservedSupply() - u.supplyRequired());

		return true;
	}
	else return false;
}

void ProductionManager::update()
{
	if (!Broodwar->self()) return;
	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return;
	if (Broodwar->self()->getUnits().size() <= 0) return;

	//makeUpgradeAt(UpgradeTypes::Singularity_Charge, 15);
	//makeUpgradeAt(UpgradeTypes::Leg_Enhancements, 20);

	updateResources();
	makeAllIdlesWork(getRefineriesAmount());
	updateResources();

	//if supplies are running low, order to build more supplies
	if (getSupplyStatus() == PLENTY || getSupplyStatus() == FULL)
	{
		UnitType supplyT = Broodwar->self()->getRace().getSupplyProvider();
		if (supplyT)
		{
			buildSupply(0);
			updateResources();
		}
	}

	if (getMineralStatus() == PLENTY || getMineralStatus() == SOME)
	{
		///Broodwar << "IM RICH IM RICH IM RICH" << endl;

		if (zealotRush)
		{
			if (buildingsQueue.size() == 0)
			{
				searchAndBuildRefinery();
			}
		}
		else
		{
			searchAndBuildRefinery();
		}
	}

	if (getSupply() >= 7)
	{
		Position p(0, 0);
		if (!holdScouting)
			sendScout();
	}

	updateScoutStatus();

	//searchAndBuildRefinery();
	updateResources();
	makeBuildingsBuildEndless();
	updateResources();

	followBuildingOrder();
	updateResources();

	followUnitOrder();
	updateResources();

	if (getSupplyStatus() != PLENTY || getSupplyStatus() != FULL && ((getMaxSupply() - getSupply()) <= 2))
	{
		//if(!hold_worker_production)
		makeIdleNexusBuildWorkers();
		updateResources();
	}
}

bool ProductionManager::sendScout()
{
	//isScounting = true;
	Unit hold = getPossibleScout(Broodwar->self()->getRace().getWorker());
	if (hold && hold->exists() && getScout() == nullptr)
	{
		Broodwar << "setou scout" << endl;
		setScout(hold);
		moveScouts();
	}

	return true;
}

void ProductionManager::updateScoutStatus()
{
	if (holdScouting) resetScout();

	if (!getScout()) return;

	if (!isScounting && getScout()->exists())
	{
		isScounting = true;
	}

	if (getClosestEnemyNexus()) holdScouting = true;
	//if (isScounting && !getScout()->exists()) isScounting = false;

	return;
}

bool ProductionManager::setZealotRushQueues()
{
	//need to fix this
	//if (Broodwar->self()->getRace() != Protoss) return false;

	buildingInfo firstGate;
	firstGate.building = UnitTypes::Protoss_Gateway;
	firstGate.supply_timing = 10;
	firstGate.quantity = 1;

	buildingInfo secondGate;
	secondGate.building = UnitTypes::Protoss_Gateway;
	secondGate.supply_timing = 12;
	secondGate.quantity = 1;

	unitInfo firstZealot;
	firstZealot.unit = UnitTypes::Protoss_Zealot;
	firstZealot.supply_timing = 10;
	firstZealot.quantity = 1;

	unitInfo secondZealot;
	secondZealot.unit = UnitTypes::Protoss_Zealot;
	secondZealot.supply_timing = 12;
	secondZealot.quantity = 2;

	unitInfo thirdZealot;
	secondZealot.unit = UnitTypes::Protoss_Zealot;
	secondZealot.supply_timing = 13;
	secondZealot.quantity = -1;

	buildingsQueue.push_back(firstGate);
	buildingsQueue.push_back(secondGate);

	unitsQueue.push_back(firstZealot);
	unitsQueue.push_back(secondZealot);
	unitsQueue.push_back(thirdZealot);

	zealotRush = true;

	return true;
}

bool ProductionManager::setSneakTemplarsQueues()
{
	buildingInfo firstGate;
	firstGate.building = UnitTypes::Protoss_Gateway;
	firstGate.supply_timing = 10;
	firstGate.quantity = 1;

	buildingInfo secondGate;
	secondGate.building = UnitTypes::Protoss_Gateway;
	secondGate.supply_timing = 12;
	secondGate.quantity = 1;

	unitInfo firstZealot;
	firstZealot.unit = UnitTypes::Protoss_Dark_Templar;
	firstZealot.supply_timing = 10;
	firstZealot.quantity = 1;

	unitInfo secondZealot;
	secondZealot.unit = UnitTypes::Protoss_Dark_Templar;
	secondZealot.supply_timing = 12;
	secondZealot.quantity = 2;

	unitInfo thirdZealot;
	secondZealot.unit = UnitTypes::Protoss_Dark_Templar;
	secondZealot.supply_timing = 13;
	secondZealot.quantity = -1;

	buildingsQueue.push_back(firstGate);
	buildingsQueue.push_back(secondGate);

	unitsQueue.push_back(firstZealot);
	unitsQueue.push_back(secondZealot);
	unitsQueue.push_back(thirdZealot);

	return true;
}

void ProductionManager::makeIdleNexusBuildWorkers()
{
	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return;

	if (!isThereAvailableResourcesFor(race.getWorker())) return;

	if (buildingsQueue.size() > 0)
	{
		int mineral_needed = 0;
		int gas_needed = 0;
		int supply_needed = 0;

		for (auto &b : buildingsQueue)
		{
			if (b.quantity >= 1)
			{
				int eminent = b.supply_timing - getSupply();
				if (eminent >= 0)
				{
					mineral_needed += b.building.mineralPrice();
					gas_needed = b.building.gasPrice();
					supply_needed = b.building.supplyRequired() / 2;
				}
			}
		}

		//if (!isThereAvailableResourcesFor(mineral_needed, gas_needed, supply_needed)) return;
	}

	for (auto &u : Broodwar->self()->getUnits())
	{
		if (!u->exists()) continue;

		if (u->getType().isResourceDepot() && u->exists()) // A resource depot is a Command Center, Nexus, or Hatchery
		{
			//if there is no mineral field close by stop making workers
			//Unit closet_mineral = u->getClosestUnit(Filter::IsMineralField);
			//if (!closet_mineral || !closet_mineral->getType().isMineralField()) return;

			if ((u->isIdle()) && !u->train(u->getType().getRace().getWorker()))
			{
				// If that fails, draw the error at the location so that you can visibly see what went wrong!
				// However, drawing the error once will only appear for a single frame
				// so create an event that keeps it on the screen for some frames
				Position pos = u->getPosition();
				Error lastErr = Broodwar->getLastError();
				Broodwar->registerEvent([pos, lastErr](Game*) { Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
					nullptr,    // condition
					Broodwar->getLatencyFrames());  // frames to run
			} // closure: failed to train idle unit
			else
			{
				if (isThereAvailableResourcesFor(u->getType().getRace().getWorker()))
					reserveUnitPrice(u->getType().getRace().getWorker());
			}
			// Order the depot to construct more workers! But only when it is idle.
		}
	}// closure: unit iterator
}

void ProductionManager::makeBuildingsBuildEndless()
{
	for (int i = 0; i < unitsQueue.size(); i++)
	{
		//if we need to make endless units, need to find the unit that builds it and order it to do it
		if (unitsQueue[i].quantity == -1 && unitsQueue[i].supply_timing <= getSupply())
		{
			//Broodwar << "build endless" << endl;
			makeSomeBuildingBuild(unitsQueue[i].unit, howManyUnitsCanMake(unitsQueue[i].unit));
		}
	}
}

int ProductionManager::howManyUnitsCanMake(UnitType unitToBuild)
{
	int count = 0;

	for (auto &u : Broodwar->self()->getUnits())
	{
		if (!u->exists() || !u) continue;

		if (u->getType() == unitToBuild.whatBuilds().first)
		{
			count++;
		}
	}

	return count;
}

void ProductionManager::makeAllIdlesWork(int refineries_amount)
{
	bool check3atGas = false;
	int amount_gathering_gas = 0;

	// need to make 3 workers at refinery work!!!

	//if we have at least one refinery check to see if at least 3 workers are gathering there
	if (refineries_amount > 0)
	{
		for (auto &u : Broodwar->self()->getUnits())
		{
			if (!u->exists()) continue;

			if (u->getType().isWorker() && !isScout(u))
			{
				if (u->isGatheringGas())
					amount_gathering_gas++;
			}
		}

		// if less than 3, order the necessary workers to achieve 3 at refinery
		if (amount_gathering_gas < 3)
		{
			for (int i = 0; i < 3 - amount_gathering_gas; i++)
			{
				if (amount_gathering_gas == 3) break;
				for (auto &u : Broodwar->self()->getUnits())
				{
					if (!u->exists()) continue;

					if (amount_gathering_gas == 3) break;

					if (u->getType().isWorker() && !u->isGatheringGas() && !isScout(u))
					{
						if (isUnitDisabled(u)) continue;

						if (!u->gather(u->getClosestUnit(Filter::IsRefinery)))
						{
							// If the call fails, then print the last error message
							Broodwar << Broodwar->getLastError() << std::endl;
						}
						else
						{
							break;
							amount_gathering_gas++;
						}
					}
				}
			}
		}
	}

	for (auto &u : Broodwar->self()->getUnits())
	{
		if (!u->exists()) continue;

		if (isUnitDisabled(u))
			continue;

		// Finally make the unit do some stuff!

		// If the unit is a worker unit
		if (u->getType().isWorker())
		{
			// if our worker is idle
			if (u->isIdle() && !u->isConstructing() && !isScout(u))
			{
				// Order workers carrying a resource to return them to the center,
				// otherwise find a mineral patch to harvest.
				if (u->isCarryingGas())
				{
					u->returnCargo();
				}
				else if (u->isCarryingMinerals())
				{
					u->returnCargo();
				}
				else if (!u->getPowerUp())  // The worker cannot harvest anything if it
				{                             // is carrying a powerup such as a flag
											  // Harvest from the nearest mineral patch or gas refinery
											  //if (!u->gather(u->getClosestUnit()->getType().isMineralField() || u->getClosestUnit()->getType().isRefinery()))
					if (!u->isGatheringGas())
					{
						if (!u->gather(u->getClosestUnit(Filter::IsMineralField)))
						{
							// If the call fails, then print the last error message
							Broodwar << Broodwar->getLastError() << std::endl;
						}
						else
						{
						}
					}
				} // closure: has no powerup
			} // closure: if idle
		}
	}
}

bool ProductionManager::makeWorkerBuild(UnitType unitToBuild, int queueAmountThreshold)
{
	if (!Broodwar->self()) return false;
	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return false;
	if (Broodwar->self()->getUnits().size() <= 0) return false;

	if (!isThereAvailableResourcesFor(unitToBuild)) return false;

	// Iterate through all the units that we own
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (!u->exists() || !u) continue;

		Position pos = u->getPosition();
		Error lastErr = Broodwar->getLastError();
		Broodwar->registerEvent([pos, lastErr](Game*) { Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
			nullptr,    // condition
			Broodwar->getLatencyFrames());  // frames to run

		static int lastChecked = 0;

		// If we haven't tried constructing more recently
		if (lastChecked + 400 < Broodwar->getFrameCount() &&
			Broodwar->self()->incompleteUnitCount(unitToBuild) <= queueAmountThreshold)
		{
			lastChecked = Broodwar->getFrameCount();

			// Retrieve a unit that is capable of constructing the supply needed
			Unit builder = u->getClosestUnit(GetType == unitToBuild.whatBuilds().first &&
				(IsIdle || IsGatheringMinerals) && IsOwned);
			// If a unit was found
			if (builder && !isUnitDisabled(builder) && builder->exists() && !isScout(u))
			{
				//if unit to build is a building
				if (unitToBuild.isBuilding() && unitToBuild)
				{
					TilePosition targetBuildLocation = Broodwar->getBuildLocation(unitToBuild, builder->getTilePosition());
					if (targetBuildLocation)
					{
						// Register an event that draws the target build location
						Broodwar->registerEvent([targetBuildLocation, unitToBuild](Game*)
						{
							Broodwar->drawBoxMap(Position(targetBuildLocation),
								Position(targetBuildLocation + unitToBuild.tileSize()),
								Colors::Blue);
						},
							nullptr,  // condition
							unitToBuild.buildTime() + 100);  // frames to run

															 // Order the builder to construct the supply structure
															 //Broodwar << "ENTROU" << endl;

						if (builder->build(unitToBuild, targetBuildLocation))
						{
							if (isThereAvailableResourcesFor(unitToBuild))
							{
								//maybe put inside if
								reserveUnitPrice(unitToBuild);
								return true;
							}

							//Broodwar << "Reservou UM probe (100)" << endl;
						}
					}
				}
				else
				{
					// Train the supply provider (Overlord) if the provider is not a structure

					if (builder->train(unitToBuild))
						return true;
				}
			} // closure: supplyBuilder is valid
		} // closure: insufficient supply
	}

	return false;
}

bool ProductionManager::makeSomeBuildingBuild(UnitType unitToBuild, int queueAmountThreshold)
{
	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return false;

	if (!isThereAvailableResourcesFor(unitToBuild))
	{
		//Broodwar << " no resource to someBuild" << endl;
		return false;
	}

	bool createdAtLeastOne = false;
	int count = 0;
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (!u->exists() || isScout(u)) continue;

		// and there arent more than X other units already building it
		//and training queue is not full or is idle

		//if it can build the unit and is complete
		/*
		*/
		if ((u->getType() == unitToBuild.whatBuilds().first && u->isCompleted()) && (u->getTrainingQueue().size() < 3 || u->isIdle()))
		{
			//and training queue is not full or is idle
			if (!isThereAvailableResourcesFor(unitToBuild)) continue;

			count++;
			//Broodwar << "achou gateway idle ID: " + to_string(count)  << endl;
			if (!u->train(unitToBuild))
			{
				// If that fails, draw the error at the location so that you can visibly see what went wrong!
				// However, drawing the error once will only appear for a single frame
				// so create an event that keeps it on the screen for some frames
				Position pos = u->getPosition();
				Error lastErr = Broodwar->getLastError();
				Broodwar->registerEvent([pos, lastErr](Game*) { Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
					nullptr,    // condition
					Broodwar->getLatencyFrames());  // frames to run
			} // closure: failed to train idle unit
			else
			{
				//Broodwar << "go train Zealot" << endl;
				if (isThereAvailableResourcesFor(unitToBuild))
				{
					reserveUnitPrice(unitToBuild);
				}

				createdAtLeastOne = true;
			}
		}
	}// closure: unit iterator

	return createdAtLeastOne;
}

void ProductionManager::searchAndBuildRefinery()
{
	if (!Broodwar->self()) return;
	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return;
	if (Broodwar->self()->getUnits().size() <= 0) return;

	if (!isThereAvailableResourcesFor(race.getRefinery())) return;

	//first check if we don't have an refinery already
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (!u->exists()) continue;

		if (u->exists() && u->getType().isRefinery() && u)
		{
			//Broodwar << " JA TEMOSSS" << endl;

			return;
		}
	}

	// Resource_Vespene_Geyser

	for (auto &u : Broodwar->self()->getUnits())
	{
		if (!u->exists()) continue;
		static int lastChecked = 0;

		Unit geyser = u->getClosestUnit(GetType == UnitTypes::Resource_Vespene_Geyser);

		UnitType refineryType = u->getType().getRace().getRefinery();

		if (!geyser || !geyser->exists() || !refineryType || isUnitDisabled(geyser)) continue;
		//if (isUnitDisabled(u)) continue;

		// If we haven't tried constructing more recently
		if (lastChecked + 400 < Broodwar->getFrameCount() &&
			Broodwar->self()->incompleteUnitCount(refineryType) == 0)
		{
			lastChecked = Broodwar->getFrameCount();
			Unit refineryBuilder;

			if (u->exists() || u)
				refineryBuilder = u->getClosestUnit(GetType == refineryType.whatBuilds().first &&
				(IsIdle || IsGatheringMinerals) && IsOwned);

			if (refineryBuilder && refineryBuilder->exists() && refineryBuilder->isCompleted() && !refineryBuilder->isConstructing() && !isScout(u))
			{
				if (refineryType.isBuilding() && refineryType)
				{
					TilePosition targetBuildLocation = Broodwar->getBuildLocation(refineryType, geyser->getTilePosition());

					if (targetBuildLocation && targetBuildLocation.isValid())
					{
						// Register an event that draws the target build location
						Broodwar->registerEvent([targetBuildLocation, refineryType](Game*)
						{
							Broodwar->drawBoxMap(Position(targetBuildLocation),
								Position(targetBuildLocation + refineryType.tileSize()),
								Colors::Blue);
						},
							nullptr,  // condition
							refineryType.buildTime() + 100);  // frames to run

															  // Order the builder to construct the supply structure
															  //Broodwar << "ENTROU" << endl;

						if (refineryBuilder->build(refineryType, targetBuildLocation))
						{
							if (isThereAvailableResourcesFor(refineryType))
								reserveUnitPrice(refineryType);
						}
					}
				}
				else
				{
					// Train the refinery provider if the provider is not a structure
					//return 1;
					if (!isUnitDisabled(refineryBuilder) && refineryBuilder && refineryBuilder->exists())
					{
						if (refineryBuilder->train(refineryType))
						{
							if (isThereAvailableResourcesFor(refineryType))
								reserveUnitPrice(refineryType);
						}
					}
				}
			}
		}
	}
}

void ProductionManager::followBuildingOrder()
{
	if (!Broodwar->self()) return;
	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return;
	if (Broodwar->self()->getUnits().size() <= 0) return;

	if (buildingsQueue.size() <= 0) return;

	nextBuidling = buildingsQueue.front();

	if (!nextBuidling.quantity) return;

	//if we need to build at least one building
	if (nextBuidling.quantity >= 1)
	{
		////updateResources();
		if (!nextBuidling.supply_timing) return;

		//if it is the right time and we have the resources for it
		if (getSupply() >= nextBuidling.supply_timing)
		{
			//Broodwar << "entrou build order" << endl;
			//Broodwar << "Need to build a: " + nextBuidling.building.getName() << std::endl;
			//Broodwar << "supply = " + std::to_string(Broodwar->self()->supplyTotal()) + " | supply_timing = " + std::to_string(nextBuidling.supply_timing) << std::endl;

			if (isThereAvailableResourcesFor(nextBuidling.building))
			{
				//make a worker build that building, if the quantity is more than one, it will construct even if its already being build somewhere

				if (nextBuidling.quantity >= 1)
					makeWorkerBuild(nextBuidling.building, nextBuidling.quantity - 1);
			}
		}
	}

	//if()
}

void ProductionManager::followUnitOrder()
{
	if (!Broodwar->self()) return;
	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return;
	if (Broodwar->self()->getUnits().size() <= 0) return;

	if (unitsQueue.size() <= 0) return;

	nextUnit = unitsQueue.front();

	if (!nextUnit.quantity) return;

	//if we need to build at least one unit
	if (nextUnit.quantity >= 1)
	{
		////updateResources();
		if (!nextUnit.supply_timing) return;

		//if it is the right time and we have the resources for it
		if (getSupply() >= nextUnit.supply_timing)
		{
			//Broodwar << "entrou build order" << endl;
			//Broodwar << "Need to build a: " + nextBuidling.building.getName() << std::endl;
			//Broodwar << "supply = " + std::to_string(Broodwar->self()->supplyTotal()) + " | supply_timing = " + std::to_string(nextBuidling.supply_timing) << std::endl;

			if (isThereAvailableResourcesFor(nextUnit.unit))
			{
				//make a worker build that building, if the quantity is more than one, it will construct even if its already being build somewhere

				if (nextUnit.quantity >= 1)
					makeWorkerBuild(nextUnit.unit, nextUnit.quantity - 1);
			}
		}
	}
	else if (nextUnit.quantity == -1)
	{
	}
}

void ProductionManager::rebuildBackup()
{
	for (auto &bd : buildingsQueueBackup)
	{
		//if(Broodwar->self()->allUnitCount())
	}
}

//returns the amount of refineries that we own
int ProductionManager::getRefineriesAmount()
{
	int amount = 0;
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (!u->exists()) continue;

		if (u->getType() == u->getType().getRace().getRefinery())
		{
			if (!isUnitDisabled(u))
				amount++;
		}
	}

	return amount;
}

bool ProductionManager::isScout(Unit u)
{
	if (!u || !u->exists() || !getScout() || !getScout()->exists()) return false;

	return u == getScout();
}

//function to never let reserved resources be bigger than actual ones
void ProductionManager::contigencyReservedResources()
{
	if (getReservedMinerals() > getMinerals()) setReservedMinerals(getMinerals());
	if (getReservedGas() > getGas()) setReservedGas(getGas());
	if (getReservedSupply() > getAvailableSupply()) setReservedSupply(getSupply());
}

bool ProductionManager::decrementFirstBuildingInQueueOrder(Unit u)
{
	if (buildingsQueue.size() < 0) return false;
	if (!u || !u->exists()) return false;

	bool decremented = false;

	for (int i = 0; i < buildingsQueue.size(); i++)
	{
		if (buildingsQueue[i].building == u->getType() && buildingsQueue[i].quantity >= 1)
		{
			//Broodwar << "decrementei" << endl;
			decremented = true;
			buildingsQueue[i].quantity--;
			break;
		}
	}

	//update building queue to remove any building that has 0 quantity
	for (int i = 0; i < buildingsQueue.size(); i++)
	{
		if (buildingsQueue[i].quantity == 0)
		{
			buildingsQueue.erase(buildingsQueue.begin() + i);
		}
	}

	return decremented;
}

bool ProductionManager::decrementFirstUnitInQueueOrder(Unit u)
{
	if (unitsQueue.size() < 0) return false;

	bool decremented = false;

	for (int i = 0; i < unitsQueue.size(); i++)
	{
		if (unitsQueue[i].unit == u->getType() && unitsQueue[i].quantity >= 1)
		{
			decremented = true;
			unitsQueue[i].quantity--;
			break;
		}
	}

	//update building queue to remove any building that has 0 quantity
	for (int i = 0; i < unitsQueue.size(); i++)
	{
		if (unitsQueue[i].quantity == 0)
		{
			unitsQueue.erase(unitsQueue.begin() + i);
		}
	}

	return decremented;
}

bool ProductionManager::decrementFirstInQueueOrder(Unit u)
{
	if (u->getType().isBuilding())
	{
		return decrementFirstBuildingInQueueOrder(u);
	}
	else
	{
		return decrementFirstUnitInQueueOrder(u);
	}
}

bool ProductionManager::isUnitInQueueOrder(Unit u)
{
	if (!u || !u->exists()) return false;

	if (u->getType().isBuilding())
	{
		if (buildingsQueue.size() < 0) return false;

		for (int i = 0; i < buildingsQueue.size(); i++)
		{
			if (buildingsQueue[i].building == u->getType())
			{
				return true;
			}
		}
	}
	else
	{
		if (unitsQueue.size() < 0) return false;
		for (int i = 0; i < unitsQueue.size(); i++)
		{
			if (unitsQueue[i].unit == u->getType())
			{
				return true;
			}
		}
	}

	return false;
}

BWAPI::Unit ProductionManager::getPossibleScout(UnitType type)
{
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (!u->exists())
		{
			continue;
		}
		// if doesnt exists or disabled or not the type that we want
		if (u->getType() != type)
		{
			//Broodwar << "scout NON worker " << endl;
			continue;
		}

		//if worker and carrying gas
		if (u->isCarryingGas() || !u->isCompleted() || u->isConstructing() || u->isCarryingMinerals() || u->getHitPoints() <= 5)
		{
			//Broodwar << "scout GAS CONSTRUCTING INCOMPLETE" << endl;
			continue;
		}

		return u;
	}

	return nullptr;
}

unsigned int ProductionManager::getAvailableSupply() const
{
	return getMaxSupply() - getSupply() - getReservedSupply();
}

unsigned int ProductionManager::getAvailableMinerals() const
{
	return getMinerals() - getReservedMinerals();
}

unsigned int ProductionManager::getAvailableGas() const
{
	return getGas() - getReservedGas();
}

bool ProductionManager::isUnitDisabled(Unit u)
{
	if (!Broodwar->self()) return false;

	if (!u->exists())
		return true;

	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return false;
	if (Broodwar->self()->getUnits().size() <= 0) return false;

	// Ignore the unit if it no longer exists
	// Make sure to include this block when handling any Unit pointer!

	// Ignore the unit if it has one of the following status ailments
	if (u->isLockedDown() || u->isMaelstrommed() || u->isStasised())
		return true;

	// Ignore the unit if it is in one of the following states
	if (u->isLoaded() || !u->isPowered() || u->isStuck())
		return true;

	// Ignore the unit if it is incomplete or busy constructing
	if (!u->isCompleted() || u->isConstructing())
		return true;

	return false;
}

void ProductionManager::pushStandartBuildingQueue()
{
	//if (!buildingsQueue.empty()) continue;

	buildingInfo cyberneticCore;
	cyberneticCore.building = UnitTypes::Protoss_Cybernetics_Core;
	cyberneticCore.supply_timing = 13;
	cyberneticCore.quantity = 1;

	buildingInfo forge;
	forge.building = UnitTypes::Protoss_Forge;
	forge.supply_timing = 20;
	forge.quantity = 1;

	buildingInfo citadel;
	citadel.building = UnitTypes::Protoss_Citadel_of_Adun;
	citadel.supply_timing = 25;
	citadel.quantity = 1;

	buildingInfo arch;
	arch.building = UnitTypes::Protoss_Templar_Archives;
	arch.supply_timing = 25;
	arch.quantity = 1;

	unitInfo firstDragoon;
	firstDragoon.unit = UnitTypes::Protoss_Dragoon;
	firstDragoon.supply_timing = 15;
	firstDragoon.quantity = 1;

	unitInfo secondsDragoon;
	secondsDragoon.unit = UnitTypes::Protoss_Dragoon;
	secondsDragoon.supply_timing = 17;
	secondsDragoon.quantity = 2;

	buildingsQueue.push_back(cyberneticCore);

	buildingsQueue.push_back(forge);

	buildingsQueue.push_back(citadel);

	buildingsQueue.push_back(arch);

	unitsQueue.push_back(firstDragoon);

	unitsQueue.push_back(secondsDragoon);

	buildingsQueueBackup.push_back(cyberneticCore);

	buildingsQueueBackup.push_back(forge);

	buildingsQueueBackup.push_back(citadel);

	buildingsQueueBackup.push_back(arch);
}

void ProductionManager::smartMove(BWAPI::Unit attacker, BWAPI::Position targetPosition)
{
	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Move && currentCommand.getTargetPosition() == targetPosition)
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->move(targetPosition);
}

void ProductionManager::moveScouts()
{
	if (getScout() == nullptr || !getScout()->exists())
	{
		Broodwar << "erro no movescout" << endl;
		return;
	}

	bool scoutUnderAttack = false;

	// we only care if the scout is under attack within the enemy region
	// this ignores if their scout worker attacks it on the way to their base
	if (getScout()->isUnderAttack())
	{
		scoutUnderAttack = true;
	}

	if (!getScout()->isUnderAttack() && !enemyWorkerInRadius())
	{
		scoutUnderAttack = false;
	}

	//if (scoutUnderAttack) holdScouting = true;
	//else holdScouting = true;

	Position bias(0, 0);
	// for each start location in the level
	if (!getClosestEnemyNexus() && lastEnemyBaseLocation == bias)
	{
		Broodwar << "nao achou nexus" << endl;
		for (auto & u : BWAPI::Broodwar->getStartLocations())
		{
			// if we haven't explored it yet
			if (!BWAPI::Broodwar->isExplored(u)) {
				// assign a scout to go scout it
				smartMove(getScout(), BWAPI::Position(u));
				return;
			}
		}
	}
	else
	{
	}

	/*if (scout->exists() && scout->getClosestUnit()->getPlayer()->isEnemy(Broodwar->self()))
	{
	int x = scout->getClosestUnit()->getPosition().x;
	int y = scout->getClosestUnit()->getPosition().y;
	Broodwar << "Setei last enemy como: (" + to_string(x) + ", " + to_string(y) + ")" << endl;
	lastEnemyBaseLocation = scout->getClosestUnit()->getPosition();
	}*/
}

bool ProductionManager::makeUpgradeAt(UpgradeType upgType, int supply_timing)
{
	if (getSupply() < supply_timing || Broodwar->self()->isUpgrading(upgType)) return false;

	for (auto &u : Broodwar->self()->getUnits())
	{
		if (!u->exists())
		{
			continue;
		}

		// if it is not a the type that upgrades it
		if (u->getType() != upgType.whatUpgrades())
		{
			continue;
		}
		else
		{
			if (!u->isCompleted() || u->isConstructing() || u->isUpgrading() || !isThereAvailableResourcesFor(upgType)) continue;

			if (u->upgrade(upgType))
			{
				return true;
			}
		}
	}

	return false;
}

bool ProductionManager::obtainNextUpgrade(BWAPI::UpgradeType upgType)
{
	BWAPI::Player self = BWAPI::Broodwar->self();
	int maxLvl = self->getMaxUpgradeLevel(upgType);
	int currentLvl = self->getUpgradeLevel(upgType);
	if (!self->isUpgrading(upgType) && currentLvl < maxLvl &&
		self->completedUnitCount(upgType.whatsRequired(currentLvl + 1)) > 0 &&
		self->completedUnitCount(upgType.whatUpgrades()) > 0)
		return true; // self->getUnits().upgrade(upgType);
	return false;
}
bool ProductionManager::enemyWorkerInRadius()
{
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType().isWorker() && (unit->getDistance(getScout()) < 300))
		{
			return true;
		}
	}

	return false;
}

BWAPI::Unit ProductionManager::closestEnemyWorker()
{
	BWAPI::Unit enemyWorker = nullptr;
	double maxDist = 0;

	//BWAPI::Unit geyser = getEnemyGeyser();

	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType().isWorker() && unit->isConstructing())
		{
			return unit;
		}
	}

	// for each enemy worker
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType().isWorker())
		{
			double dist = 0;
			Unit geyser = unit->getClosestUnit(Filter::IsRefinery);
			if (geyser && geyser->exists())
				dist = unit->getDistance(geyser);

			if (dist < 800 && dist > maxDist)
			{
				maxDist = dist;
				enemyWorker = unit;
			}
		}
	}

	return enemyWorker;
}

BWAPI::Unit ProductionManager::getClosestEnemyNexus()
{
	for (auto & unit : BWAPI::Broodwar->enemy()->getUnits())
	{
		if (unit->getType().isResourceDepot())
		{
			return unit;
		}
	}

	return nullptr;
}

ProductionManager::~ProductionManager()
{
}