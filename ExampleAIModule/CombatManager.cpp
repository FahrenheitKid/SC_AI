#include "CombatManager.h"

CombatManager::CombatManager()
{
}

void CombatManager::init(ProductionManager prM)
{
	prManager_ptr = &prM;
	attackTroopSize = 6;
}

void CombatManager::updateArmyQuantity()
{
	for (auto & u : BWAPI::Broodwar->self()->getUnits())
	{
		if (!u->exists()) continue;

		//if it is worker we dont count it
		if (u->getType() == Broodwar->self()->getRace().getWorker()) continue;

		if (u->getType().isBuilding() || !u->isCompleted() || u->isConstructing()) continue;

		if (!isThisUnitInThatVector(u, allArmy)) allArmy.push_back(u);
	}

	if (zealotRush)
	{
		for (auto &u : Broodwar->self()->getUnits())
		{
			if (!u || !u->exists() || u->getType() != UnitTypes::Protoss_Zealot) continue;

			if (!isThisUnitInThatVector(u, attackArmy))
				attackArmy.push_back(u);
		}
	}

	for (int i = 0; i < attackSets.size(); i++)
	{
		if (attackSets[i].size() < attackTroopSize)
		{
			if (attackSets.size() > i)
				attackSets.erase(attackSets.begin() + i);
		}
	}

	//update dps values
	for (int i = 0; i < attackArmy.size(); i++)
	{
		if (i == 0) attackArmy_DPS = 0;

		if (attackArmy[i] == nullptr || !attackArmy[i]->exists())
		{
			attackArmy.erase(attackArmy.begin() + i);
			i = 0;
			continue;
		}
		attackArmy_DPS += attackArmy[i]->getType().maxGroundHits();
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

	//make idle units attack as troop
	makeIdleArmyAttack(attackTroopSize);

	//if zealotrush strat
	if (zealotRush)
	{
		//fill attacksets
		if (attackArmy.size() >= attackTroopSize)
		{
			Unitset u;
			for (int i = 0; i < attackArmy.size(); i++)
			{
				if (u.size() < attackTroopSize)
				{
					u.insert(attackArmy[i]);
				}
			}

			if (u.size() >= attackTroopSize && !isThisUnitInThatVector(u, attackSets))
				attackSets.push_back(u);
		}

		//check for sets that can attack
		for (int i = 0; i < attackSets.size(); i++)
		{
			if (attackSets[i].size() >= attackTroopSize)
			{
				for (const auto& setUnit : attackSets[i])
				{
					if (!setUnit->exists()) continue;

					Position bias(0, 0);
					if (!enemyBasePosition || enemyBasePosition == bias) continue;

					int nexDis = enemyBasePosition.getDistance(setUnit->getPosition());
					Unit closest = setUnit->getClosestUnit(Filter::IsEnemy);

					if (!closest || !closest->exists()) continue;

					lastEnemySeenPosition = closest->getPosition();
					//attack the nexus or if cant find nexus and near nexus position, attack last seen enemy
					if (nexDis <= 20 && closest->getType() != Broodwar->enemy()->getRace().getResourceDepot())
					{
						SmartAttackMove(setUnit, lastEnemySeenPosition);
					}
					else
					{
						SmartAttackMove(setUnit, enemyBasePosition);
					}
				}
			}
		}
	}
}

void CombatManager::makeIdleArmyAttack(int idle_amount)
{
	int idle_count = 0;
	vector<Unit> idles;
	//fill with idle units
	for (auto &a : allArmy)
	{
		if (!a->exists()) continue;

		if (a->isIdle() || (!a->isAttacking() && !a->isMoving())) idles.push_back(a);
	}

	if (idles.size() > idle_amount) return;

	//make idle units attack
	for (auto &a : idles)
	{
		if (!a->exists()) continue;

		if (idles.size() >= attackTroopSize)
		{
			if (!a->exists()) continue;

			Position bias(0, 0);
			if (!enemyBasePosition || enemyBasePosition == bias) continue;

			int nexDis = enemyBasePosition.getDistance(a->getPosition());
			Unit closest = a->getClosestUnit(Filter::IsEnemy);

			if (!closest || !closest->exists()) continue;

			lastEnemySeenPosition = closest->getPosition();
			//attack the nexus or if cant find nexus and near nexus position, attack last seen enemy
			if (nexDis <= 20 && closest->getType() != Broodwar->enemy()->getRace().getResourceDepot())
			{
				SmartAttackMove(a, lastEnemySeenPosition);
			}
			else
			{
				SmartAttackMove(a, enemyBasePosition);
			}
		}
	}
}

bool CombatManager::pushAttackArmy(Unit u)
{
	if (!u || !u->exists()) return false;

	attackArmy.push_back(u);

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

void CombatManager::SmartAttackMove(BWAPI::Unit attacker, const BWAPI::Position  targetPosition)
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

void CombatManager::SmartMove(BWAPI::Unit attacker, const BWAPI::Position  targetPosition)
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

bool CombatManager::isThisUnitInThatVector(Unit u, std::vector<BWAPI::Unit> val)
{
	if (!u || !u->exists() || val.empty()) return false;

	for (int i = 0; i < val.size(); i++)
	{
		if (u == val[i]) return true;
	}

	return false;
}

bool CombatManager::isThisUnitInThatVector(Unitset u, std::vector<BWAPI::Unitset> val)
{
	if (u.empty() || val.empty()) return false;

	for (int i = 0; i < val.size(); i++)
	{
		if (u == val[i]) return true;
	}

	return false;
}

bool CombatManager::isThisUnitInThatUnisetVector(Unit u, std::vector<BWAPI::Unitset> val)
{
	if (!u->exists() || val.empty()) return false;

	for (int i = 0; i < val.size(); i++)
	{
		for (const auto& setUnit : val[i])
		{
			if (!setUnit->exists()) continue;

			if (setUnit == u) return true;

			//if(getClosestEnemyNexus()->exists() && setUnit->exists())
			//SmartAttackMove(setUnit, getClosestEnemyNexus()->getPosition());
		}
	}

	return false;
}

CombatManager::~CombatManager()
{
}