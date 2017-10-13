#include "CombatManager.h"

CombatManager::CombatManager()
{
}

void CombatManager::init(ProductionManager prM)
{
	prManager_ptr = &prM;
	
}

void CombatManager::updateArmyQuantity()
{
	for (auto & u : BWAPI::Broodwar->self()->getUnits())
	{
		if (!u->exists()) continue;

		//if it is worker we dont count it
		if (u->getType() == Broodwar->self()->getRace().getWorker()) continue;

		if (u->getType().isBuilding() || !u->isCompleted() || u->isConstructing()) continue;

		if (u->canAttack()) allArmy.push_back(u);
	}

	
	//update dps values
	for (int i = 0; i < getAttackArmy().size(); i++)
	{
		if(i == 0) attackArmy_DPS = 0;

		if (getAttackArmy()[i] == nullptr || !getAttackArmy()[i]->exists())
		{
			getAttackArmy().erase(getAttackArmy().begin() + i);
			i = 0;
			continue;
		}
		attackArmy_DPS += getAttackArmy()[i]->getType().maxGroundHits();

	}

	for (int i = 0; i < defenseArmy.size(); i++)
	{
		if (i == 0) defenseArmy_DPS = 0;

		if (defenseArmy[i] == nullptr || !defenseArmy[i]->exists())
		{
			defenseArmy.erase(defenseArmy.begin() + i);
			i = 0;
			continue;
		}
		defenseArmy_DPS += defenseArmy[i]->getType().maxGroundHits();

	}

	for (int i = 0; i < swatArmy.size(); i++)
	{

		if (i == 0) swatArmy_DPS = 0;

		if (swatArmy[i] == nullptr || !swatArmy[i]->exists())
		{
			swatArmy.erase(swatArmy.begin() + i);
			i = 0;
			continue;
		}
		swatArmy_DPS += swatArmy[i]->getType().maxGroundHits();

	}

	zealotRush = prManager_ptr->isZealotRush();
}

void CombatManager::update()
{
	updateArmyQuantity();

	if (zealotRush)
	{

		if (getAttackArmy().size() >= 5)
		{
			Unitset u;
			for (int i = 0; i < getAttackArmy().size(); i++)
			{
				if (u.size() < 5)
				{
					u.insert(getAttackArmy()[i]);
				}
			}

			if(getAttackArmy().size() >= 5)
			getAttackSets().push_back(u);
		}

		//check for sets that can attack
		for (int i = 0; i < getAttackSets().size(); i++)
		{
			if (getAttackSets()[i].size() >= 5)
			{
				//attackSets[i].smartattack(getClosestEnemyNexus(), Broodwar->enemy()->getRace()->getWorker());

				//attackSets[i].smartattack(getClosestEnemyNexus(), Broodwar->enemy()->getRace()->getWorker());
				for (const auto& setUnit : getAttackSets()[i])
				{
					if (!setUnit->exists()) continue;
				
					SmartAttackMove(setUnit, getClosestEnemyNexus()->getPosition());
				}
				

			}
		}
	}

}

bool CombatManager::pushAttackArmy(Unit u)
{
	if (!u || !u->exists()) return false;

	getAttackArmy().push_back(u);

	updateArmyQuantity();
	return true;

}

BWAPI::Unit CombatManager::getClosestEnemyNexus()
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


void CombatManager::SmartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target)
{

	if (!attacker || !target)
	{
		return;
	}

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || attacker->isAttackFrame())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit &&	currentCommand.getTarget() == target)
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->attack(target);
	
}

void CombatManager::SmartAttackMove(BWAPI::Unit attacker, const BWAPI::Position & targetPosition)
{
	//UAB_ASSERT(attacker, "SmartAttackMove: Attacker not valid");
	//UAB_ASSERT(targetPosition.isValid(), "SmartAttackMove: targetPosition not valid");

	if (!attacker || !targetPosition.isValid())
	{
		return;
	}

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || attacker->isAttackFrame())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to attack this target, ignore this command
	if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Move &&	currentCommand.getTargetPosition() == targetPosition)
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->attack(targetPosition);
	
}

void CombatManager::SmartMove(BWAPI::Unit attacker, const BWAPI::Position & targetPosition)
{
	//UAB_ASSERT(attacker, "SmartAttackMove: Attacker not valid");
	//UAB_ASSERT(targetPosition.isValid(), "SmartAttackMove: targetPosition not valid");

	if (!attacker || !targetPosition.isValid())
	{
		return;
	}

	// if we have issued a command to this unit already this frame, ignore this one
	if (attacker->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount() || attacker->isAttackFrame())
	{
		return;
	}

	// get the unit's current command
	BWAPI::UnitCommand currentCommand(attacker->getLastCommand());

	// if we've already told this unit to move to this position, ignore this command
	if ((currentCommand.getType() == BWAPI::UnitCommandTypes::Move) && (currentCommand.getTargetPosition() == targetPosition) && attacker->isMoving())
	{
		return;
	}

	// if nothing prevents it, attack the target
	attacker->move(targetPosition);
	
}
CombatManager::~CombatManager()
{
}