// TrackNode.h

#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <algorithm>  // for std::min and std::max

class TrackNode {
public:
  TrackNode(const glm::vec2& position, float roadWidth = 30.0f)
    : m_position(position), m_roadWidth(roadWidth) {}

  const glm::vec2& getPosition() const { return m_position; }
  void setPosition(const glm::vec2& pos) { m_position = pos; }

  bool isSelected() const { return m_isSelected; }
  void setSelected(bool selected) { m_isSelected = selected; }

  bool isHovered() const { return m_isHovered; }
  void setHovered(bool hovered) { m_isHovered = hovered; }

  void setRoadWidth(float width) {
    // Use min/max instead of clamp
    m_roadWidth = std::max(10.0f, std::min(width, 100.0f));
    std::cout << "Node road width set to: " << m_roadWidth << std::endl;
  }

  float getRoadWidth() const {
    return m_roadWidth;
  }

  std::pair<glm::vec2, glm::vec2> getRoadEdgePoints(const glm::vec2& nextNodePos) const {
    glm::vec2 direction = nextNodePos - m_position;
    glm::vec2 perpendicular(-direction.y, direction.x);
    perpendicular = glm::normalize(perpendicular);
    glm::vec2 leftEdge = m_position + perpendicular * m_roadWidth;
    glm::vec2 rightEdge = m_position - perpendicular * m_roadWidth;
    return { leftEdge, rightEdge };
  }

private:
  glm::vec2 m_position;
  float m_roadWidth;
  bool m_isSelected = false;
  bool m_isHovered = false;
};