#pragma once
#include <BWAPI.h>
#include "data.h"
#include <array>
#include "map.h"

//×Ô¼ºµÄ
class AttackGroup
{
private:
	BWAPI::Unitset CombatUnit;
	int m_identifier = -1;

public:
	AttackGroup() {};
	void addUnit(BWAPI::Unit u) { CombatUnit.insert(u); }
	void removeUnit(BWAPI::Unit u) { CombatUnit.erase(u); }
	BWAPI::Unitset getUnits() { return this->CombatUnit; }
	BWAPI::Unitset getUnits(BWAPI::Position center, int radius, bool ourUnits);
	int size(); 
	void attack();
	void holdPosition() { CombatUnit.holdPosition(); }
	void stop() { CombatUnit.stop(); }
	void move(BWAPI::Position target) { CombatUnit.move(target); }
	void setId(int number) { m_identifier = number; }
	int getId() { return this->m_identifier; }
	void update();
	void clear() { CombatUnit.clear(); this->m_identifier = -1; }
	void flee(Data data);
};

class CombatCenter
{
private:
	const int maxGroups = 20;
	AttackGroup combatCenter[20];
public:
	CombatCenter() {};
	void initial();
	void addCombatUnit(AttackGroup gp);
	void removeCombatUnit(AttackGroup gp);
	void update();
};

class MeleeManager
{
	CombatCenter combatCenter;

    public:
        MeleeManager();
        void executeMicro(AttackGroup meleeUnits, const BWAPI::Unitset& targets);
		//void executeMicro(const BWAPI::Unitset& targets);
        
		void assignTargets(BWAPI::Unitset meleeUnits, const BWAPI::Unitset& targets); //base code for attacking
		//void assignTargetsOld(const BWAPI::Unitset& targets);

        int getAttackPriority(BWAPI::Unit attacker, BWAPI::Unit unit);
        BWAPI::Unit closestMeleeUnit(BWAPI::Unit target, const BWAPI::Unitset& meleeUnitToAssign);
        BWAPI::Unit getTarget(BWAPI::Unit meleeUnit, const BWAPI::Unitset& targets);
        std::pair<BWAPI::Unit, BWAPI::Unit> findClosestUnitPair(const BWAPI::Unitset& attackers, const BWAPI::Unitset& targets);
        //BWAPI::Unitset getCombatUnits();
		AttackGroup getCombatGroup(Data data, int radius, int number);//to be modify by adding the Type character, given the function in Tools::GetAllUnitOfType()
		AttackGroup getCombatGroup(BWAPI::Position center, int radius, int number);
        
        //void getUnits(BWAPI::Unitset& units, BWAPI::Position center, int radius, bool ourUnits, bool oppUnits);
		BWAPI::Unitset getUnits(BWAPI::Position center, int radius, bool ourUnits);
        void SmartAttackUnit(BWAPI::Unit attacker, BWAPI::Unit target);
        void setRallyPoint(BWAPI::Position);

		void attackLocation(AttackGroup meleeUnits, BWAPI::Position center, int radius, bool includeWorkers);
		void attackBase(AttackGroup meleeUnits, BWAPI::Position enemy_base);

		void defendBase(BWAPI::Position defend_location);
		void Strategy(BWAPI::Position defend_location, BWAPI::Position enemy_base, int defensiveUnit);
};


