#include "ProductionManager.h"



ProductionManager::ProductionManager()
{
	speedtest = 5;
	hello = "Hello World";
}

void ProductionManager::updateResources()
{
	minerals = Broodwar->self()->minerals();
	supply = Broodwar->self()->supplyUsed();
	max_supply = Broodwar->self()->supplyTotal();
	gas = Broodwar->self()->gas();


	//mineral sorting
	{

		if (minerals == 0)
		{
			minerals_status = NONE;

		}
		else if (minerals > 0 && minerals <= 400)
		{
			minerals_status = LITTLE;
		}
		else if (minerals > 400 && minerals <= 1000)
		{
			minerals_status = SOME;
		}
		else if (minerals > 1000 && minerals <= 2500)
		{
			minerals_status = PLENTY;
		}
		else if (minerals > 2500 && minerals <= 70000)
		{
			minerals_status = FULL;
		}

	}

	//gas sorting
	{

		if (gas == 0)
		{
			gas_status = NONE;

		}
		else if (gas > 0 && gas <= 400)
		{
			gas_status = LITTLE;
		}
		else if (gas > 400 && gas <= 1000)
		{
			gas_status = SOME;
		}
		else if (gas > 1000 && gas <= 2500)
		{
			gas_status = PLENTY;
		}
		else if (gas > 2500 && gas <= 70000)
		{
			gas_status = FULL;
		}

	}
	
	//supply sorting
	{
		float coef;
		float percentage;

		/*
		max_supply -- 100 %
		supply --- x

		 x max_supply == 100 supply
		 x == (100 supply) / max_supply
		*/
		percentage = 100 * supply / max_supply;
		if (percentage <= 0)
		{
			gas_status = NONE;

		}
		else if (percentage > 0 && percentage <= 30)
		{
			supply_status = LITTLE;
		}
		else if (percentage > 30 && percentage <= 80)
		{
			supply_status = SOME;
		}
		else if (percentage > 80 && percentage < 100)
		{
			supply_status = PLENTY;
		}
		else if (percentage >= 100)
		{
			supply_status = FULL;
		}

	}
	


}

void ProductionManager::update()
{
	updateResources();

	if (minerals_status == PLENTY)
	{
		//Broodwar << "IM RICH IM RICH IM RICH" << endl;
	}
}


ProductionManager::~ProductionManager()
{
}
