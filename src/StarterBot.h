#pragma once

#include "MapTools.h"
#include "data.h"
#include <BWAPI.h>
#include "MeleeManager.h"


class StarterBot
{
    MapTools m_mapTools;
	Data m_data;
	MeleeManager m_meleeManager;

	BWAPI::Unit m_scout = nullptr;

public:

    StarterBot();

    // helper functions to get you started with bot programming and learn the API
    void sendIdleWorkersToMinerals();
    void trainAdditionalWorkers();
    void buildAdditionalSupply();
    void drawDebugInformation();

    // functions that are triggered by various BWAPI events from main.cpp
	void onStart();
	void onFrame();
	void onEnd(bool isWinner);
	void onUnitDestroy(BWAPI::Unit unit);
	void onUnitMorph(BWAPI::Unit unit);
	void onSendText(std::string text);
	void onUnitCreate(BWAPI::Unit unit);
	void onUnitComplete(BWAPI::Unit unit);
	void onUnitShow(BWAPI::Unit unit);
	void onUnitHide(BWAPI::Unit unit);
	void onUnitRenegade(BWAPI::Unit unit);

	//update
	void initialStrategy();
	void build();
	void train();
	void check();
	void upgrade();
	void setScout(BWAPI::Unit unit);
	void sendScout();
	void front_strategy(); // to be modified by using MapTools::isBuildable();
	bool BuildBuilding(BWAPI::UnitType type);
};