#pragma once
#include <BWAPI.h>
#include "Grid.hpp"

class BuildingPlaceManager {
private:
    Grid<int> m_reserveMap;

    int m_boxTop = std::numeric_limits<int>::max();
    int m_boxBottom = std::numeric_limits<int>::lowest();
    int m_boxLeft = std::numeric_limits<int>::max();
    int m_boxRight = std::numeric_limits<int>::lowest();
public:
    BuildingPlaceManager();
    void computeResourceBox();
    BWAPI::TilePosition getRefineryPosition() const;

    BWAPI::TilePosition getDesiredPosition(BWAPI::UnitType type) const;
};
