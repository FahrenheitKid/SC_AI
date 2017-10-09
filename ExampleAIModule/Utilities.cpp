#include "Utilities.h"

Utilities::Utilities()
{
}

bool Utilities::isUnitDisabled(Unit u)
{
	// Ignore the unit if it no longer exists
	// Make sure to include this block when handling any Unit pointer!
	if (!u->exists())
		return true;

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

void Utilities::makeAllIdlesWork()
{
	for (auto &u : Broodwar->self()->getUnits())
	{
		if (isUnitDisabled(u))
			continue;

		// Finally make the unit do some stuff!

		// If the unit is a worker unit
		if (u->getType().isWorker())
		{
			// if our worker is idle
			if (u->isIdle() && !u->isConstructing())
			{
				// Order workers carrying a resource to return them to the center,
				// otherwise find a mineral patch to harvest.
				if (u->isCarryingGas() || u->isCarryingMinerals())
				{
					u->returnCargo();
				}
				else if (!u->getPowerUp())  // The worker cannot harvest anything if it
				{                             // is carrying a powerup such as a flag
											  // Harvest from the nearest mineral patch or gas refinery
											  //if (!u->gather(u->getClosestUnit()->getType().isMineralField() || u->getClosestUnit()->getType().isRefinery()))
					if (!u->gather(u->getClosestUnit(Filter::IsMineralField || Filter::IsRefinery)))
					{
						// If the call fails, then print the last error message
						Broodwar << Broodwar->getLastError() << std::endl;
					}
				} // closure: has no powerup
			} // closure: if idle
		}
	}
}

Utilities::~Utilities()
{
}