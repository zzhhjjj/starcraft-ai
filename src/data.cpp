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

//Clear the previous job of a worker and set it to idle
void Data::clearPreviousJob(BWAPI::Unit unit)
{
    if (!unit) { return; }

    WorkerJob previousJob = getWorkerJob(unit);

    if (previousJob == Build)
    {
        m_workerBuildingTypeMap.erase(unit);
    }

    m_workerJobMap.erase(unit);
}

//get the job of a worker
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

//get a woker who is mining who is closest to position pos
BWAPI::Unit Data::get_a_miner(BWAPI::Position pos) {

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

//get a random miner
BWAPI::Unit Data::get_a_miner() {

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


//get the current mineral that can be used: the current mineral offered by the BWAPI - mineral distributed to build buildings(distributed but not used because the worker is on the way)
int Data::current_mineral() {

    return  BWAPI::Broodwar->self()->minerals() - current_minus_mineral;
}


//get the closest enemy building to attack
BWAPI::Unit Data::enemy_building() {
    BWAPI::Unit closestUnit = nullptr;

    for (std::map<BWAPI::Unit, BWAPI::Position>::iterator it = enemy_buildings.begin(); it != enemy_buildings.end(); ++it)
    {
        if (!closestUnit || it->first->getDistance(BWAPI::Position(front_pylon_pos)) < closestUnit->getDistance(BWAPI::Position(front_pylon_pos)))
        {
            closestUnit = it->first;
        }
    }
    return closestUnit;

}

//get the position of the closest enemy building
BWAPI::Position Data::enemy_base() {
    BWAPI::Unit closestUnit = nullptr;

    for (std::map<BWAPI::Unit, BWAPI::Position>::iterator it = enemy_buildings.begin(); it != enemy_buildings.end(); ++it)
    {
        if (!closestUnit || it->first->getDistance(BWAPI::Position(front_pylon_pos)) < closestUnit->getDistance(BWAPI::Position(front_pylon_pos)))
        {
            closestUnit = it->first;
        }
    }

    return enemy_buildings[closestUnit];

}
