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

std::vector<SplineTrack::StartPosition> SplineTrack::calculateStartPositions() const {
  std::vector<StartPosition> positions;
  const TrackNode* startNode = getStartLineNode();
  if (!startNode || m_nodes.size() < 4) return positions;

  // Get detailed spline points for smooth curve following
  auto splinePoints = getSplinePoints(200);
  if (splinePoints.empty()) return positions;

  // Find start line index in spline points
  size_t startIndex = 0;
  float minDist = std::numeric_limits<float>::max();
  for (size_t i = 0; i < splinePoints.size(); i++) {
    float dist = glm::distance(startNode->getPosition(), splinePoints[i].position);
    if (dist < minDist) {
      minDist = dist;
      startIndex = i;
    }
  }

  // Calculate initial direction and perpendicular
  glm::vec2 direction;
  if (startIndex < splinePoints.size() - 1) {
    direction = glm::normalize(splinePoints[startIndex + 1].position - splinePoints[startIndex].position);
  }
  else {
    direction = glm::normalize(splinePoints[0].position - splinePoints[startIndex].position);
  }

  if (m_startConfig.isClockwise) {
    direction = -direction;
  }

  // Calculate perpendicular for lane offset
  glm::vec2 perpDirection(-direction.y, direction.x);

  // Calculate dynamic lane spacing based on road width at each position
  float baseRoadWidth = startNode->getRoadWidth();
  float baseLaneOffset = baseRoadWidth * m_startConfig.laneWidthRatio;

  // Calculate positions for each car
  for (int i = 0; i < m_startConfig.numPositions; i++) {
    bool isLeftLane = i % 2 == 0;
    int rowIndex = i / 2;

    // Calculate back distance along track
    float backDistance = rowIndex * m_startConfig.carSpacing;

    // Find position along spline at this distance
    size_t pointIndex = startIndex;
    float remainingDist = backDistance;

    // Walk backwards along spline points
    while (remainingDist > 0 && pointIndex > 0) {
      float segmentLength = glm::distance(
        splinePoints[pointIndex].position,
        splinePoints[pointIndex - 1].position
      );
      if (remainingDist > segmentLength) {
        remainingDist -= segmentLength;
        pointIndex--;
      }
      else {
        break;
      }
    }

    // Calculate local road width at this point for dynamic lane spacing
    float localRoadWidth = splinePoints[pointIndex].roadWidth;
    float localLaneOffset = localRoadWidth * m_startConfig.laneWidthRatio;

    // Calculate final position and direction
    glm::vec2 pointDirection;
    if (pointIndex > 0) {
      pointDirection = glm::normalize(
        splinePoints[pointIndex].position -
        splinePoints[pointIndex - 1].position
      );
    }
    else {
      pointDirection = glm::normalize(
        splinePoints[0].position -
        splinePoints.back().position
      );
    }

    if (m_startConfig.isClockwise) {
      pointDirection = -pointDirection;
    }

    // Calculate perpendicular at this point
    glm::vec2 pointPerp(-pointDirection.y, pointDirection.x);

    StartPosition pos;
    // Base position on spline
    pos.position = splinePoints[pointIndex].position;
    // Apply remaining distance
    pos.position -= pointDirection * remainingDist;
    // Apply lane offset using local road width
    pos.position += (isLeftLane ? pointPerp : -pointPerp) * localLaneOffset;
    // Set angle based on local track direction
    pos.angle = atan2(pointDirection.y, pointDirection.x);

    positions.push_back(pos);
  }

  return positions;
}

void SplineTrack::setStartLine(TrackNode* node) {
  // Clear existing start line
  for (auto& existingNode : m_nodes) {
    if (&existingNode != node && existingNode.isStartLine()) {
      std::cout << "Clearing previous start line\n";
      existingNode.setStartLine(false);
    }
  }

  // Set new start line
  if (node) {
    node->setStartLine(true);
    std::cout << "Set new start line\n";
  }
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

glm::vec2 SplineTrack::getTrackDirectionAtNode(const TrackNode* node) const {
  if (!node || m_nodes.size() < 2) return glm::vec2(1, 0);

  // Find the node's index
  auto it = std::find_if(m_nodes.begin(), m_nodes.end(),
    [node](const TrackNode& n) { return &n == node; });

  if (it == m_nodes.end()) return glm::vec2(1, 0);

  size_t nodeIndex = std::distance(m_nodes.begin(), it);
  size_t nextIndex = (nodeIndex + 1) % m_nodes.size();

  // Calculate direction to next node
  glm::vec2 direction = m_nodes[nextIndex].getPosition() - node->getPosition();
  return glm::normalize(direction);
}

std::pair<std::vector<glm::vec2>, std::vector<glm::vec2>>
SplineTrack::calculateStartLanes(const TrackNode* startNode, const glm::vec2& direction) const {
  std::vector<glm::vec2> leftLane, rightLane;
  if (!startNode) return { leftLane, rightLane };

  // Get perpendicular direction (left of track direction)
  glm::vec2 perpDirection(-direction.y, direction.x);

  // Calculate lane centers based on road width
  float halfRoadWidth = startNode->getRoadWidth() * 0.5f;
  float laneOffset = halfRoadWidth * 0.4f; // Position cars 40% from center to edge

  // Calculate lane center points
  glm::vec2 startPos = startNode->getPosition();
  glm::vec2 leftCenter = startPos + perpDirection * laneOffset;
  glm::vec2 rightCenter = startPos - perpDirection * laneOffset;

  leftLane.push_back(leftCenter);
  rightLane.push_back(rightCenter);

  return { leftLane, rightLane };
}
