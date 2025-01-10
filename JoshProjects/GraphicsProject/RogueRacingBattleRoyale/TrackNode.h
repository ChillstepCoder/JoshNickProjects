// TrackNode.h

#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <algorithm>

// Forward declaration
class SplineTrack;

class TrackNode {
public:
  TrackNode(const glm::vec2& position, float roadWidth = 30.0f,
    const glm::vec2& offroadWidth = glm::vec2(20.0f, 20.0f),
    const glm::vec2& barrierDistance = glm::vec2(0.0f, 0.0f));

  bool isSelected() const;
  bool isHovered() const;
  bool isStartLine() const;

  // Getters
  const glm::vec2& getOffroadWidth() const;
  float getRoadWidth() const;
  const glm::vec2& getPosition() const;
  const glm::vec2& getBarrierDistance() const;
  std::pair<glm::vec2, glm::vec2> getRoadEdgePoints(const glm::vec2& nextNodePos) const;

  // Setters
  void setPosition(const glm::vec2& pos);
  void setSelected(bool selected);
  void setStartLine(bool isStart);
  void setHovered(bool hovered);
  void setRoadWidth(float width);
  void setOffroadWidth(const glm::vec2& width);
  void setBarrierDistance(const glm::vec2& distance);
  void setParentTrack(SplineTrack* track);

private:
  SplineTrack* m_parentTrack = nullptr;
  glm::vec2 m_position;
  float m_roadWidth;
  glm::vec2 m_offroadWidth;
  glm::vec2 m_barrierDistance;
  bool m_isSelected = false;
  bool m_isHovered = false;
  bool m_isStartLine = false;
};
