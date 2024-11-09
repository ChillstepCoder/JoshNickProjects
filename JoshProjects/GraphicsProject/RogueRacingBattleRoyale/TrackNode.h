// TrackNode.h

#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <algorithm>  // for std::min and std::max

class TrackNode {
public:
  TrackNode(const glm::vec2& position, float roadWidth = 30.0f,
    const glm::vec2& offroadWidth = glm::vec2(20.0f, 20.0f),
    const glm::vec2& barrierDistance = glm::vec2(0.0f, 0.0f))
    : m_position(position)
    , m_roadWidth(roadWidth)
    , m_offroadWidth(offroadWidth)
    , m_barrierDistance(barrierDistance) {}


  bool isSelected() const { return m_isSelected; }
  bool isHovered() const { return m_isHovered; }
  bool isStartLine() const { return m_isStartLine; }

  // Getters
  const glm::vec2& getOffroadWidth() const { return m_offroadWidth; }
  float getRoadWidth() const {
    return m_roadWidth;
  }

  const glm::vec2& getPosition() const { return m_position; }
  const glm::vec2& getBarrierDistance() const { return m_barrierDistance; }

  std::pair<glm::vec2, glm::vec2> getRoadEdgePoints(const glm::vec2& nextNodePos) const {
    glm::vec2 direction = nextNodePos - m_position;
    glm::vec2 perpendicular(-direction.y, direction.x);
    perpendicular = glm::normalize(perpendicular);
    glm::vec2 leftEdge = m_position + perpendicular * m_roadWidth;
    glm::vec2 rightEdge = m_position - perpendicular * m_roadWidth;
    return { leftEdge, rightEdge };
  }


  // Setters
  void setPosition(const glm::vec2& pos) { m_position = pos; }
  void setSelected(bool selected) { m_isSelected = selected; }
  void setStartLine(bool isStart) { m_isStartLine = isStart; }
  void setHovered(bool hovered) { m_isHovered = hovered; }

  void setRoadWidth(float width) {
    // Use min/max instead of clamp
    m_roadWidth = std::max(10.0f, std::min(width, 100.0f));
    std::cout << "Node road width set to: " << m_roadWidth << std::endl;
  }

  void setOffroadWidth(const glm::vec2& width) {
    // Left side (x) and right side (y)
    m_offroadWidth = glm::clamp(width, glm::vec2(0.0f), glm::vec2(100.0f));
  }

  void setBarrierDistance(const glm::vec2& distance) {
    // x = left side, y = right side
    m_barrierDistance = glm::clamp(distance, glm::vec2(0.0f), glm::vec2(100.0f));
  }

private:
  glm::vec2 m_position;
  float m_roadWidth;
  glm::vec2 m_offroadWidth;
  glm::vec2 m_barrierDistance;

  bool m_isSelected = false;
  bool m_isHovered = false;
  bool m_isStartLine = false;
};
