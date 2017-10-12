#include "Utilities.h"

Utilities::Utilities()
{
}

bool Utilities::isUnitDisabled(Unit u)
{
	// Ignore the unit if it no longer exists
	// Make sure to include this block when handling any Unit pointer!
	if (!u->exists() || !u)
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

Utilities::~Utilities()
{
}