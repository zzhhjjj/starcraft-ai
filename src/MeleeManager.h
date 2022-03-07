#pragma once
#include <BWAPI.h>



class MeleeManager
{

    public:

        MeleeManager();
        void executeMicro(const BWAPI::Unitset& targets);
        
        void assignTargetsOld(const BWAPI::Unitset& targets);

        int getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit unit);
        BWAPI::Unit closestMeleeUnit(BWAPI::Unit target, const BWAPI::Unitset& meleeUnitToAssign);
        BWAPI::Unit getTarget(BWAPI::Unit meleeUnit, const BWAPI::Unitset& targets);
        std::pair<BWAPI::Unit, BWAPI::Unit> findClosestUnitPair(const BWAPI::Unitset& attackers, const BWAPI::Unitset& targets);
        //update
        BWAPI::Unitset getCombatUnits();
        void getUnits(BWAPI::Unitset& units, BWAPI::Position center, int radius, bool ourUnits, bool oppUnits);
        void SmartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target);
        void attackLocation(BWAPI::Unitset my_units, BWAPI::Position center, int radius, bool includeWorkers);
        void MeleeManager::attackBase(BWAPI::Position enemy_base, int units_supply);

        void MeleeManager::defendBase();

};
