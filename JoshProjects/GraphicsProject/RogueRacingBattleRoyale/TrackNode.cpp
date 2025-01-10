// TrackNode.cpp

#include "TrackNode.h"
#include "SplineTrack.h"

TrackNode::TrackNode(const glm::vec2& position, float roadWidth,
  const glm::vec2& offroadWidth, const glm::vec2& barrierDistance)
  : m_position(position)
  , m_roadWidth(roadWidth)
  , m_offroadWidth(offroadWidth)
  , m_barrierDistance(barrierDistance) {
}

bool TrackNode::isSelected() const {
  return m_isSelected;
}

bool TrackNode::isHovered() const {
  return m_isHovered;
}

bool TrackNode::isStartLine() const {
  return m_isStartLine;
}

const glm::vec2& TrackNode::getOffroadWidth() const {
  return m_offroadWidth;
}

float TrackNode::getRoadWidth() const {
  return m_roadWidth;
}

const glm::vec2& TrackNode::getPosition() const {
  return m_position;
}

const glm::vec2& TrackNode::getBarrierDistance() const {
  return m_barrierDistance;
}

std::pair<glm::vec2, glm::vec2> TrackNode::getRoadEdgePoints(const glm::vec2& nextNodePos) const {
  glm::vec2 direction = nextNodePos - m_position;
  glm::vec2 perpendicular(-direction.y, direction.x);
  perpendicular = glm::normalize(perpendicular);
  glm::vec2 leftEdge = m_position + perpendicular * m_roadWidth;
  glm::vec2 rightEdge = m_position - perpendicular * m_roadWidth;
  return { leftEdge, rightEdge };
}

void TrackNode::setPosition(const glm::vec2& pos) {
  m_position = pos;
  if (m_parentTrack) {
    m_parentTrack->invalidateCache();
  }
}

void TrackNode::setSelected(bool selected) {
  m_isSelected = selected;
}

void TrackNode::setStartLine(bool isStart) {
  m_isStartLine = isStart;
}

void TrackNode::setHovered(bool hovered) {
  m_isHovered = hovered;
}

void TrackNode::setRoadWidth(float width) {
  m_roadWidth = std::max<float>(10.0f, std::min<float>(width, 100.0f));
  std::cout << "Node road width set to: " << m_roadWidth << std::endl;
}

void TrackNode::setOffroadWidth(const glm::vec2& width) {
  m_offroadWidth = glm::clamp(width, glm::vec2(0.0f), glm::vec2(100.0f));
}

void TrackNode::setBarrierDistance(const glm::vec2& distance) {
  m_barrierDistance = glm::clamp(distance, glm::vec2(0.0f), glm::vec2(100.0f));
}

void TrackNode::setParentTrack(SplineTrack* track) {
  m_parentTrack = track;
}
