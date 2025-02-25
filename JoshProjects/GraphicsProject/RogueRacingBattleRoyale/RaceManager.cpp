// RaceManager.cpp

#include "RaceManager.h"

#include <numeric>

void RaceManager::updateRacePositions(std::vector<std::unique_ptr<Car>>& cars, int finishedOffset) {
  if (cars.empty()) return;

  // For cars that are still racing, determine their rank among themselves.
  std::vector<size_t> activeIndices;
  for (size_t i = 0; i < cars.size(); i++) {
    if (cars[i] && !cars[i]->getProperties().finished) {
      activeIndices.push_back(i);
    }
  }

  std::sort(activeIndices.begin(), activeIndices.end(), [&cars](size_t a, size_t b) {
    // Compare by lap first, then by progress along the lap.
    auto& propsA = cars[a]->getProperties();
    auto& propsB = cars[b]->getProperties();

    if (propsA.currentLap != propsB.currentLap)
      return propsA.currentLap > propsB.currentLap;

    // For same lap, use total race progress.
    float progressA = cars[a]->getTotalRaceProgress();
    float progressB = cars[b]->getTotalRaceProgress();
    return progressA > progressB;
    });

  for (size_t i = 0; i < activeIndices.size(); i++) {
    if (cars[activeIndices[i]]) {
      cars[activeIndices[i]]->getProperties().racePosition = static_cast<int>(i + 1 + finishedOffset);
    }
  }

}

