#include "MeleeManager.h"
#include "MapTools.h"
#include "Tools.h"

// 自己的

MeleeManager::MeleeManager()
{
    combatCenter.initial();
}

void MeleeManager::Strategy(BWAPI::Position defend_location, BWAPI::Position enemy_base, int defensiveUnit) {
    // defend at first
    if (BWAPI::Broodwar->self()->supplyUsed() <= defensiveUnit) {
        defendBase(defend_location);
    }
    // if we have a lot of units, attack
    else {
        AttackGroup combatUnits = getCombatGroup(BWEM::Map::Instance().Center(), 32 * BWEM::Map::Instance().Size().x, 200);
        attackBase(combatUnits,enemy_base);
    }
}

void MeleeManager::defendBase(BWAPI::Position defend_location) {
    int defense_radius = 400;
    //BWAPI::Position home = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());
    AttackGroup defenseGroup = getCombatGroup(defend_location, defense_radius, true);

    //BWAPI::Unitset Enemyunits = getUnits(home,960,false);// return ennemies
    //for (BWAPI::Unit u : defenseGroup.getUnits())// all enemies near our units
    //{
    //    BWAPI::Unitset tmp = getUnits(u->getPosition(), defense_radius, false);
    //    for (BWAPI::Unit unit : tmp) { Enemyunits.insert(unit); }
    //}

    //assignTargets(defenseGroup.getUnits(), Enemyunits);
    attackLocation(defenseGroup, defend_location, defense_radius, false);

}

void MeleeManager::attackBase(AttackGroup meleeUnits, BWAPI::Position enemy_base) {
    int attack_radius = 400;

    if (enemy_base.x == 1000 && enemy_base.y == 1002) {//didn't find ennemy base
        return;
    }
    else {
        //const BWAPI::Unitset my_units = meleeUnits.getUnits();
        attackLocation(meleeUnits, enemy_base, attack_radius, false);
    }
}


// attack the enemies in a certain location with radius R. On the mean time decide if we attach workers.
void MeleeManager::attackLocation(AttackGroup my_units, BWAPI::Position center, int radius, bool includeWorkers)
{
    BWAPI::Unitset Enemyunits = getUnits(center, radius, false);// return ennemies
    for (BWAPI::Unit u : my_units.getUnits())// all enemies near our units
    {
        BWAPI::UnitType t = u->getType();
        BWAPI::Unitset tmp = getUnits(u->getPosition(), radius, false);
        for (BWAPI::Unit unit : tmp) { Enemyunits.insert(unit); }
    }
    if (!includeWorkers) {// not attach workers.
        BWAPI::Unitset workersRemoved;
        for (auto& enemyUnit : Enemyunits)
        {
            if (!enemyUnit->getType().isWorker())
            {
                workersRemoved.insert(enemyUnit);
            }
        }
        assignTargets(my_units.getUnits(),workersRemoved);
    }
    else {
        assignTargets(my_units.getUnits(),Enemyunits);
    }

    if (Enemyunits.empty()) { // not ennemies aroud us, move to the location
        for (auto& unit : my_units.getUnits())//move toward the location
        {
            unit->move(center);
        }
    }

}

void MeleeManager::executeMicro(AttackGroup meleeUnits, const BWAPI::Unitset& targets)
{
    assignTargets(meleeUnits.getUnits(),targets);
}

//assign target old

//void MeleeManager::assignTargetsOld(const BWAPI::Unitset& targets)//传入敌人部队
//{
//    const BWAPI::Unitset meleeUnits = getCombatUnits();
//    //getUnits(meleeUnits, BWAPI::Position center, int radius, bool ourUnits, bool oppUnits);
//
//    // figure out targets
//    BWAPI::Unitset meleeUnitTargets;
//    for (auto& target : targets)//敌人部队
//    {
//        // conditions for targeting
//        if (!(target->getType().isFlyer()) &&
//            !(target->isLifted()) && //人族建筑，且飞起来了
//            !(target->getType() == BWAPI::UnitTypes::Zerg_Larva) &&
//            !(target->getType() == BWAPI::UnitTypes::Zerg_Egg) &&
//            target->isVisible())
//        {
//            meleeUnitTargets.insert(target);
//        }
//    }
//
//    // for each meleeUnit
//    for (auto& meleeUnit : meleeUnits)
//    {
//        // if the order is to attack or defend
//        
//            //// run away if we meet the retreat critereon
//            //if (meleeUnitShouldRetreat(meleeUnit, targets))
//            //{
//            //    BWAPI::Position fleeTo(BWAPI::Broodwar->self()->getStartLocation());
//
//            //    Micro::SmartMove(meleeUnit, fleeTo);
//            //}
//            // if there are targets
//            if (!meleeUnitTargets.empty())
//            {
//                // find the best target for this meleeUnit
//                BWAPI::Unit target = getTarget(meleeUnit, meleeUnitTargets);
//
//                // attack it
//                SmartAttackUnit(meleeUnit, target);
//            }
//            //// if there are no targets
//            //else
//            //{
//            //    // if we're not near the order position
//            //    if (meleeUnit->getDistance(m_order.getPosition()) > 100)
//            //    {
//            //        // move to it
//            //        Micro::SmartMove(meleeUnit, m_order.getPosition());
//            //    }
//            //}
//        
//
//        /*if (Config::Debug::DrawUnitTargetInfo)
//        {
//            BWAPI::Broodwar->drawLineMap(meleeUnit->getPosition().x, meleeUnit->getPosition().y,
//                meleeUnit->getTargetPosition().x, meleeUnit->getTargetPosition().y, Config::Debug::ColorLineTarget);
//        }*/
//    }
//}

void MeleeManager::assignTargets(BWAPI::Unitset meleeUnits , const BWAPI::Unitset& targets)//传入敌人部队
{
    // figure out targets
    BWAPI::Unitset meleeUnitTargets;
    for (auto& target : targets)//敌人部队
    {
        // conditions for targeting
        if (!(target->getType().isFlyer()) &&
            !(target->isLifted()) && //人族建筑，且飞起来了
            !(target->getType() == BWAPI::UnitTypes::Zerg_Larva) &&
            !(target->getType() == BWAPI::UnitTypes::Zerg_Egg) &&
            target->isVisible())
        {
            meleeUnitTargets.insert(target);
        }
    }

    // for each meleeUnit
    for (auto& meleeUnit : meleeUnits)
    {
        if (!meleeUnitTargets.empty())
        {
            // find the best target for this meleeUnit
            BWAPI::Unit target = getTarget(meleeUnit, meleeUnitTargets);

            // attack it
            SmartAttackUnit(meleeUnit, target);
        }
    }
}


std::pair<BWAPI::Unit, BWAPI::Unit> MeleeManager::findClosestUnitPair(const BWAPI::Unitset& attackers, const BWAPI::Unitset& targets)
{
    std::pair<BWAPI::Unit, BWAPI::Unit> closestPair(nullptr, nullptr);
    double closestDistance = std::numeric_limits<double>::max();

    for (auto& attacker : attackers)
    {
        BWAPI::Unit target = getTarget(attacker, targets);
        double dist = attacker->getDistance(attacker);

        if (!closestPair.first || (dist < closestDistance))
        {
            closestPair.first = attacker;
            closestPair.second = target;
            closestDistance = dist;
        }
    }

    return closestPair;
}

// get a target for the meleeUnit to attack
BWAPI::Unit MeleeManager::getTarget(BWAPI::Unit meleeUnit, const BWAPI::Unitset& targets)// 自己部队，敌人部队
{
    int highPriority = 0;
    double closestDist = std::numeric_limits<double>::infinity();
    BWAPI::Unit closestTarget = nullptr;

    // for each target possiblity
    for (auto& unit : targets)
    {
        int priority = getAttackPriority(meleeUnit, unit);
        int distance = meleeUnit->getDistance(unit);

        // if it's a higher priority, or it's closer, set it
        if (!closestTarget || (priority > highPriority) || (priority == highPriority && distance < closestDist))
        {
            closestDist = distance;
            highPriority = priority;
            closestTarget = unit;
        }
    }

    return closestTarget;
}

// get the attack priority of a type in relation to a zergling
int MeleeManager::getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit unit)
{
    BWAPI::UnitType type = unit->getType();

    if (attacker->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar
        && unit->getType() == BWAPI::UnitTypes::Terran_Missile_Turret
        && (BWAPI::Broodwar->self()->deadUnitCount(BWAPI::UnitTypes::Protoss_Dark_Templar) == 0))
    {
        return 13;
    }

    if (attacker->getType() == BWAPI::UnitTypes::Protoss_Dark_Templar && unit->getType().isWorker())
    {
        return 12;
    }

    // highest priority is something that can attack us or aid in combat
    if (type == BWAPI::UnitTypes::Terran_Bunker)
    {
        return 11;
    }
    else if (type == BWAPI::UnitTypes::Terran_Medic ||
        (type.groundWeapon() != BWAPI::WeaponTypes::None && !type.isWorker()) ||
        type == BWAPI::UnitTypes::Terran_Bunker ||
        type == BWAPI::UnitTypes::Protoss_High_Templar ||
        type == BWAPI::UnitTypes::Protoss_Reaver ||
        (type.isWorker() ))
    {
        return 10;
    }
    // next priority is worker
    else if (type.isWorker())
    {
        return 9;
    }
    // next is special buildings
    else if (type == BWAPI::UnitTypes::Zerg_Spawning_Pool)
    {
        return 5;
    }
    // next is special buildings
    else if (type == BWAPI::UnitTypes::Protoss_Pylon)
    {
        return 5;
    }
    // next is buildings that cost gas
    else if (type.gasPrice() > 0)
    {
        return 4;
    }
    else if (type.mineralPrice() > 0)
    {
        return 3;
    }
    // then everything else
    else
    {
        return 1;
    }
}

BWAPI::Unit MeleeManager::closestMeleeUnit(BWAPI::Unit target, const BWAPI::Unitset& meleeUnitsToAssign)
{
    double minDistance = 0;
    BWAPI::Unit closest = nullptr;

    for (auto& meleeUnit : meleeUnitsToAssign)
    {
        double distance = meleeUnit->getDistance(target);
        if (!closest || distance < minDistance)
        {
            minDistance = distance;
            closest = meleeUnit;
        }
    }

    return closest;
}

//getCombatUnits

//obtain all the zealots and dragons

//BWAPI::Unitset MeleeManager::getCombatUnits() {
//    // For each unit that we own
//    BWAPI::Unitset return_units = BWAPI::Unitset();
//    for (auto& unit : BWAPI::Broodwar->self()->getUnits())
//    {
//        // if the unit is of the correct type, and it actually has been constructed, return it
//        if ( (unit->getType() == BWAPI::UnitTypes::Protoss_Zealot || unit->getType() == BWAPI::UnitTypes::Protoss_Dragoon)           
//            && unit->isCompleted())
//        {
//            if (unit->getType().isWorker() && unit->isMoving()) { continue; }
//            return_units.insert(unit);            
//        }
//    }
//    // If we didn't find a valid unit to return, make sure we return nullptr
//    return return_units;
//}

AttackGroup MeleeManager::getCombatGroup(Data data, int radius, int number = 6)
{
    AttackGroup gp = getCombatGroup(data.CombatRallyPoint, radius, number);
    return gp;
}

AttackGroup MeleeManager::getCombatGroup(BWAPI::Position center, int radius, int number = 6)
{
    AttackGroup gp = AttackGroup();
    BWAPI::Unitset units = getUnits(center, radius, true);
    for (BWAPI::Unit u : units)
    {   
        if (u->getType() == BWAPI::UnitTypes::Protoss_Zealot || u->getType() == BWAPI::UnitTypes::Protoss_Dragoon)
        {
            gp.addUnit(u);
        }
        if (gp.size() >= number) { break; }
    }
    //combatCenter.addCombatUnit(gp);
    return gp;
}

//getUnits

//update
//返回center周围一定半径内的单位，放在Units中

//void MeleeManager::getUnits(BWAPI::Unitset& units, BWAPI::Position center, int radius, bool ourUnits, bool oppUnits)
//{
//    const int radiusSq(radius * radius);
//
//    if (ourUnits)
//    {
//        for (auto& unit : BWAPI::Broodwar->self()->getUnits())//自己的单位
//        {
//            BWAPI::Position d(unit->getPosition() - center); //初始化position
//            if (d.x * d.x + d.y * d.y <= radiusSq)
//            {
//                if (!units.contains(unit))
//                {
//                    units.insert(unit);
//                }
//            }
//        }
//    }
//
//    if (oppUnits)
//    {
//        for (auto& unit : BWAPI::Broodwar->enemy()->getUnits())
//        {
//            if (unit->getType() == BWAPI::UnitTypes::Unknown || !unit->isVisible())
//            {
//                continue;
//            }
//
//            BWAPI::Position d(unit->getPosition() - center);
//            if (d.x * d.x + d.y * d.y <= radiusSq)
//            {
//                if (!units.contains(unit))
//                {
//                    units.insert(unit);
//                }
//            }
//        }
//    }
//}

BWAPI::Unitset MeleeManager::getUnits(BWAPI::Position center, int radius, bool ourUnits)
{
    BWAPI::Unitset units = BWAPI::Broodwar->getUnitsInRadius(center.x, center.y, radius);
    BWAPI::Unitset output;
    if (ourUnits)
    {
        for (BWAPI::Unit u : units)
        {
            if (u->getPlayer() == BWAPI::Broodwar->self())
            {
                output.insert(u);
            }
        }
    }
    else 
    {
        for (BWAPI::Unit u : units)
        {
            if (u->getPlayer() == BWAPI::Broodwar->enemy())
            {
                output.insert(u);
            }
        }
    }
    return output;
}


void MeleeManager::SmartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target)
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
    if (currentCommand.getType() == BWAPI::UnitCommandTypes::Attack_Unit && currentCommand.getTarget() == target)
    {
        return;
    }

    // if nothing prevents it, attack the target
    attacker->attack(target);
    

    //if (Config::Debug::DrawUnitTargetInfo)
    //{
    //    BWAPI::Broodwar->drawCircleMap(attacker->getPosition(), dotRadius, BWAPI::Colors::Red, true);
    //    BWAPI::Broodwar->drawCircleMap(target->getPosition(), dotRadius, BWAPI::Colors::Red, true);
    //    BWAPI::Broodwar->drawLineMap(attacker->getPosition(), target->getPosition(), BWAPI::Colors::Red);
    //}
}


//end update
void MeleeManager::setRallyPoint(BWAPI::Position pos)
{
    pos = pos.makeValid();
    for (auto& u : BWAPI::Broodwar->self()->getUnits())
    {
        if (u->getType() == BWAPI::UnitTypes::Protoss_Gateway)
        {
            u->setRallyPoint(pos);
        }    
    }
}


void CombatCenter::initial()
{
    for (int i = 0; i < maxGroups; i++)
    {
        combatCenter[i] = AttackGroup();
    }
}

void CombatCenter::addCombatUnit(AttackGroup gp)
{
    for (int i = 0; i < maxGroups; i++)
    {
        if (combatCenter[i].getId() == -1) {
            combatCenter[i] = gp;
            gp.setId(i);
            return;
        }
    }
}

void CombatCenter::removeCombatUnit(AttackGroup gp)
{
    int num = gp.getId();
    if (num == -1) { return; }
    else {
        gp.clear();
        combatCenter[num] = gp;
    }
}

void CombatCenter::update()
{
    for (AttackGroup& gp : combatCenter)
    {
        gp.update();
    }
}

void AttackGroup::attack()
{
    for (BWAPI::Unit u : CombatUnit)
    {
        // to complete
    }
}

void AttackGroup::update()
{
    for (BWAPI::Unit u : CombatUnit)
    {
        if (!u->exists())
        {
            removeUnit(u);
        }
    }
}

int AttackGroup::size()
{
    if (CombatUnit.empty()) { return 0; }
    else { return CombatUnit.size(); }
}

void AttackGroup::flee(Data data)
{
    for (BWAPI::Unit u : CombatUnit)
    {
        BWAPI::Position unitPos = u->getPosition();
        BWAPI::Position targetPos;
        BWAPI::Unitset unitset = getUnits(u->getPosition(), 7, false);
        for (BWAPI::Unit unit : unitset){
            if (unit->getType() == BWAPI::UnitTypes::Zerg_Sunken_Colony ||
                unit->getType() == BWAPI::UnitTypes::Protoss_Photon_Cannon)
            {
                BWAPI::Position GuardPos= unit->getPosition();
                float dist = u->getDistance(unit);
            }
        }

    }
    
}

BWAPI::Unitset AttackGroup::getUnits(BWAPI::Position center, int radius, bool ourUnits)
{
    BWAPI::Unitset units = BWAPI::Broodwar->getUnitsInRadius(center.x, center.y, radius);
    BWAPI::Unitset output;
    if (ourUnits)
    {
        for (BWAPI::Unit u : units)
        {
            if (u->getPlayer() == BWAPI::Broodwar->self())
            {
                output.insert(u);
            }
        }
    }
    else
    {
        for (BWAPI::Unit u : units)
        {
            if (u->getPlayer() == BWAPI::Broodwar->enemy())
            {
                output.insert(u);
            }
        }
    }
    return output;
}