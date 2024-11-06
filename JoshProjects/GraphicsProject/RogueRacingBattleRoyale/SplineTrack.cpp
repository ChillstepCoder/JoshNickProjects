// SplineTrack.cpp
#include "SplineTrack.h"
#include <cmath>

SplineTrack::SplineTrack() {
  createDefaultTrack();
}

void SplineTrack::createDefaultTrack() {
  m_nodes.clear();
  float radius = 100.0f;

  // Create nodes with explicit positions and default properties
  TrackNode node1(glm::vec2(radius, 0.0f));
  TrackNode node2(glm::vec2(0.0f, radius));
  TrackNode node3(glm::vec2(-radius, 0.0f));
  TrackNode node4(glm::vec2(0.0f, -radius));

  // Set default barrier distances
  glm::vec2 defaultBarrierDist(10.0f, 10.0f); // Increased from 0.0f
  node1.setBarrierDistance(defaultBarrierDist);
  node2.setBarrierDistance(defaultBarrierDist);
  node3.setBarrierDistance(defaultBarrierDist);
  node4.setBarrierDistance(defaultBarrierDist);

  m_nodes.push_back(node1);
  m_nodes.push_back(node2);
  m_nodes.push_back(node3);
  m_nodes.push_back(node4);
}

void SplineTrack::addNode(const glm::vec2& position) {
  m_nodes.push_back(TrackNode(position));
}

void SplineTrack::removeNode(size_t index) {
  if (index < m_nodes.size()) {
    m_nodes.erase(m_nodes.begin() + index);
  }
}

TrackNode* SplineTrack::getNodeAtPosition(const glm::vec2& position, float threshold) {
  for (auto& node : m_nodes) {
    glm::vec2 diff = node.getPosition() - position;
    float distSquared = diff.x * diff.x + diff.y * diff.y;
    if (distSquared < threshold * threshold) {
      return &node;
    }
  }
  return nullptr;
}

std::vector<SplineTrack::SplinePointInfo> SplineTrack::getSplinePoints(int subdivisions) const {
  std::vector<SplinePointInfo> points;
  if (m_nodes.size() < 4) return points;

  for (size_t i = 0; i < m_nodes.size(); ++i) {
    for (int j = 0; j < subdivisions; ++j) {
      float t = static_cast<float>(j) / subdivisions;

      // Get positions and widths from nodes
      const glm::vec2& p1 = m_nodes[i].getPosition();
      const glm::vec2& p2 = m_nodes[(i + 1) % m_nodes.size()].getPosition();
      const glm::vec2& p3 = m_nodes[(i + 2) % m_nodes.size()].getPosition();
      const glm::vec2& p0 = m_nodes[(i > 0 ? i - 1 : m_nodes.size() - 1)].getPosition();

      float w1 = m_nodes[i].getRoadWidth();
      float w2 = m_nodes[(i + 1) % m_nodes.size()].getRoadWidth();
      float w3 = m_nodes[(i + 2) % m_nodes.size()].getRoadWidth();
      float w0 = m_nodes[(i > 0 ? i - 1 : m_nodes.size() - 1)].getRoadWidth();

      const glm::vec2& o1 = m_nodes[i].getOffroadWidth();
      const glm::vec2& o2 = m_nodes[(i + 1) % m_nodes.size()].getOffroadWidth();
      const glm::vec2& o3 = m_nodes[(i + 2) % m_nodes.size()].getOffroadWidth();
      const glm::vec2& o0 = m_nodes[(i > 0 ? i - 1 : m_nodes.size() - 1)].getOffroadWidth();

      // Add barrier distance interpolation
      const glm::vec2& b1 = m_nodes[i].getBarrierDistance();
      const glm::vec2& b2 = m_nodes[(i + 1) % m_nodes.size()].getBarrierDistance();
      const glm::vec2& b3 = m_nodes[(i + 2) % m_nodes.size()].getBarrierDistance();
      const glm::vec2& b0 = m_nodes[(i > 0 ? i - 1 : m_nodes.size() - 1)].getBarrierDistance();

      SplinePointInfo pointInfo;
      pointInfo.position = catmullRom(p0, p1, p2, p3, t);
      pointInfo.roadWidth = catmullRomValue(w0, w1, w2, w3, t);
      pointInfo.offroadWidth = catmullRomVec2(o0, o1, o2, o3, t);
      pointInfo.barrierDistance = catmullRomVec2(b0, b1, b2, b3, t);  // Add this line

      points.push_back(pointInfo);
    }
  }

  return points;
}

glm::vec2 SplineTrack::catmullRom(const glm::vec2& p0, const glm::vec2& p1,
  const glm::vec2& p2, const glm::vec2& p3, float t) const {
  float t2 = t * t;
  float t3 = t2 * t;

  glm::vec2 result = p1 * 2.0f;
  result += (-p0 + p2) * t;
  result += (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2;
  result += (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3;
  result *= 0.5f;

  return result;
}

glm::vec2 SplineTrack::catmullRomVec2(const glm::vec2& p0, const glm::vec2& p1,
  const glm::vec2& p2, const glm::vec2& p3, float t) const {
  float t2 = t * t;
  float t3 = t2 * t;
  glm::vec2 result = p1 * 2.0f;
  result += (-p0 + p2) * t;
  result += (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2;
  result += (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3;
  result *= 0.5f;
  return result;
}

float SplineTrack::catmullRomValue(float p0, float p1, float p2, float p3, float t) const {
  float t2 = t * t;
  float t3 = t2 * t;

  float result = p1 * 2.0f;
  result += (-p0 + p2) * t;
  result += (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2;
  result += (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3;
  result *= 0.5f;

  return result;
}
