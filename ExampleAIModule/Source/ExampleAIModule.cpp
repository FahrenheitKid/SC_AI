#include "ExampleAIModule.h"
#include <iostream>
#include <ProductionManager.h>
#include <StrategyManager.h>
#include <CombatManager.h>
#include <Utilities.h>
using namespace BWAPI;
using namespace Filter;

StrategyManager sManager;
ProductionManager prManager;
CombatManager cManager;

void ExampleAIModule::onStart()
{
	// Hello World!

	Broodwar->sendText(prManager.hello.c_str());

	//here we initialize the managers
	prManager.constructorInit();
	sManager.init(prManager);
	cManager.init(prManager);

	//Broodwar->setLocalSpeed(0);
	// Print the map name.
	// BWAPI returns std::string when retrieving a string, don't forget to add .c_str() when printing!
	Broodwar << "The map is " << Broodwar->mapName() << "!" << std::endl;

	// Enable the UserInput flag, which allows us to control the bot and type messages.
	Broodwar->enableFlag(Flag::UserInput);

	// Uncomment the following line and the bot will know about everything through the fog of war (cheat).
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	// Set the command optimization level so that common commands can be grouped
	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(2);
	//Broodwar->self()->mi
	// Check if this is a replay
	if (Broodwar->isReplay())
	{
		// Announce the players in the replay
		Broodwar << "The following players are in this replay:" << std::endl;

		// Iterate all the players in the game using a std:: iterator
		Playerset players = Broodwar->getPlayers();
		for (auto p : players)
		{
			// Only print the player if they are not an observer
			if (!p->isObserver())
				Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
		}
	}
	else // if this is not a replay
	{
		// Retrieve you and your enemy's races. enemy() will just return the first enemy.
		// If you wish to deal with multiple enemies then you must use enemies().
		if (Broodwar->enemy()) // First make sure there is an enemy
			Broodwar << "The matchup is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;
	}

	//prManager.pushStandartBuildingQueue();
}

void ExampleAIModule::onEnd(bool isWinner)
{
	// Called when the game ends
	if (isWinner)
	{
		// Log your win here!
	}
}

void ExampleAIModule::onFrame()
{
	if (Broodwar->self()->isDefeated()) return;

	string texttest = "FPS: %d";
	// Display the game frame rate as text in the upper left area of the screen
	Broodwar->drawTextScreen(200, 0, texttest.c_str(), Broodwar->getFPS());
	Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS());
	Broodwar->drawTextScreen(200, 40, "reserved_minerals: %d | Minerals: %d", prManager.getReservedMinerals(), prManager.getMinerals());
	Broodwar->drawTextScreen(50, 60, "attackArmySize: %d | AttackSetsSize: %d | AllArmySize: %d", cManager.attackArmy.size(), cManager.attackSets.size(), cManager.allArmy.size());
	Broodwar->drawTextScreen(50, 800, "EnemyUnitsCount: %d | EnemyNexusCount: %d | EnemyWorkerCount: %d", Broodwar->enemy()->allUnitCount(), Broodwar->enemy()->allUnitCount(Broodwar->enemy()->getRace().getResourceDepot()), Broodwar->enemy()->allUnitCount(Broodwar->enemy()->getRace().getWorker()));
	Broodwar->drawTextScreen(50, 100, "enemyBase: (%d , %d) | lastEnemyBase: (%d , %d)", cManager.enemyBasePosition.x, cManager.enemyBasePosition.y, prManager.lastEnemyBaseLocation.x, prManager.lastEnemyBaseLocation.y);
	if (prManager.getBuildingsQueue().size() > 0)
	{
		buildingInfo nextB = prManager.getBuildingsQueue().front();

		string text = "Next building is a: ";
		text += nextB.building.getName();
		text += " | Quantity %d | timing: %d | cost: %d";

		Broodwar->drawTextScreen(100, 70, text.c_str(),
			nextB.quantity, nextB.supply_timing, nextB.building.mineralPrice());
	}
	else
	{
		//Broodwar << "VAZIOOOOO " << endl;
	}

	// Return if the game is a replay or is paused
	if (Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self())
		return;

	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if (Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
		return;

	// here the main update functions execute and make the bot alive!
	prManager.update();
	cManager.update();

	//this part updates the last enemy nexus position in managers
	if (Broodwar->enemy()->allUnitCount() > 0)
	{
		for (auto &u : Broodwar->enemy()->getUnits())
		{
			if (!u->exists()) continue;
			if (u->getType() == Broodwar->enemy()->getRace().getResourceDepot())
			{
				//Broodwar << "achei nexus" << endl;
				int x = u->getPosition().x;
				int y = u->getPosition().y;
				//Broodwar << "OnFrame::last enemy set: (" + to_string(x) + ", " + to_string(y) + ")" << endl;
				cManager.enemyBasePosition = u->getPosition();
				prManager.lastEnemyBaseLocation = u->getPosition();
			}

			cManager.lastEnemySeenPosition = u->getPosition();
		}
	}
}

void ExampleAIModule::onSendText(std::string text)
{
	// Send the text to the game if it is not being processed.
	Broodwar->sendText("%s", text.c_str());

	// Make sure to use %s and pass the text as a parameter,
	// otherwise you may run into problems when you use the %(percent) character!
}

void ExampleAIModule::onReceiveText(BWAPI::Player player, std::string text)
{
	// Parse the received text
	Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void ExampleAIModule::onPlayerLeft(BWAPI::Player player)
{
	// Interact verbally with the other players in the game by
	// announcing that the other player has left.
	Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void ExampleAIModule::onNukeDetect(BWAPI::Position target)
{
	// Check if the target is a valid position
	if (target)
	{
		// if so, print the location of the nuclear strike target
		Broodwar << "Nuclear Launch Detected at " << target << std::endl;
	}
	else
	{
		// Otherwise, ask other players where the nuke is!
		Broodwar->sendText("Where's the nuke?");
	}

	// You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
}

void ExampleAIModule::onUnitDiscover(BWAPI::Unit unit)
{
}

void ExampleAIModule::onUnitEvade(BWAPI::Unit unit)
{
}

void ExampleAIModule::onUnitShow(BWAPI::Unit unit)
{
}

void ExampleAIModule::onUnitHide(BWAPI::Unit unit)
{
}

void ExampleAIModule::onUnitCreate(BWAPI::Unit unit)
{
	if (Broodwar->isReplay())
	{
		// if we are in a replay, then we will print out the build order of the structures
		if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
		{
			int seconds = Broodwar->getFrameCount() / 24;
			int minutes = seconds / 60;
			seconds %= 60;
			Broodwar->sendText("%.2d:%.2d: %s creates a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
		}
	}
}

void ExampleAIModule::onUnitDestroy(BWAPI::Unit unit)
{
	if (!unit->exists()) return;

	if (unit->getPlayer()->isEnemy(Broodwar->self()))
	{
		//if we killed a nexus search for the next one
		if (unit->getPlayer()->getRace().getResourceDepot() == unit->getType())
		{
			//prManager.holdScouting = false;
		}
	}
	else
	{
		if ((cManager.attackTroopSize + 1 + Broodwar->self()->completedUnitCount(Broodwar->self()->getRace().getWorker())) < 190)
			cManager.attackTroopSize++;
	}
}

void ExampleAIModule::onUnitMorph(BWAPI::Unit unit)
{
	if (Broodwar->isReplay())
	{
		// if we are in a replay, then we will print out the build order of the structures
		if (unit->getType().isBuilding() && !unit->getPlayer()->isNeutral())
		{
			int seconds = Broodwar->getFrameCount() / 24;
			int minutes = seconds / 60;
			seconds %= 60;
			Broodwar->sendText("%.2d:%.2d: %s morphs a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
		}
	}
}

void ExampleAIModule::onUnitRenegade(BWAPI::Unit unit)
{
}

void ExampleAIModule::onSaveGame(std::string gameName)
{
	Broodwar << "The game was saved to \"" << gameName << "\"" << std::endl;
}

void ExampleAIModule::onUnitComplete(BWAPI::Unit unit)
{
	if (!unit || !unit->exists()) return;

	if (Broodwar->self()->isDefeated()) return;

	//for now, if it isn't our units, we don't care
	if (unit->getPlayer() != Broodwar->self()) return;

	//we should remove the reserved resources for that unit once it is completed/finally created
	if (prManager.dereserveUnitPrice(unit->getType()))
	{
		//if that unit was the next building to create in the build order queue, we should decrement it from there
		if (prManager.isUnitInQueueOrder(unit))
		{
			if (prManager.decrementFirstInQueueOrder(unit))
			{
				Broodwar << "removido da queue order: " + unit->getType().getName() << endl;
			}
		}
		//	Broodwar << "desreservou UM " + unit->getType().getName() << endl;
	}

	//if we are making zealot rush and it is zealot, add them to the army
	if (prManager.isZealotRush() && unit->getType() == UnitTypes::Protoss_Zealot)
	{
		//Broodwar << "adicionou zealot na army" << endl;
	}
}