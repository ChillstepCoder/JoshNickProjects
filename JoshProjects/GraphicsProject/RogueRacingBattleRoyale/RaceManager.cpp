#include "RaceManager.h"

#include <numeric>

void RaceManager::updateRacePositions(std::vector<std::unique_ptr<Car>>& cars) {
  if (cars.empty()) return;

  // Create sorted indices based on race progress
  std::vector<size_t> indices(cars.size());
  std::iota(indices.begin(), indices.end(), 0);

  // Sort with null pointer protection
  std::sort(indices.begin(), indices.end(),
    [&cars](size_t a, size_t b) {
      // Null check first
      if (!cars[a] || !cars[b]) return false;

      float progressA = cars[a]->getTotalRaceProgress();
      float progressB = cars[b]->getTotalRaceProgress();

      // If progress is very close, break ties by distance to ideal racing line
      if (std::abs(progressA - progressB) < 0.001f) {
        return progressA > progressB;
      }
      return progressA > progressB;
    });

  // Assign positions (with null check)
  for (size_t i = 0; i < indices.size(); i++) {
    if (cars[indices[i]]) {
      cars[indices[i]]->getProperties().racePosition = static_cast<int>(i + 1);
    }
  }
}
