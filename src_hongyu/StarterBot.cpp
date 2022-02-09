﻿#include "StarterBot.h"
#include "Tools.h"
#include "MapTools.h"
#include "map.h"

//update
#include <queue>
#include<list>
using namespace std;
typedef pair<int, BWAPI::UnitType> p; //the moment to build unit
typedef pair<int, BWAPI::UpgradeType> q; //the moment to upgrade
BWAPI::Unit m_scout = nullptr;

struct cmp {
    bool operator()(const p p1, const p p2) {
        return p1.first > p2.first;
    }
};

struct cmp1 {
    bool operator()(const q p1, const q p2) {
        return p1.first > p2.first;
    }
};

priority_queue<p, vector<p>, cmp> building_order;
priority_queue<p, vector<p>, cmp> train_order;
priority_queue<q, vector<q>, cmp1> upgrade_order;
//end update

StarterBot::StarterBot()
{
    
}

// Called when the bot starts!
void StarterBot::onStart()
{
    // Set our BWAPI options here    
	BWAPI::Broodwar->setLocalSpeed(10);
    BWAPI::Broodwar->setFrameSkip(0);
    
    // Enable the flag that tells BWAPI top let users enter input while bot plays
    BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

    // Call MapTools OnStart
    m_mapTools.onStart();

    //update
    initialStrategy();
    //end update

    BWEM::Map::Instance().Initialize(BWAPI::BroodwarPtr);
}

// Called whenever the game ends and tells you if you won or not
void StarterBot::onEnd(bool isWinner) 
{
    std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
}

// Called on each frame of the game
void StarterBot::onFrame()
{
    // Update our MapTools information
    m_mapTools.onFrame();

    // Send our idle workers to mine minerals so they don't just stand there
    sendIdleWorkersToMinerals();
        
    // Train more workers so we can gather more income
    trainAdditionalWorkers();

    // Build more supply if we are going to run out soon
    buildAdditionalSupply();

    // Draw unit health bars, which brood war unfortunately does not do
    Tools::DrawUnitHealthBars();

    // Draw some relevent information to the screen to help us debug the bot
    drawDebugInformation();

    //update
    build();
    train();
    upgrade();
    check();
    if (BWAPI::Broodwar->self()->supplyUsed() > 16) { StarterBot::sendScout(); }
    //end update
}

// Send our idle workers to mine minerals so they don't just stand there
void StarterBot::sendIdleWorkersToMinerals()
{
    // Let's send all of our starting workers to the closest mineral to them
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker() && unit->isIdle())
        {
            // Get the closest mineral to this worker unit
            BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());

            // If a valid mineral was found, right click it with the unit in order to start harvesting
            if (closestMineral) { unit->rightClick(closestMineral); }
        }
    }
}

// Train more workers so we can gather more income
void StarterBot::trainAdditionalWorkers()
{
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersWanted = 20;
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());
    if (workersOwned < workersWanted)
    {
        // get the unit pointer to my depot
        const BWAPI::Unit myDepot = Tools::GetDepot();

        // if we have a valid depot unit and it's currently not training something, train a worker
        // there is no reason for a bot to ever use the unit queueing system, it just wastes resources
        if (myDepot && !myDepot->isTraining()) { myDepot->train(workerType); }
    }
}

// Build more supply if we are going to run out soon
void StarterBot::buildAdditionalSupply()
{
    // Get the amount of supply supply we currently have unused
    const int unusedSupply = Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed();

    // If we have a sufficient amount of supply, we don't need to do anything
    if (unusedSupply >= 2) { return; }

    // Otherwise, we are going to build a supply provider
    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

    const bool startedBuilding = Tools::BuildBuilding(supplyProviderType);
    if (startedBuilding)
    {
        BWAPI::Broodwar->printf("Started Building %s", supplyProviderType.getName().c_str());
    }
}

// Draw some relevent information to the screen to help us debug the bot
void StarterBot::drawDebugInformation()
{
    BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 10), "Hello, World!\n");
    Tools::DrawUnitCommands();
    Tools::DrawUnitBoundingBoxes();
}

// Called whenever a unit is destroyed, with a pointer to the unit
void StarterBot::onUnitDestroy(BWAPI::Unit unit)
{
	
}

// Called whenever a unit is morphed, with a pointer to the unit
// Zerg units morph when they turn into other units
void StarterBot::onUnitMorph(BWAPI::Unit unit)
{
	
}

// Called whenever a text is sent to the game by a user
void StarterBot::onSendText(std::string text) 
{ 
    if (text == "/map")
    {
        m_mapTools.toggleDraw();
    }
}

// Called whenever a unit is created, with a pointer to the destroyed unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void StarterBot::onUnitCreate(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit finished construction, with a pointer to the unit
void StarterBot::onUnitComplete(BWAPI::Unit unit)
{
	
}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void StarterBot::onUnitShow(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit gets hidden, with a pointer to the destroyed unit
// This is usually triggered when units enter the fog of war and are no longer visible
void StarterBot::onUnitHide(BWAPI::Unit unit)
{ 
	
}

// Called whenever a unit switches player control
// This usually happens when a dark archon takes control of a unit
void StarterBot::onUnitRenegade(BWAPI::Unit unit)
{ 
	
}


//update
bool StarterBot::send1WorkerToGas()
{
    // Let's send all of our starting workers to the closest mineral to them
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    const BWAPI::Unit assimilator = Tools::GetUnitOfType(BWAPI::UnitTypes::Protoss_Assimilator);
    if (assimilator == nullptr) { return false; }

    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
    for (auto& unit : myUnits)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere
        if (unit->getType().isWorker())// �����ũ��һֱ��󣬾ͻ�ʲô������
        {
            unit->rightClick(assimilator);
            return true;
        }
    }
}

void StarterBot::initialStrategy() {
    building_order.push(make_pair(20, BWAPI::UnitTypes::Protoss_Gateway));
    building_order.push(make_pair(22, BWAPI::UnitTypes::Protoss_Cybernetics_Core));
    building_order.push(make_pair(26, BWAPI::UnitTypes::Protoss_Assimilator));

    train_order.push(make_pair(26, BWAPI::UnitTypes::Protoss_Zealot));
    train_order.push(make_pair(36, BWAPI::UnitTypes::Protoss_Dragoon));

    upgrade_order.push(make_pair(40, BWAPI::UpgradeTypes::Singularity_Charge));
}
//����
void StarterBot::build() {
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    if (!building_order.empty()) {
        const p b1 = building_order.top();
        if (supply >= b1.first) {//�˿ڹ���
            const bool startedBuilding = Tools::BuildBuilding(b1.second);// ���ؽ����Ƿ�ʼ����
            if (startedBuilding)
            {
                BWAPI::Broodwar->printf("Started Building %s", BWAPI::UnitTypes::Protoss_Gateway.getName().c_str());
                building_order.pop(); //��������Ӷ������ӵ�
            }

        }
    }
}


void StarterBot::train() {
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    if (!train_order.empty()) {
        const p b1 = train_order.top();
        if (supply >= b1.first) {//�˿ڹ���

            const bool startedtrain = Tools::train_unit(b1.second);// ���ؽ����Ƿ�ʼ����
            if (startedtrain)
            {
                BWAPI::Broodwar->printf("Started train %s", b1.second.getName().c_str());
                train_order.pop(); //��������Ӷ������ӵ�
            }

        }
    }
}


void StarterBot::upgrade() {
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    if (!upgrade_order.empty()) {
        const q b1 = upgrade_order.top();
        if (supply >= b1.first) {//�˿ڹ���
            const bool startedupgrade = Tools::upgrade(b1.second);// ���ؽ����Ƿ�ʼ����
            if (startedupgrade)
            {
                BWAPI::Broodwar->printf("Started upgrade %s", b1.second.getName().c_str());
                upgrade_order.pop(); //��������Ӷ������ӵ�
            }

        }
    }
}

void StarterBot::check() {
    if (Tools::GetUnitOfType(BWAPI::UnitTypes::Protoss_Assimilator) != nullptr) {
        const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        int workers_on_assmilator = 0;
        for (auto& unit : myUnits)
        {
            // Check the unit type, if it is an idle worker, then we want to send it somewhere
            if (unit->getType().isWorker() && unit->getTarget() != nullptr && unit->getTarget()->getType() == BWAPI::UnitTypes::Protoss_Assimilator)// �����ũ��һֱ��󣬾ͻ�ʲô������
            {
                workers_on_assmilator++;
            }
        }
        if (workers_on_assmilator < 3) {
            send1WorkerToGas();
            workers_on_assmilator++;
            if (workers_on_assmilator >= 3) { return; }
            //for (auto& unit : myUnits)
            //{
            //    // Check the unit type, if it is an idle worker, then we want to send it somewhere
            //    if (unit->getType().isWorker())// �����ũ��һֱ��󣬾ͻ�ʲô������
            //    {
            //        // Get the closest mineral to this worker unit
            //        BWAPI::Unit assimilator = Tools::GetUnitOfType(BWAPI::UnitTypes::Protoss_Assimilator);

            //        // If a valid mineral was found, right click it with the unit in order to start harvesting
            //        unit->rightClick(assimilator);
            //        workers_on_assmilator++;
            //        if (workers_on_assmilator >= 3) { return; }
            //    }

            //}
        }

    }
}

void StarterBot::setScout(BWAPI::Unit unit) 
{
    if (m_scout)
    {
        return;
    }
    m_scout = unit;
}

void StarterBot::sendScout()
{
    if (!m_scout||!m_scout->exists())
    {
        const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : myUnits)
        {
            if (unit->getType().isWorker()) 
            {
                StarterBot::setScout(unit);
            }
        }
    }

    for (auto location : BWAPI::Broodwar->getStartLocations())
    {
        if (!BWAPI::Broodwar->isExplored(location))
        {
            m_scout->move(BWAPI::Position(location));
            return;
        }
    }
}
