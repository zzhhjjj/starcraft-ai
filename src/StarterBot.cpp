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

    if (BWAPI::Broodwar->self()->supplyUsed() > 10 && !m_data.detecte_enemy) { StarterBot::sendScout(); }
    front_strategy();

    BWAPI::Unitset targets = BWAPI::Broodwar->enemy()->getUnits();

    //m_meleeManager.Strategy(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()), m_data.enemy_base, 80);
    
    m_meleeManager.defendBase(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));

    if (m_data.front_pylon) { m_meleeManager.setRallyPoint(BWAPI::Position(m_data.front_pylon_pos)); }
    else { m_meleeManager.setRallyPoint(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation())); }

    AttackGroup attackGp = m_meleeManager.getCombatGroup(m_data.CombatRallyPoint, 60, 3);
    m_meleeManager.attackBase(attackGp,m_data.enemy_base);  
    //end update
}

// Send our idle workers to mine minerals so they don't just stand there
//void StarterBot::sendIdleWorkersToMinerals()
//{
//    // Let's send all of our starting workers to the closest mineral to them
//    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
//    for (auto& unit : m_data.m_workers)
//    {
//        // Check the unit type, if it is an idle worker, then we want to send it somewhere
//        if ( unit->isIdle())
//        {
//            // Get the closest mineral to this worker unit
//            BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());
//
//            // If a valid mineral was found, right click it with the unit in order to start harvesting
//            if (closestMineral) { unit->rightClick(closestMineral); }
//            m_data.m_workerJobMap[unit] = m_data.Minerals;
//            /*m_data.m_workerMineralMap[unit] = closestMineral;*/
//        }
//    }
//}

// Send our idle workers to mine minerals so they don't just stand there
void StarterBot::sendIdleWorkersToMinerals()
{
    // Let's send all of our starting workers to the closest mineral to them
    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
    for (auto& unit : m_data.m_workers)
    {
        // Check the unit type, if it is an idle worker, then we want to send it somewhere

        if (m_data.m_workerJobMap[unit] == m_data.Idle)

        {
            // Get the closest mineral to this worker unit
            BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());

            // If a valid mineral was found, right click it with the unit in order to start harvesting
            if (closestMineral) { unit->rightClick(closestMineral); }
            m_data.m_workerJobMap[unit] = m_data.Minerals;
            /*m_data.m_workerMineralMap[unit] = closestMineral;*/
        }
    }

}

// Train more workers so we can gather more income
void StarterBot::trainAdditionalWorkers()
{
    const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
    const int workersWanted = 20;
    const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());
    if (workersOwned < workersWanted && m_data.current_mineral()>50)
    {
        // get the unit pointer to my depot
        const BWAPI::Unit myDepot = Tools::GetDepot();

        // if we have a valid depot unit and it's currently not training something, train a worker
        // there is no reason for a bot to ever use the unit queueing system, it just wastes resources
        if (myDepot && !myDepot->isTraining()) { myDepot->train(workerType); }
    }
}

// Build more supply if we are going to runbu out soon
void StarterBot::buildAdditionalSupply()
{
    // Get the amount of supply supply we currently have unused
    const int unusedSupply = Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed();

    // If we have a sufficient amount of supply, we don't need to do anything

    // early stage
    if ((unusedSupply >= 6 && BWAPI::Broodwar->self()->supplyUsed() <= 26) || m_data.current_mineral() < 100) { return; }
    // after
    if (unusedSupply >= 10 || m_data.current_mineral() < 100) { return; }

    // Otherwise, we are going to build a supply provider
    const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

    const bool startedBuilding = BuildBuilding(supplyProviderType);
    if (startedBuilding)
    {
        m_data.current_minus_mineral += supplyProviderType.mineralPrice();
        //BWAPI::Broodwar->printf("Started Building %s", supplyProviderType.getName().c_str());
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
    if (unit == m_scout) {
        m_scout = m_data.get_a_miner();
        m_data.m_workerJobMap[m_scout] = m_data.Scout;
    }
    if (unit->getType() == BWAPI::UnitTypes::Protoss_Pylon) {
        m_data.front_pylon = false;
        m_data.second_pylon = false;
    }
    if (unit->getType() == BWAPI::UnitTypes::Protoss_Gateway) {
        m_data.front_gateway--;
    }
    if (unit->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon) {
        m_data.front_canon--;
    }
    if (unit->getType() == m_data.worker_type) {
        m_data.m_workerJobMap.erase(unit);
    }	
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
    if (unit->getType() == m_data.worker_type) {
       
        m_data.m_workers.insert(unit);
        m_data.m_workerJobMap[unit] = m_data.Idle;
        /*m_data.m_workerDepotMap[unit] = Tools::GetClosestUnitTo(unit, m_data.m_depots);*/
    }
}

// Called whenever a unit finished construction, with a pointer to the unit
void StarterBot::onUnitComplete(BWAPI::Unit unit)
{
    if (unit->getType().getID() == BWAPI::UnitTypes::Protoss_Assimilator.getID()) {
        for (int i = 0; i < 3; i++) {
            BWAPI::Unit gaser = m_data.get_a_miner(unit->getPosition());
            if (gaser != nullptr) {
                gaser->rightClick(unit);
                m_data.m_workerJobMap[gaser] = m_data.Gas;
            }
            else {
                BWAPI::Broodwar->printf("can't find worker!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ");
            }           
        }
    }

    if (unit->getType() == m_data.worker_type) {

        m_data.m_workers.insert(unit);
        m_data.m_workerJobMap[unit] = m_data.Idle;
        /*m_data.m_workerDepotMap[unit] = Tools::GetClosestUnitTo(unit, m_data.m_depots);*/
    }
}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void StarterBot::onUnitShow(BWAPI::Unit unit)
{

    if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()) == true) {

        if (unit->getType().isBuilding() && m_data.detecte_enemy == false) {
            m_data.enemy_race = BWAPI::Broodwar->enemy()->getRace();
            m_data.enemy_building = unit;

            m_data.detecte_enemy = true;
            BWAPI::Position enemy_building_pos = unit->getPosition();
            m_data.enemy_base = unit->getPosition();
            BWAPI::Position scouter_pos = m_scout->getPosition();

            BWAPI::Broodwar->printf("find enenmy base!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ");
            BWAPI::Broodwar->printf("%s ", m_scout->getLastCommand().getType().c_str());

            m_data.front_pylon_pos = BWAPI::TilePosition(BWAPI::Position(1.5 * scouter_pos.x - 0.5 * enemy_building_pos.x, 1.5 * scouter_pos.y - 0.5 * enemy_building_pos.y).makeValid());

            m_scout->move(BWAPI::Position(1.7 * scouter_pos.x - 0.7 * enemy_building_pos.x, 1.7 * scouter_pos.y - 0.7 * enemy_building_pos.y));
        }
    }
    if (unit->getPlayer() == (BWAPI::Broodwar->self()) && unit->getType().isBuilding()) {

        m_data.current_minus_mineral -= unit->getType().mineralPrice();
    }
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
//bool StarterBot::send1WorkerToGas()
//{
//    // Let's send all of our starting workers to the closest mineral to them
//    // First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
//    const BWAPI::Unit assimilator = Tools::GetUnitOfType(BWAPI::UnitTypes::Protoss_Assimilator);
//    if (assimilator == nullptr) { return false; }
//
//    const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
//    for (auto& unit : myUnits)
//    {
//        // Check the unit type, if it is an idle worker, then we want to send it somewhere
//        if (unit->getType().isWorker())// �����ũ��һֱ��󣬾ͻ�ʲô������
//        {
//            unit->rightClick(assimilator);
//            return true;
//        }
//    }
//}

void StarterBot::initialStrategy() {
    building_order.push(make_pair(60, BWAPI::UnitTypes::Protoss_Cybernetics_Core));
    building_order.push(make_pair(60, BWAPI::UnitTypes::Protoss_Assimilator));
    building_order.push(make_pair(18, BWAPI::UnitTypes::Protoss_Forge));

    train_order.push(make_pair(32, BWAPI::UnitTypes::Protoss_Zealot));
    train_order.push(make_pair(36, BWAPI::UnitTypes::Protoss_Dragoon));

    upgrade_order.push(make_pair(40, BWAPI::UpgradeTypes::Singularity_Charge));

    /*building_order.push(make_pair(20, BWAPI::UnitTypes::Protoss_Gateway));
    building_order.push(make_pair(20, BWAPI::UnitTypes::Protoss_Gateway));
    building_order.push(make_pair(24, BWAPI::UnitTypes::Protoss_Cybernetics_Core));
    building_order.push(make_pair(2, BWAPI::UnitTypes::Protoss_Assimilator));

    train_order.push(make_pair(32, BWAPI::UnitTypes::Protoss_Zealot));
    
    train_order.push(make_pair(20, BWAPI::UnitTypes::Protoss_Zealot));
    train_order.push(make_pair(32, BWAPI::UnitTypes::Protoss_Zealot));
    train_order.push(make_pair(32, BWAPI::UnitTypes::Protoss_Zealot));
    train_order.push(make_pair(32, BWAPI::UnitTypes::Protoss_Zealot));
    train_order.push(make_pair(32, BWAPI::UnitTypes::Protoss_Zealot));
    train_order.push(make_pair(36, BWAPI::UnitTypes::Protoss_Dragoon));
    train_order.push(make_pair(36, BWAPI::UnitTypes::Protoss_Dragoon));
    train_order.push(make_pair(36, BWAPI::UnitTypes::Protoss_Dragoon));
    train_order.push(make_pair(36, BWAPI::UnitTypes::Protoss_Dragoon));
    train_order.push(make_pair(20, BWAPI::UnitTypes::Protoss_Dragoon));
    train_order.push(make_pair(36, BWAPI::UnitTypes::Protoss_Dragoon));
    train_order.push(make_pair(20, BWAPI::UnitTypes::Protoss_Dragoon));
    train_order.push(make_pair(36, BWAPI::UnitTypes::Protoss_Dragoon));
    train_order.push(make_pair(36, BWAPI::UnitTypes::Protoss_Dragoon));
    upgrade_order.push(make_pair(40, BWAPI::UpgradeTypes::Singularity_Charge));*/
}
//����
void StarterBot::build() {
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    if (!building_order.empty()) {
        const p b1 = building_order.top();
        if (supply >= b1.first && m_data.current_mineral()>b1.second.mineralPrice()) {
            //BWAPI::Broodwar->printf("Want to build %s, mineral:%d...%d", b1.second.getName().c_str(),m_data.current_mineral(),m_data.current_minus_mineral);
            //const bool startedBuilding = Tools::BuildBuilding(b1.second);
            const bool startedBuilding = BuildBuilding(b1.second);
           
            if (startedBuilding)
            {
                m_data.current_minus_mineral += b1.second.mineralPrice();
                //BWAPI::Broodwar->printf("Started Building %s", BWAPI::UnitTypes::Protoss_Gateway.getName().c_str());
                building_order.pop(); 
            }
        }
    }
}


void StarterBot::train() {
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    //if (!train_order.empty()) {
    //    const p b1 = train_order.top();
    //    if (supply >= b1.first && m_data.current_mineral()>b1.second.mineralPrice()) {//�˿ڹ���

    //        const bool startedtrain = Tools::train_unit(b1.second);// ���ؽ����Ƿ�ʼ����
    //        if (startedtrain)
    //        {
    //            BWAPI::Broodwar->printf("Started train %s", b1.second.getName().c_str());
    //            train_order.pop(); //��������Ӷ������ӵ�
    //        }

    //    }
    //}
    if (supply > 20 && supply < 80 && m_data.current_mineral() > BWAPI::UnitTypes::Protoss_Zealot.mineralPrice()) {
        const bool startedtrain = Tools::train_unit(BWAPI::UnitTypes::Protoss_Zealot);
        if (startedtrain)
        {
            BWAPI::Broodwar->printf("Started train %s", BWAPI::UnitTypes::Protoss_Zealot.getName().c_str());
            //train_order.pop();
        }
        else {
            BWAPI::Broodwar->printf("Training fail ");
        }
    }
    if (supply > 32 && m_data.current_mineral() > BWAPI::UnitTypes::Protoss_Dragoon.mineralPrice()) {
        const bool startedtrain = Tools::train_unit(BWAPI::UnitTypes::Protoss_Dragoon);
        if (startedtrain)
        {
            BWAPI::Broodwar->printf("Started train %s", BWAPI::UnitTypes::Protoss_Dragoon.getName().c_str());
            //train_order.pop();
        }
        else {
            BWAPI::Broodwar->printf("Training fail ");
        }
    }
}


void StarterBot::upgrade() {
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    if (!upgrade_order.empty()) {
        const q b1 = upgrade_order.top();
        if (supply >= b1.first && m_data.current_mineral() > b1.second.mineralPrice()) {//�˿ڹ���
            const bool startedupgrade = Tools::upgrade(b1.second);// ���ؽ����Ƿ�ʼ����
            if (startedupgrade)
            {
                //BWAPI::Broodwar->printf("Started upgrade %s", b1.second.getName().c_str());
                upgrade_order.pop(); //��������Ӷ������ӵ�
            }

        }
    }
}

void StarterBot::check() {
    

    
}

void StarterBot::setScout(BWAPI::Unit unit) 
{
    if (m_scout)
    {
        return;
    }
    m_scout = unit;
    m_data.m_workerJobMap[unit] = m_data.Scout;
}

void StarterBot::sendScout()
{
    if (!m_scout||!m_scout->exists())
    {
        /*const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
        for (auto& unit : myUnits)
        {
            if (unit->getType().isWorker()) 
            {
                StarterBot::setScout(unit);
            }
        }*/
        StarterBot::setScout(m_data.get_a_miner());

    }

    for (auto location : BWAPI::Broodwar->getStartLocations())
    {
        if (m_data.detecte_enemy == true) {
            BWAPI::Broodwar->printf("Enemy building is %s, position: %d, %d", m_data.enemy_building->getType().getName().c_str(), m_data.enemy_building->getPosition().x, m_data.enemy_building->getPosition().y);
            return;
        }
        if (!BWAPI::Broodwar->isExplored(location))
        {
            if (m_data.detecte_enemy == true) {
                BWAPI::Broodwar->printf("Enemy building is %s, position: %d, %d", m_data.enemy_building->getType().getName().c_str(), m_data.enemy_building->getPosition().x, m_data.enemy_building->getPosition().y);
                return;
            }
            m_scout->move(BWAPI::Position(location));
            return;
        }
    }
}

void StarterBot::front_strategy() {
    if (m_data.detecte_enemy) {

        if (!m_data.front_pylon && m_data.current_mineral() > BWAPI::UnitTypes::Protoss_Pylon.mineralPrice()) {
            BWAPI::Broodwar->printf("want to build front pylon  %d", m_data.current_mineral());
            BWAPI::TilePosition pylonpos = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::TilePosition(m_data.front_pylon_pos), 32, false);
            bool  succeed = m_scout->build(BWAPI::UnitTypes::Protoss_Pylon, pylonpos);
            if (succeed) {
                m_data.front_pylon_pos = pylonpos;
                m_data.current_minus_mineral += BWAPI::UnitTypes::Protoss_Pylon.mineralPrice();
                m_data.front_pylon = true;

            }
        }

        if (m_data.front_canon<3 && m_data.front_pylon && m_data.current_mineral() > BWAPI::UnitTypes::Protoss_Photon_Cannon.mineralPrice()) {

            BWAPI::Position wanted_pos = BWAPI::Position(0.9 * BWAPI::Position(m_data.front_pylon_pos).x + 0.1 * m_data.enemy_base.x, 0.9 * BWAPI::Position(m_data.front_pylon_pos).y + 0.1 * m_data.enemy_base.y);
            BWAPI::TilePosition cannonpos = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Protoss_Photon_Cannon, BWAPI::TilePosition(wanted_pos), 32, false);
            BWAPI::Broodwar->printf("want canon  (%d,%d) (%d,%d) (%d,%d)", m_data.front_pylon_pos.x, m_data.front_pylon_pos.y, BWAPI::TilePosition(m_data.front_pylon_pos).x, BWAPI::TilePosition(m_data.front_pylon_pos).y, cannonpos.x, cannonpos.y);
            bool  succeed = m_scout->build(BWAPI::UnitTypes::Protoss_Photon_Cannon, cannonpos);
            if (succeed) {
                m_data.current_minus_mineral += BWAPI::UnitTypes::Protoss_Photon_Cannon.mineralPrice();
                m_data.front_canon++;
            }
        }
        if (m_data.front_gateway < 3 && m_data.front_canon>1 && m_data.current_mineral() > BWAPI::UnitTypes::Protoss_Gateway.mineralPrice()) {
            BWAPI::Broodwar->printf("want to build front Gateway  %d", m_data.current_mineral());

            BWAPI::TilePosition gatewaypos = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::TilePosition(m_data.front_pylon_pos), 64, false);
            bool  succeed = m_scout->build(BWAPI::UnitTypes::Protoss_Gateway, gatewaypos.makeValid());
            if (succeed) {
                m_data.current_minus_mineral += BWAPI::UnitTypes::Protoss_Gateway.mineralPrice();
                m_data.front_gateway++;
            }
        }
        if (m_data.front_gateway > 0 && m_data.second_pylon == false && m_data.current_mineral() > BWAPI::UnitTypes::Protoss_Pylon.mineralPrice()) {
            BWAPI::Broodwar->printf("want to build front pylon  %d", m_data.current_mineral());
            BWAPI::Position wanted_pos = BWAPI::Position(0.5 * BWAPI::Position(m_data.front_pylon_pos).x + 0.5 * m_data.enemy_base.x, 0.5 * BWAPI::Position(m_data.front_pylon_pos).y + 0.5 * m_data.enemy_base.y);
            BWAPI::TilePosition pylonpos = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::TilePosition(wanted_pos), 32, false);
            bool  succeed = m_scout->build(BWAPI::UnitTypes::Protoss_Pylon, pylonpos);
            if (succeed) {

                m_data.current_minus_mineral += BWAPI::UnitTypes::Protoss_Pylon.mineralPrice();
                m_data.second_pylon = true;
            }
        }
        if (m_data.second_pylon && m_data.front_canon<8 && m_data.current_mineral() > BWAPI::UnitTypes::Protoss_Photon_Cannon.mineralPrice()) {
            BWAPI::Broodwar->printf("want to build front canon  %d", m_data.current_mineral());
            BWAPI::Position wanted_pos = BWAPI::Position(0.2 * BWAPI::Position(m_data.front_pylon_pos).x + 0.8 * m_data.enemy_base.x, 0.2 * BWAPI::Position(m_data.front_pylon_pos).y + 0.8 * m_data.enemy_base.y);
            BWAPI::TilePosition cannonpos = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Protoss_Photon_Cannon, BWAPI::TilePosition(wanted_pos), 32, false);
            bool  succeed = m_scout->build(BWAPI::UnitTypes::Protoss_Photon_Cannon, cannonpos);
            if (succeed) {
                m_data.current_minus_mineral += BWAPI::UnitTypes::Protoss_Photon_Cannon.mineralPrice();
                m_data.front_canon++;
            }
        }
    }
}

bool StarterBot::BuildBuilding(BWAPI::UnitType type)
{
    // Get the type of unit that is required to build the desired building
    BWAPI::UnitType builderType = type.whatBuilds().first;

    // Get a unit that we own that is of the given type so it can build
    // If we can't find a valid builder unit, then we have to cancel the building
    BWAPI::Unit builder = m_data.get_a_miner();
    if (!builder) { return false; }

    // Get a location that we want to build the building next to
    BWAPI::TilePosition desiredPos = BWAPI::Broodwar->self()->getStartLocation();

    // Ask BWAPI for a building location near the desired position for the type
    int maxBuildRange = 64;
    bool buildingOnCreep = type.requiresCreep();
    BWAPI::TilePosition buildPos = BWAPI::Broodwar->getBuildLocation(type, desiredPos, maxBuildRange, buildingOnCreep);
    return builder->build(type, buildPos);
}




