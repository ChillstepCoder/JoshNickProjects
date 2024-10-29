// SplineTrack.h
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "TrackNode.h"

class SplineTrack {
public:
  SplineTrack();

  void createDefaultTrack();
  void addNode(const glm::vec2& position);
  void removeNode(size_t index);

  TrackNode* getNodeAtPosition(const glm::vec2& position, float threshold = 10.0f);
  std::vector<glm::vec2> getSplinePoints(int subdivisions = 50) const;

  const std::vector<TrackNode>& getNodes() const { return m_nodes; }
  std::vector<TrackNode>& getNodes() { return m_nodes; }

private:
  std::vector<TrackNode> m_nodes;
  glm::vec2 calculateSplinePoint(float t) const;
  glm::vec2 catmullRom(const glm::vec2& p0, const glm::vec2& p1,
    const glm::vec2& p2, const glm::vec2& p3, float t) const;
};
