#include <BWAPI.h>
#include "Tools.h"
class Data {

public:

	enum WorkerJob { Minerals, Gas, Build, Combat, Idle, Repair, Move, Scout, Default };
    const BWAPI::UnitType depot_type = BWAPI::Broodwar->self()->getRace().getResourceDepot();
    const BWAPI::UnitType worker_type = BWAPI::UnitTypes::Protoss_Probe;


	BWAPI::Unitset m_workers;
	BWAPI::Unitset m_depots;
    BWAPI::Unitset m_minerals;

    std::map<BWAPI::Unit, enum WorkerJob>   m_workerJobMap;
    std::map<BWAPI::Unit, BWAPI::Unit>      m_workerMineralMap;
    std::map<BWAPI::Unit, BWAPI::Unit>      m_workerDepotMap;
    std::map<BWAPI::Unit, BWAPI::Unit>      m_workerRefineryMap;
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

    void workerDestroyed(BWAPI::Unit unit);

    enum WorkerJob  getWorkerJob(BWAPI::Unit unit);

    BWAPI::Unit get_a_miner(BWAPI::Position pos);



};