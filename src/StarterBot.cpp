#include "StarterBot.h"
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
    front_strategy();

 
    
    //end update
}


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
    //if our scout is destroyed, reset one miner to scout mission
    if (unit == m_scout) {
        m_scout = m_data.get_a_miner();
        m_data.m_workerJobMap[m_scout] = m_data.Scout;
    }
    //if a building is destroyed, we consider it as a front building and rebuild one
    if (unit->getType() == BWAPI::UnitTypes::Protoss_Pylon) {
        m_data.front_pylon = true;
        m_data.front_pylon_pos = m_data.front_pylon_pos_pre;
        m_data.second_pylon = false;
    }
    if (unit->getType() == BWAPI::UnitTypes::Protoss_Gateway) {
        m_data.front_gateway--;
    }
    if (unit->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon) {
        m_data.front_canon--;
    }
    //erase a worker from our data if it's destroyed
    if (unit->getType() == m_data.worker_type) {
        m_data.m_workerJobMap.erase(unit);
    }	
    //if an enemy building is destroyed, erase it
    if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()) == true) {

        if (unit->getType().isBuilding()) {
            m_data.enemy_buildings.erase(unit);


        }
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
        
    }
}

// Called whenever a unit finished construction, with a pointer to the unit
void StarterBot::onUnitComplete(BWAPI::Unit unit)
{   
    //Once the gas station is built, send three workers to it
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
    
    //front policy: 
    if (unit->getType() == BWAPI::UnitTypes::Protoss_Pylon && m_data.front_pylon) {
        BWAPI::Broodwar->printf("New pylon position: %d, %d", unit->getTilePosition().x, unit->getTilePosition().y);
        if (unit->getDistance(BWAPI::Position(m_data.front_pylon_pos)) < 32 * 15) {
            m_data.second_pylon = false;
            m_data.front_gateway = 0;
            m_data.front_canon = 0;
            m_data.front_pylon_pos_pre = m_data.front_pylon_pos;
            m_data.front_pylon_pos = unit->getTilePosition();
        }
    }//update

    if (unit->getType() == BWAPI::UnitTypes::Protoss_Gateway) {
        m_data.front_gateway++;
    }
    if (unit->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon) {
        m_data.front_canon++;
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
    // if we can detect the enemy building, add it to list of enemy buildings
    if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()) == true) {
        if (unit->getType().isBuilding()) {
            m_data.enemy_race = BWAPI::Broodwar->enemy()->getRace();
            
            m_data.enemy_buildings[unit] = unit->getPosition();

            m_data.detecte_enemy = true;
            

            BWAPI::Broodwar->printf("find enenmy base!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ");
            BWAPI::Broodwar->printf("%s ", m_scout->getLastCommand().getType().c_str());

            
            if (!m_data.front_pylon) { m_scout->move(BWAPI::Position(m_data.home)); }

            if (unit->getType() == BWAPI::UnitTypes::Zerg_Sunken_Colony || unit->getType() == BWAPI::UnitTypes::Terran_Bunker) { m_data.groupSize = 9; }
            //m_scout->holdPosition();
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

//buidling, training strategies
void StarterBot::initialStrategy() {
    building_order.push(make_pair(60, BWAPI::UnitTypes::Protoss_Cybernetics_Core));
    building_order.push(make_pair(60, BWAPI::UnitTypes::Protoss_Assimilator));
    building_order.push(make_pair(18, BWAPI::UnitTypes::Protoss_Forge));
    building_order.push(make_pair(20, BWAPI::UnitTypes::Protoss_Photon_Cannon));
    building_order.push(make_pair(20, BWAPI::UnitTypes::Protoss_Photon_Cannon));
    building_order.push(make_pair(20, BWAPI::UnitTypes::Protoss_Photon_Cannon));
    
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

//execute building policy, checked every frame
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


//execute training policy, checked every frame
void StarterBot::train() {
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
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


//execute upgrade policy, checked every frame
void StarterBot::upgrade() {
    const int supply = BWAPI::Broodwar->self()->supplyUsed();
    if (!upgrade_order.empty()) {
        const q b1 = upgrade_order.top();
        if (supply >= b1.first && m_data.current_mineral() > b1.second.mineralPrice()) {//�˿ڹ���
            const bool startedupgrade = Tools::upgrade(b1.second);// ���ؽ����Ƿ�ʼ����
            if (startedupgrade)
            {
                //BWAPI::Broodwar->printf("Started upgrade %s", b1.second.getName().c_str());
                upgrade_order.pop(); 
            }

        }
    }
}

//check for front 
void StarterBot::check() {
    if (m_data.detecte_enemy && !m_data.front_pylon)
    {
        float dist = m_scout->getDistance(m_data.enemy_base());
        if (dist >= 22 * 32){
        m_scout->holdPosition();
        m_data.front_pylon_pos = m_scout->getTilePosition();//update
        }
    }
    if (BWAPI::Broodwar->self()->supplyUsed() > 10 && !m_data.detecte_enemy) { StarterBot::sendScout(); }
    

    BWAPI::Unitset targets = BWAPI::Broodwar->enemy()->getUnits();

    //m_meleeManager.Strategy(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()), m_data.enemy_base(), 80);
    
    m_meleeManager.defendBase(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));

    if (m_data.front_pylon) { m_meleeManager.setRallyPoint(BWAPI::Position(m_data.front_pylon_pos)); }
    //else { m_meleeManager.setRallyPoint(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation())); }

    AttackGroup attackGp = m_meleeManager.getCombatGroup(m_data.CombatRallyPoint, 128 * 32, m_data.groupSize);
    //BWAPI::BroodwarPtr->printf("Attack group formed: %d", attackGp.size());
    m_meleeManager.combatCenter.update();

    if (attackGp.size() == m_data.groupSize) { 
        m_meleeManager.combatCenter.addCombatUnit(attackGp);
        m_meleeManager.attackBase(attackGp, m_data.enemy_base()); }
    for (AttackGroup gp : m_meleeManager.combatCenter.getGroups()) {
        if(gp.size() > 0){ m_meleeManager.attackBase(gp, m_data.enemy_base()); }      
    }

    
}

//set unit to be scout unit
void StarterBot::setScout(BWAPI::Unit unit) 
{
    if (m_scout)
    {
        return;
    }
    m_scout = unit;
    m_data.m_workerJobMap[unit] = m_data.Scout;
}


//send a worker as scouter to look up all the locations offered by BWAPI
void StarterBot::sendScout()
{
    if (!m_scout||!m_scout->exists())
    {
        StarterBot::setScout(m_data.get_a_miner());

    }

    for (auto location : BWAPI::Broodwar->getStartLocations())
    {
        if (m_data.detecte_enemy == true) {
            BWAPI::Broodwar->printf("Enemy building is %s, position: %d, %d", m_data.enemy_building()->getType().getName().c_str(), m_data.enemy_building()->getPosition().x, m_data.enemy_building()->getPosition().y);
            return;
        }
        if (!BWAPI::Broodwar->isExplored(location))
        {
            if (m_data.detecte_enemy == true) {
                BWAPI::Broodwar->printf("Enemy building is %s, position: %d, %d", m_data.enemy_building()->getType().getName().c_str(), m_data.enemy_building()->getPosition().x, m_data.enemy_building()->getPosition().y);
                return;
            }
            m_scout->move(BWAPI::Position(location));
            return;
        }
    }
}

/*Our front strategy: 
1.once detect enemy, calculate the front pylon position
2.build first pylon, gateway and canons
3.iterate to build more and more pylon, canons to approach the enemy buildings
*/
void StarterBot::front_strategy() {
    if (m_data.detecte_enemy) {
        BWAPI::Broodwar->printf("FrontPylonTilePosition : %d,%d", m_data.front_pylon_pos.x, m_data.front_pylon_pos.y);
        BWAPI::Broodwar->printf("HomePosition : %d,%d", m_data.home.x, m_data.home.y);
        BWAPI::Broodwar->printf("EnemyPosition : %d,%d", BWAPI::TilePosition(m_data.enemy_base()).x, BWAPI::TilePosition(m_data.enemy_base()).y);

        if (!m_data.front_pylon && m_data.current_mineral() > BWAPI::UnitTypes::Protoss_Pylon.mineralPrice() &&m_scout->isHoldingPosition()) {
            //m_scout->holdPosition();//update
            //m_data.front_pylon_pos = m_scout->getTilePosition();//update
            
            BWAPI::Broodwar->printf("want to build front pylon  %d", m_data.current_mineral());
            BWAPI::TilePosition pylonpos = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::TilePosition(m_data.front_pylon_pos), 64, false).makeValid();
            bool  succeed = m_scout->build(BWAPI::UnitTypes::Protoss_Pylon, pylonpos);
            if (succeed) {
                m_data.front_pylon_pos = pylonpos;
                m_data.current_minus_mineral += BWAPI::UnitTypes::Protoss_Pylon.mineralPrice();
                m_data.front_pylon = true;
                
                

            }
        }

        if (m_data.front_canon < 3 && m_data.front_pylon && m_data.current_mineral() > BWAPI::UnitTypes::Protoss_Photon_Cannon.mineralPrice()) {

            BWAPI::Position wanted_pos = BWAPI::Position(0.9 * BWAPI::Position(m_data.front_pylon_pos).x + 0.1 * m_data.enemy_base().x, 0.9 * BWAPI::Position(m_data.front_pylon_pos).y + 0.1 * m_data.enemy_base().y);
            BWAPI::TilePosition cannonpos = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Protoss_Photon_Cannon, BWAPI::TilePosition(wanted_pos), 64, false);
            BWAPI::Broodwar->printf("want canon  (%d,%d) (%d,%d) (%d,%d)", m_data.front_pylon_pos.x, m_data.front_pylon_pos.y, BWAPI::TilePosition(m_data.front_pylon_pos).x, BWAPI::TilePosition(m_data.front_pylon_pos).y, cannonpos.x, cannonpos.y);
            bool  succeed = m_scout->build(BWAPI::UnitTypes::Protoss_Photon_Cannon, cannonpos);
            if (succeed) {
                m_data.current_minus_mineral += BWAPI::UnitTypes::Protoss_Photon_Cannon.mineralPrice();
                //m_data.front_canon++;
            }
        }
        if (m_data.front_gateway < 2 && m_data.front_canon>1 && m_data.current_mineral() > BWAPI::UnitTypes::Protoss_Gateway.mineralPrice()) {
            BWAPI::Broodwar->printf("want to build front Gateway  %d", m_data.current_mineral());

            BWAPI::TilePosition gatewaypos = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Protoss_Gateway, BWAPI::TilePosition(m_data.front_pylon_pos), 64, false).makeValid();
            bool  succeed = m_scout->build(BWAPI::UnitTypes::Protoss_Gateway, gatewaypos);
            if (succeed) {
                m_data.current_minus_mineral += BWAPI::UnitTypes::Protoss_Gateway.mineralPrice();
                //m_data.front_gateway++;
            }
        }
        if (m_data.front_gateway > 0 && m_data.second_pylon == false && m_data.current_mineral() > BWAPI::UnitTypes::Protoss_Pylon.mineralPrice()) {
            BWAPI::Broodwar->printf("want to build front pylon  %d", m_data.current_mineral());
            BWAPI::Position wanted_pos = BWAPI::Position(1.0 * BWAPI::Position(m_data.front_pylon_pos).x + 0.0 * m_data.enemy_base().x, 1.0 * BWAPI::Position(m_data.front_pylon_pos).y + 0.0 * m_data.enemy_base().y).makeValid();
            BWAPI::TilePosition pylonpos = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Protoss_Pylon, BWAPI::TilePosition(wanted_pos), 64, false);
            bool  succeed = m_scout->build(BWAPI::UnitTypes::Protoss_Pylon, pylonpos);
            if (succeed) {

                m_data.current_minus_mineral += BWAPI::UnitTypes::Protoss_Pylon.mineralPrice();
                m_data.second_pylon = true;
            }
        }
        if (m_data.second_pylon && m_data.front_canon<4 && m_data.current_mineral() > BWAPI::UnitTypes::Protoss_Photon_Cannon.mineralPrice()) {
            BWAPI::Broodwar->printf("want to build front canon  %d", m_data.current_mineral());
            BWAPI::Position wanted_pos = BWAPI::Position(0.95 * BWAPI::Position(m_data.front_pylon_pos).x + 0.05 * m_data.enemy_base().x, 0.95 * BWAPI::Position(m_data.front_pylon_pos).y + 0.05 * m_data.enemy_base().y).makeValid();
            BWAPI::TilePosition cannonpos = BWAPI::Broodwar->getBuildLocation(BWAPI::UnitTypes::Protoss_Photon_Cannon, BWAPI::TilePosition(wanted_pos), 64, false);
            bool  succeed = m_scout->build(BWAPI::UnitTypes::Protoss_Photon_Cannon, cannonpos);
            if (succeed) {
                m_data.current_minus_mineral += BWAPI::UnitTypes::Protoss_Photon_Cannon.mineralPrice();
                //m_data.front_canon++;
            }
        }
    }
}


//build a building of type type
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




