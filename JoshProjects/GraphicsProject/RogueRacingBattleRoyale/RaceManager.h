// RaceManager.h

#pragma once

#include <memory>
#include <vector>
#include "Car.h"

class RaceManager
{
public:
  void updateRacePositions(std::vector<std::unique_ptr<Car>>& cars, int finishedOffset);
private:
};

