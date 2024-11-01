// SplineTrack.cpp
#include "SplineTrack.h"
#include <cmath>

SplineTrack::SplineTrack() {
  createDefaultTrack();
}

void SplineTrack::createDefaultTrack() {
  m_nodes.clear();
  // Create a circular track with 4 nodes
  float radius = 100.0f;
  // Create nodes with explicit positions and default road width
  m_nodes.emplace_back(glm::vec2(radius, 0.0f));
  m_nodes.emplace_back(glm::vec2(0.0f, radius));
  m_nodes.emplace_back(glm::vec2(-radius, 0.0f));
  m_nodes.emplace_back(glm::vec2(0.0f, -radius));
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

      // Get four points for the spline segment
      const glm::vec2& p1 = m_nodes[i].getPosition();
      const glm::vec2& p2 = m_nodes[(i + 1) % m_nodes.size()].getPosition();
      const glm::vec2& p3 = m_nodes[(i + 2) % m_nodes.size()].getPosition();
      const glm::vec2& p0 = m_nodes[(i > 0 ? i - 1 : m_nodes.size() - 1)].getPosition();

      // Get the road widths for interpolation
      float w1 = m_nodes[i].getRoadWidth();
      float w2 = m_nodes[(i + 1) % m_nodes.size()].getRoadWidth();
      float w3 = m_nodes[(i + 2) % m_nodes.size()].getRoadWidth();
      float w0 = m_nodes[(i > 0 ? i - 1 : m_nodes.size() - 1)].getRoadWidth();

      SplinePointInfo pointInfo;
      pointInfo.position = catmullRom(p0, p1, p2, p3, t);
      pointInfo.roadWidth = catmullRomValue(w0, w1, w2, w3, t);
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
