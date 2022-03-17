#pragma once
#include <BWAPI.h>
#include "Tools.h"
class Data {

public:

	enum WorkerJob { Minerals, Gas, Build, Combat, Idle, Repair, Move, Scout, Default };
    const BWAPI::UnitType depot_type = BWAPI::UnitTypes::Protoss_Nexus;
    const BWAPI::UnitType worker_type = BWAPI::UnitTypes::Protoss_Probe;

    BWAPI::TilePosition home = BWAPI::Broodwar->self()->getStartLocation();
    BWAPI::Race enemy_race ;
    //BWAPI::Unit enemy_building ;
    
    BWAPI::TilePosition front_pylon_pos_pre;
    BWAPI::TilePosition front_pylon_pos;
    BWAPI::Position CombatRallyPoint;
    bool detecte_enemy = false;
    int groupSize = 3;

    bool front_pylon = false;
    bool second_pylon = false;
    int front_gateway = 0;
    bool base_forge = false;
    int front_canon = 0;
    
    //BWAPI::Position enemy_base = BWAPI::Position(1000,1002);
    BWAPI::Position enemy_base();
    BWAPI::Unit enemy_building();
    std::map<BWAPI::Unit, BWAPI::Position> enemy_buildings;

	BWAPI::Unitset m_workers;
	BWAPI::Unitset m_depots;
    BWAPI::Unitset m_minerals;
    int number_of_gaser;

    int current_minus_mineral=400;

    std::map<BWAPI::Unit, enum WorkerJob>   m_workerJobMap;
    /*std::map<BWAPI::Unit, BWAPI::Unit>      m_workerMineralMap;
    std::map<BWAPI::Unit, BWAPI::Unit>      m_workerDepotMap;
    std::map<BWAPI::Unit, BWAPI::Unit>      m_workerRefineryMap;*/
    //std::map<BWAPI::Unit, BWAPI::Unit>      m_workerRepairMap;
    //std::map<BWAPI::Unit, WorkerMoveData>   m_workerMoveMap;
    std::map<BWAPI::Unit, BWAPI::UnitType>  m_workerBuildingTypeMap;

    //std::map<BWAPI::Unit, int>              m_depotWorkerCount;
    //std::map<BWAPI::Unit, int>              m_refineryWorkerCount;
    //std::map<BWAPI::Unit, int>              m_workersOnMineralPatch;
    //std::map<BWAPI::Unit, BWAPI::Unit>      m_workerMineralAssignment;
    void clearPreviousJob(BWAPI::Unit unit);

public:

    Data();

    void update();

    void workerDestroyed(BWAPI::Unit unit);

    enum WorkerJob  getWorkerJob(BWAPI::Unit unit);

    BWAPI::Unit get_a_miner(BWAPI::Position pos);

    BWAPI::Unit get_a_miner();

    int current_mineral();

};