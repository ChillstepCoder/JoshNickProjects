#include "RaceManager.h"

#include <numeric>

void RaceManager::updateRacePositions(std::vector<std::unique_ptr<Car>>& cars) {
  if (cars.empty()) return;

  // Create sorted indices based on race progress
  std::vector<size_t> indices(cars.size());
  std::iota(indices.begin(), indices.end(), 0);

  // Sort with better position determination
  std::sort(indices.begin(), indices.end(),
    [&cars](size_t a, size_t b) {
      if (!cars[a] || !cars[b]) return false;

      auto& propsA = cars[a]->getProperties();
      auto& propsB = cars[b]->getProperties();

      // First compare laps
      if (propsA.currentLap != propsB.currentLap) {
        return propsA.currentLap > propsB.currentLap;
      }

      // If on same lap, compare progress
      float progressA = cars[a]->getTotalRaceProgress();
      float progressB = cars[b]->getTotalRaceProgress();

      return progressA > progressB;
    });

  // Assign positions (with null check)
  for (size_t i = 0; i < indices.size(); i++) {
    if (cars[indices[i]]) {
      cars[indices[i]]->getProperties().racePosition = static_cast<int>(i + 1);
    }
  }
}
