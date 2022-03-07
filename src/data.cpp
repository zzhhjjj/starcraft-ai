#include "data.h"
#include "Tools.h"

Data::Data() {

    
    m_workers = Tools::GetAllUnitOfType(worker_type);
    m_depots = Tools::GetAllUnitOfType(depot_type);
    m_minerals = BWAPI::Broodwar->getMinerals();

    for (auto worker : m_workers) {
        m_workerJobMap[worker] = Idle;
       /* m_workerDepotMap[worker] = Tools::GetClosestUnitTo(worker, m_depots);*/
    }

 
}

void Data::update() {
    m_workers = Tools::GetAllUnitOfType(worker_type);
    m_depots = Tools::GetAllUnitOfType(depot_type);
    m_minerals = BWAPI::Broodwar->getMinerals();

    for (auto worker : m_workers) {
        m_workerJobMap[worker] = Idle;
        /*m_workerDepotMap[worker] = Tools::GetClosestUnitTo(worker, m_depots);*/
    }

}

void Data::workerDestroyed(BWAPI::Unit unit)
{
    if (!unit) { return; }

    clearPreviousJob(unit);
    m_workers.erase(unit);
}

void Data::clearPreviousJob(BWAPI::Unit unit)
{
    if (!unit) { return; }

    WorkerJob previousJob = getWorkerJob(unit);

    if (previousJob == Minerals)
    {
        /*m_depotWorkerCount[m_workerDepotMap[unit]] -= 1;*/

        /*m_workerDepotMap.erase(unit);*/

        // remove a worker from this unit's assigned mineral patch
        //addToMineralPatch(m_workerMineralAssignment[unit], -1);

        // erase the association from the map
        //m_workerMineralAssignment.erase(unit);
    }
    else if (previousJob == Gas)
    {
        //m_refineryWorkerCount[m_workerRefineryMap[unit]] -= 1;
        /*m_workerRefineryMap.erase(unit);*/
    }
    else if (previousJob == Build)
    {
        m_workerBuildingTypeMap.erase(unit);
    }
    //else if (previousJob == Repair)
    //{
    //    m_workerRepairMap.erase(unit);
    //}
    //else if (previousJob == Move)
    //{
    //    m_workerMoveMap.erase(unit);
    //}

    m_workerJobMap.erase(unit);
}


enum Data::WorkerJob Data::getWorkerJob(BWAPI::Unit unit)
{
    if (!unit) { return Default; }

    std::map<BWAPI::Unit, enum WorkerJob>::iterator it = m_workerJobMap.find(unit);

    if (it != m_workerJobMap.end())
    {
        return it->second;
    }

    return Default;
}

BWAPI::Unit Data::get_a_miner(BWAPI::Position pos) {

    /*BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(pos, BWAPI::Broodwar->getMinerals());*/
    /*if (closestMineral == nullptr) {
        BWAPI::Broodwar->printf("Can't find mineral!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ");

    }*/
    int k = 0;
    for (auto x : m_workerJobMap) {
        k = k + 1;
        if (x.second == Minerals){
            return x.first;
        }
        BWAPI::Broodwar->printf("worker%d doesn't work on this mineral",k);
    }
    return nullptr;
}

BWAPI::Unit Data::get_a_miner() {

    /*BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(pos, BWAPI::Broodwar->getMinerals());*/
    /*if (closestMineral == nullptr) {
        BWAPI::Broodwar->printf("Can't find mineral!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! ");

    }*/
    int k = 0;
    for (auto x : m_workerJobMap) {
        k = k + 1;
        if (x.second == Minerals) {
            return x.first;
        }
        BWAPI::Broodwar->printf("worker%d doesn't work on this mineral", k);
    }
    return nullptr;
}

int Data::current_mineral() {

    return  BWAPI::Broodwar->self()->minerals() - current_minus_mineral;
}
