#pragma once
#include <BWAPI.h>
#include "data.h"
#include <array>;

class AttackGroup
{
private:
	BWAPI::Unitset CombatUnit;
	int m_identifier = -1;

public:
	void addUnit(BWAPI::Unit u) { CombatUnit.insert(u); }
	void removeUnit(BWAPI::Unit u) { CombatUnit.erase(u); }
	int size() { return CombatUnit.size(); }
	void attack();
	void holdPosition() { CombatUnit.holdPosition(); }
	void stop() { CombatUnit.stop(); }
	void move(BWAPI::Position target) { CombatUnit.move(target); }
	void setId(int number) { m_identifier = number; }
	int getId() { return this->m_identifier; }
	void update();
	void clear() { CombatUnit.clear(); this->m_identifier = -1;}


};

class CommandCenter
{
private:
	const int maxGroups = 20;
	AttackGroup CombatCenter[20];
public:
	void initial();
	void addCombatUnit(AttackGroup gp);
	void removeCombatUnit(AttackGroup gp);
	void update();
};
