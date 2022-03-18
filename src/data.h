#pragma once
#include <BWAPI.h>
#include "Tools.h"

//the class to store information about workers, buildings, positions and resources
class Data {

public:

    //Type of jobs, workers and base
	enum WorkerJob { Minerals, Gas, Build, Combat, Idle, Repair, Move, Scout, Default };
    const BWAPI::UnitType depot_type = BWAPI::UnitTypes::Protoss_Nexus;
    const BWAPI::UnitType worker_type = BWAPI::UnitTypes::Protoss_Probe;
    BWAPI::Race enemy_race ;


    //Positions
    BWAPI::TilePosition home = BWAPI::Broodwar->self()->getStartLocation();
    BWAPI::TilePosition front_pylon_pos_pre;
    BWAPI::TilePosition front_pylon_pos;
    BWAPI::Position CombatRallyPoint;
    
    std::map<BWAPI::Unit, BWAPI::Position> enemy_buildings;

    //Counters for front strategies
    bool detecte_enemy = false;
    int groupSize = 3;
    bool front_pylon = false;
    bool second_pylon = false;
    int front_gateway = 0;
    bool base_forge = false;
    int front_canon = 0;
    

    //get units
	BWAPI::Unitset m_workers;
	BWAPI::Unitset m_depots;
    BWAPI::Unitset m_minerals;
    int number_of_gaser;
    


    //used for compute resources
    int current_minus_mineral=400;

    //worker job management
    std::map<BWAPI::Unit, enum WorkerJob>   m_workerJobMap;
    std::map<BWAPI::Unit, BWAPI::UnitType>  m_workerBuildingTypeMap;
    

public:

    Data();

    //functions to compute data
    BWAPI::Unit enemy_building();

    BWAPI::Position enemy_base();

    enum WorkerJob  getWorkerJob(BWAPI::Unit unit);

    BWAPI::Unit get_a_miner(BWAPI::Position pos);

    BWAPI::Unit get_a_miner();

    int current_mineral();

    void clearPreviousJob(BWAPI::Unit unit);

};