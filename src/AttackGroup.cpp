#include "AttackGroup.h"

void CommandCenter::initial()
{
	for (int i = 0; i < maxGroups; i++) 
	{
		CombatCenter[i] = AttackGroup();
	}
}

void CommandCenter::addCombatUnit(AttackGroup gp)
{
	for (int i = 0; i < maxGroups; i++) 
	{
		if (CombatCenter[i].getId() == -1){
			CombatCenter[i] = gp;
			return;
		}
	}
}

void CommandCenter::removeCombatUnit(AttackGroup gp)
{
	int num = gp.getId();
	if (num == -1) { return; }
	else {
		gp.clear();
		CombatCenter[num] = gp;
	}
}

void CommandCenter::update()
{
	for (AttackGroup& gp : CombatCenter)
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