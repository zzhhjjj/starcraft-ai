#include <BWAPI.h>
#include "Tools.h"
class Data {

public:

	enum WorkerJob { Minerals, Gas, Build, Combat, Idle, Repair, Move, Scout, Default };
    const BWAPI::UnitType depot_type = BWAPI::UnitTypes::Protoss_Nexus;
    const BWAPI::UnitType worker_type = BWAPI::UnitTypes::Protoss_Probe;

    BWAPI::Race enemy_race ;
    BWAPI::Unit enemy_building ;
    BWAPI::Position front_pylon_pos;
    bool detecte_enemy = false;
    

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