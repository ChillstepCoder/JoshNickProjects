// SplineTrack.h
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "TrackNode.h"
class SplineTrack {
public:
  struct SplinePointInfo {
    glm::vec2 position;
    float roadWidth;
    glm::vec2 offroadWidth;
    glm::vec2 barrierDistance;
  };

  struct StartPositionConfig {
    int numPositions = 10;        // Total number of cars
    int numLanes = 2;             // Number of parallel lanes (1-4)
    bool isClockwise = false;      // Track direction
    float carSpacing = 30.0f;     // Distance between cars front-to-back
    float laneWidthRatio = 0.4f;  // What fraction of total road width to use for lanes
  };

  struct StartPosition {
    glm::vec2 position;
    float angle;  // Angle in radians
  };

  SplineTrack();
  void createDefaultTrack();
  void addNode(const glm::vec2& position);
  void removeNode(size_t index);
  void insertNode(size_t index, const TrackNode& node) {
    if (index <= m_nodes.size()) {
      m_nodes.insert(m_nodes.begin() + index, node);
    }
  }

  bool isDefaultDirection() const {
      return !m_startConfig.isClockwise;  // Counter-clockwise is default
  }

  // Getters
  std::string getDirectionString() const {
      return m_startConfig.isClockwise ? "Clockwise" : "Counter-clockwise";
  }
  TrackNode* getNodeAtPosition(const glm::vec2& position, float threshold = 10.0f);
  std::vector<SplinePointInfo> getSplinePoints(int subdivisions = 50) const;
  const std::vector<TrackNode>& getNodes() const { return m_nodes; }
  std::vector<TrackNode>& getNodes() { return m_nodes; }

  TrackNode* getStartLineNode() {
    for (auto& node : m_nodes) {
      if (node.isStartLine()) {
        return &node;
      }
    }
    return nullptr;
  }

  const TrackNode* getStartLineNode() const {
    for (const auto& node : m_nodes) {
      if (node.isStartLine()) {
        return &node;
      }
    }
    return nullptr;
  }

  glm::vec2 getTrackDirectionAtNode(const TrackNode* node) const;

  // Start Position Configuration - Keep only one set of these methods
  StartPositionConfig& getStartPositionConfig() { return m_startConfig; }
  const StartPositionConfig& getStartPositionConfig() const { return m_startConfig; }
  void setStartPositionConfig(const StartPositionConfig& config) { m_startConfig = config; }
  bool isValidStartPositionCount(int count) const { return count >= 2 && count <= 20; }

  // Setters
  void setStartLine(TrackNode* node);

  // Start points calculation
  std::vector<StartPosition> calculateStartPositions() const;

private:
  std::vector<TrackNode> m_nodes;
  StartPositionConfig m_startConfig;

  glm::vec2 catmullRom(const glm::vec2& p0, const glm::vec2& p1,
    const glm::vec2& p2, const glm::vec2& p3, float t) const;
  glm::vec2 catmullRomVec2(const glm::vec2& p0, const glm::vec2& p1,
    const glm::vec2& p2, const glm::vec2& p3, float t) const;
  glm::vec2 catmullRomDerivative(const glm::vec2& p0, const glm::vec2& p1,
    const glm::vec2& p2, const glm::vec2& p3, float t) const;
  float catmullRomValue(float p0, float p1, float p2, float p3, float t) const;

  std::pair<std::vector<glm::vec2>, std::vector<glm::vec2>> calculateStartLanes(
    const TrackNode* startNode, const glm::vec2& direction) const;
};
