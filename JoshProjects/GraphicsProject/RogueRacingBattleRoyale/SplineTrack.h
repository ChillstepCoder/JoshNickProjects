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
    glm::vec2 offroadWidth;  // x = left side, y = right side
  };

  SplineTrack();
  void createDefaultTrack();
  void addNode(const glm::vec2& position);
  void removeNode(size_t index);
  TrackNode* getNodeAtPosition(const glm::vec2& position, float threshold = 10.0f);
  std::vector<SplinePointInfo> getSplinePoints(int subdivisions = 50) const;
  const std::vector<TrackNode>& getNodes() const { return m_nodes; }
  std::vector<TrackNode>& getNodes() { return m_nodes; }

  void insertNode(size_t index, const TrackNode& node) {
    if (index <= m_nodes.size()) {
      m_nodes.insert(m_nodes.begin() + index, node);
    }
  }

private:
  std::vector<TrackNode> m_nodes;
  glm::vec2 catmullRom(const glm::vec2& p0, const glm::vec2& p1,
    const glm::vec2& p2, const glm::vec2& p3, float t) const;
  glm::vec2 catmullRomVec2(const glm::vec2& p0, const glm::vec2& p1,
    const glm::vec2& p2, const glm::vec2& p3, float t) const;
  float catmullRomValue(float p0, float p1, float p2, float p3, float t) const;
};
