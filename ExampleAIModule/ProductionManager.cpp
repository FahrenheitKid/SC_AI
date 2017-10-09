#include "ProductionManager.h"

ProductionManager::ProductionManager()
{
	speedtest = 1;
	hello = "Hello World";
	ProductionManager::setGas(0);
	reserved_gas = 0;
	ProductionManager::minerals = 0;
	reserved_minerals = 0;
	supply = 4;
	max_supply = 9;
	supply_percentage = 0;

	if (!race)
		race = Broodwar->self()->getRace();

	minerals_status = NONE;
	gas_status = NONE;
	supply_status = NONE;
	/*

	*/
}

void ProductionManager::updateResources()
{
	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return;

	//int val = Broodwar->self()->getRace()->getWorker()->getType().mineralPrice();
	//mineral sorting

	setMinerals(Broodwar->self()->minerals());
	supply = Broodwar->self()->supplyUsed();
	max_supply = Broodwar->self()->supplyTotal();
	setGas(Broodwar->self()->gas());

	{
		if (getMinerals() == 0)
		{
			setMineralStatus(NONE);
		}
		else if (getMinerals() > 0 && getMinerals() <= 400)
		{
			setMineralStatus(LITTLE);
		}
		else if (getMinerals() > 400 && getMinerals() <= 1000)
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

		if (supply == 0 || max_supply == 0)
			supply_percentage = 0;
		else
			supply_percentage = 100 * supply / max_supply;

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

	// Iterate through all the units that we own
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (util.isUnitDisabled(u)) continue;

		Position pos = u->getPosition();
		Error lastErr = Broodwar->getLastError();
		Broodwar->registerEvent([pos, lastErr](Game*) { Broodwar->drawTextMap(pos, "%c%s", Text::White, lastErr.c_str()); },   // action
			nullptr,    // condition
			Broodwar->getLatencyFrames());  // frames to run

											// Retrieve the supply provider type in the case that we have run out of supplies
		UnitType supplyProviderType = u->getType().getRace().getSupplyProvider();
		static int lastChecked = 0;

		// If we are supply blocked and haven't tried constructing more recently
		if ((supply_status == PLENTY || supply_status == FULL || supply_status == SOME) &&
			/* lastChecked + 400 < Broodwar->getFrameCount() &&*/
			Broodwar->self()->incompleteUnitCount(supplyProviderType) <= queueAmountThreshold)
		{
			lastChecked = Broodwar->getFrameCount();

			// Retrieve a unit that is capable of constructing the supply needed
			Unit supplyBuilder = u->getClosestUnit(GetType == supplyProviderType.whatBuilds().first &&
				(IsIdle || IsGatheringMinerals) && IsOwned);
			// If a unit was found
			if (supplyBuilder)
			{
				if (supplyProviderType.isBuilding())
				{
					TilePosition targetBuildLocation = Broodwar->getBuildLocation(supplyProviderType, supplyBuilder->getTilePosition());
					if (targetBuildLocation)
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
							if (reserveUnitPrice(supplyProviderType))
								Broodwar << "Reservou UM probe (100)" << endl;
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
	return getReserved_minerals() > amount;
}

bool ProductionManager::isReservedGasBiggerThan(int amount)
{
	return reserved_gas > amount;
}

bool ProductionManager::isThereAvailableMinerals()
{
	return !isReservedMineralsBiggerThan(getMinerals());
}

bool ProductionManager::isThereAvailableGas()
{
	return !isReservedGasBiggerThan(getGas());
}

bool ProductionManager::reserveUnitPrice(UnitType u)
{
	if (!u) return false;

	if ((getReserved_minerals() + u.mineralPrice() <= minerals) && (reserved_gas + u.gasPrice() <= getGas()))
	{
		setReserved_minerals(getReserved_minerals() + u.mineralPrice());
		reserved_gas += u.gasPrice();
		return true;
	}
	else return false;
}

bool ProductionManager::dereserveUnitPrice(UnitType u)
{
	if (!u) return false;

	if ((getReserved_minerals() >= u.mineralPrice()) && (reserved_gas >= u.gasPrice()))
	{
		setReserved_minerals(getReserved_minerals() - u.mineralPrice());
		reserved_gas -= u.gasPrice();
		return true;
	}
	else return false;
}

void ProductionManager::update()
{
	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return;

	updateResources();

	//if supplies are running low, order to build more supplies
	if (getSupplyStatus() == PLENTY || getSupplyStatus() == FULL)
	{
		if (isThereAvailableMinerals())
			buildSupply(0);

		updateResources();
	}

	if (getMineralStatus() == PLENTY)
	{
		///Broodwar << "IM RICH IM RICH IM RICH" << endl;
	}

	updateResources();

	util.makeAllIdlesWork();

	if (isThereAvailableMinerals() && getSupplyStatus() != PLENTY || getSupplyStatus() != FULL)
	{
		makeIdleNexusBuildWorkers();
		updateResources();
	}
}

void ProductionManager::makeIdleNexusBuildWorkers()
{
	if (Broodwar->self()->isDefeated() || Broodwar->self()->isVictorious()) return;

	for (auto &u : Broodwar->self()->getUnits())
	{
		if (u->getType().isResourceDepot() && u) // A resource depot is a Command Center, Nexus, or Hatchery
		{
			if ((u->isIdle() && !u->train(u->getType().getRace().getWorker())) || !isThereAvailableMinerals())
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
				if (reserveUnitPrice(u->getType().getRace().getWorker()))
					Broodwar << "Reservou UM Worker (50)" << endl;
			}
			// Order the depot to construct more workers! But only when it is idle.
		}
	}
	// closure: unit iterator
}

ProductionManager::~ProductionManager()
{
}