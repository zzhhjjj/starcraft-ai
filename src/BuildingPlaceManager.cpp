#include "BuildingPlaceManager.h"

BuildingPlaceManager::BuildingPlaceManager()
{
    m_reserveMap = Grid<int>(BWAPI::Broodwar->mapWidth(), BWAPI::Broodwar->mapHeight(), 0);

    computeResourceBox();
}

void BuildingPlaceManager::computeResourceBox()
{
    //PROFILE_FUNCTION();

    BWAPI::Position start(BWAPI::Broodwar->self()->getStartLocation());
    BWAPI::Unitset unitsAroundNexus;

    for (auto& unit : BWAPI::Broodwar->getAllUnits())
    {
        // if the units are less than 400 away add them if they are resources
        if (unit->getDistance(start) < 300 && unit->getType().isMineralField())
        {
            unitsAroundNexus.insert(unit);
        }
    }

    for (auto& unit : unitsAroundNexus)
    {
        int x = unit->getPosition().x;
        int y = unit->getPosition().y;

        int left = x - unit->getType().dimensionLeft();
        int right = x + unit->getType().dimensionRight() + 1;
        int top = y - unit->getType().dimensionUp();
        int bottom = y + unit->getType().dimensionDown() + 1;

        m_boxTop = top < m_boxTop ? top : m_boxTop;
        m_boxBottom = bottom > m_boxBottom ? bottom : m_boxBottom;
        m_boxLeft = left < m_boxLeft ? left : m_boxLeft;
        m_boxRight = right > m_boxRight ? right : m_boxRight;
    }

    //BWAPI::Broodwar->printf("%d %d %d %d", boxTop, boxBottom, boxLeft, boxRight);
}
BWAPI::TilePosition BuildingPlaceManager::getRefineryPosition() const
{
    BWAPI::TilePosition closestGeyser = BWAPI::TilePositions::None;
    double minGeyserDistanceFromHome = std::numeric_limits<double>::max();
    BWAPI::Position homePosition = BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation());

    // for each geyser
    for (auto& geyser : BWAPI::Broodwar->getStaticGeysers())
    {
        if (geyser->getType() != BWAPI::UnitTypes::Resource_Vespene_Geyser)
        {
            continue;
        }

        BWAPI::Position geyserPos = geyser->getInitialPosition();
        BWAPI::TilePosition geyserTilePos = geyser->getInitialTilePosition();

        // check to see if it's next to one of our depots
        bool nearDepot = false;
        for (auto& unit : BWAPI::Broodwar->self()->getUnits())
        {
            if (unit->getType().isResourceDepot() && unit->getDistance(geyserPos) < 300)
            {
                nearDepot = true;
            }
        }

        if (nearDepot)
        {
            double homeDistance = geyser->getDistance(homePosition);

            if (homeDistance < minGeyserDistanceFromHome)
            {
                minGeyserDistanceFromHome = homeDistance;
                closestGeyser = geyser->getInitialTilePosition();
            }
        }
    }

    return closestGeyser;
}

BWAPI::TilePosition BuildingPlaceManager::getDesiredPosition(BWAPI::UnitType type) const {
    BWAPI::TilePosition position;
    // if the building type is Pylon, we want to build them without overlap of range 
    if (type == BWAPI::UnitTypes::Protoss_Pylon) 
    {
        for(BWAPI::Unit u: BWAPI::Broodwar->self()->getUnits())
        {
            if (u->getType() == type) 
            {
                BWAPI::TilePosition pos = u->getTilePosition();
                position = BWAPI::TilePosition(pos.x + 16, pos.y);
                if (position.isValid()) 
                {
                    return position;
                }
                
            }
            
        }
    }
    return BWAPI::Broodwar->self()->getStartLocation();
}