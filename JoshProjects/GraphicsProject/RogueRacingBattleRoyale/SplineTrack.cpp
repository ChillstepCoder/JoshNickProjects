// SplineTrack.cpp
#include "SplineTrack.h"
#define _USE_MATH_DEFINES
#include <cmath>

SplineTrack::SplineTrack() : m_cacheValid(false) {
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

  // Clear cache and mark as valid
  invalidateCache();
  m_cacheValid = true;
}

void SplineTrack::addNode(const glm::vec2& position) {
  m_nodes.push_back(TrackNode(position));
  invalidateCache();
}

void SplineTrack::removeNode(size_t index) {
  if (index < m_nodes.size()) {
    m_nodes.erase(m_nodes.begin() + index);
    invalidateCache();
  }
}

void SplineTrack::insertNode(size_t index, const TrackNode& node) {
  if (index <= m_nodes.size()) {
    m_nodes.insert(m_nodes.begin() + index, node);
    invalidateCache();
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

  auto splinePoints = getSplinePoints(200);
  if (splinePoints.empty()) return positions;

  // Find start line index
  size_t startIndex = 0;
  float minDist = std::numeric_limits<float>::max();
  for (size_t i = 0; i < splinePoints.size(); i++) {
    float dist = glm::distance(startNode->getPosition(), splinePoints[i].position);
    if (dist < minDist) {
      minDist = dist;
      startIndex = i;
    }
  }

  // Calculate rows and positions per row
  int carsPerRow = (m_startConfig.numPositions + m_startConfig.numLanes - 1) / m_startConfig.numLanes;

  size_t numPoints = splinePoints.size();

  // For each starting position
  for (int i = 0; i < m_startConfig.numPositions; i++) {
    StartPosition pos;
    int laneIndex = i % m_startConfig.numLanes;  // Which lane (0 to numLanes-1)
    int rowIndex = i / m_startConfig.numLanes;   // Which row back from start

    // Calculate position index along spline by walking along the track
    float targetDistance = (rowIndex + 1) * m_startConfig.carSpacing;
    size_t currentIndex = startIndex;
    float accumulatedDistance = 0.0f;

    // Determine direction to walk along spline based on track direction
    int indexStep = m_startConfig.isClockwise ? 1 : -1;

    // Variables to store position and direction
    glm::vec2 position;
    glm::vec2 direction;

    // Walk along spline points to find correct position
    std::vector<size_t> pathIndices;
    pathIndices.push_back(startIndex);

    while (accumulatedDistance < targetDistance) {
      size_t nextIndex = (currentIndex + indexStep + numPoints) % numPoints;
      float segmentLength = glm::distance(
        splinePoints[currentIndex].position,
        splinePoints[nextIndex].position
      );

      if (accumulatedDistance + segmentLength > targetDistance) {
        float remainingDist = targetDistance - accumulatedDistance;
        float t = remainingDist / segmentLength;
        pathIndices.push_back(nextIndex);

        // Interpolate position and direction
        glm::vec2 p1 = splinePoints[currentIndex].position;
        glm::vec2 p2 = splinePoints[nextIndex].position;
        position = glm::mix(p1, p2, t);
        direction = glm::normalize(p1 - p2);

        break;
      }

      accumulatedDistance += segmentLength;
      currentIndex = nextIndex;
      pathIndices.push_back(currentIndex);

      // Prevent infinite loop
      if (pathIndices.size() > numPoints) {
        break;
      }
    }

    // If we didn't break early, use the last known position
    if (accumulatedDistance >= targetDistance) {
      position = splinePoints[currentIndex].position;
      size_t prevIndex = (currentIndex - indexStep + numPoints) % numPoints;
      direction = glm::normalize(position - splinePoints[prevIndex].position);
    }

    // Calculate lane offset
    glm::vec2 perpDirection(-direction.y, direction.x);
    float roadWidth = splinePoints[currentIndex].roadWidth;
    float totalLaneWidth = roadWidth * 1.0f;  // Use full road width

    float laneOffset = 0.0f;
    if (m_startConfig.numLanes == 1) {
      laneOffset = 0.0f;
    }
    else {
      float normalizedLanePosition = (float)laneIndex / (float)(m_startConfig.numLanes - 1) - 0.5f;
      laneOffset = normalizedLanePosition * totalLaneWidth;

      // Flip the offset for clockwise tracks to put cars on the correct side
      if (m_startConfig.isClockwise) {
        laneOffset = -laneOffset;
      }
    }

    // Set position and angle
    pos.position = position + perpDirection * laneOffset;
    pos.angle = atan2(direction.y, direction.x);

    positions.push_back(pos);
  }

  return positions;
}

void SplineTrack::modifyNode(size_t index, const TrackNode& newNode) {
  if (index < m_nodes.size()) {
    m_nodes[index] = newNode;
    invalidateCache();
  }
}

void SplineTrack::setStartLine(TrackNode* node) {
  // Clear existing start line
  for (auto& existingNode : m_nodes) {
    if (&existingNode != node && existingNode.isStartLine()) {
      existingNode.setStartLine(false);
    }
  }

  // Set new start line
  if (node) {
    node->setStartLine(true);
  }
  invalidateCache();
}

std::vector<SplineTrack::SplinePointInfo> SplineTrack::buildSplinePoints(int subdivisions) const {
  std::vector<SplinePointInfo> points;
  if (m_nodes.empty() || subdivisions <= 0) return points;

  points.reserve(subdivisions + 1);

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

std::vector<SplineTrack::SplinePointInfo> SplineTrack::getSplinePoints(int subdivisions) const {
  if (!m_cacheValid || m_nodes.empty() || subdivisions <= 0) {
    return std::vector<SplinePointInfo>();
  }

  // Check if we have this subdivision level cached
  auto it = m_splinePointCache.find(subdivisions);
  if (it != m_splinePointCache.end()) {
    return it->second;
  }

  // Build and cache the spline points for this subdivision level
  auto points = buildSplinePoints(subdivisions);
  m_splinePointCache[subdivisions] = points;
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

glm::vec2 SplineTrack::catmullRomDerivative(const glm::vec2& p0, const glm::vec2& p1,
  const glm::vec2& p2, const glm::vec2& p3, float t) const {
  float t2 = t * t;

  glm::vec2 result = (-p0 + p2) * 0.5f;
  result += (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t;
  result += (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * 1.5f * t2;

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

TrackNode* SplineTrack::getStartLineNode() {
  for (auto& node : m_nodes) {
    if (node.isStartLine()) {
      return &node;
    }
  }
  return nullptr;
}

const TrackNode* SplineTrack::getStartLineNode() const {
  for (const auto& node : m_nodes) {
    if (node.isStartLine()) {
      return &node;
    }
  }
  return nullptr;
}

glm::vec2 SplineTrack::getTrackDirectionAtNode(const TrackNode* node) const {
  if (!node || m_nodes.size() < 4) return glm::vec2(1, 0);

  // Get detailed spline points
  auto splinePoints = getSplinePoints(200);  // High-resolution spline
  if (splinePoints.empty()) return glm::vec2(1, 0);

  // Find the index of the spline point closest to the node
  size_t closestIndex = 0;
  float minDist = std::numeric_limits<float>::max();
  for (size_t i = 0; i < splinePoints.size(); ++i) {
    float dist = glm::distance(node->getPosition(), splinePoints[i].position);
    if (dist < minDist) {
      minDist = dist;
      closestIndex = i;
    }
  }

  size_t numPoints = splinePoints.size();

  // Determine direction at that point
  int indexStep = 1; // Counter-clockwise by default
  if (getStartPositionConfig().isClockwise) {
    indexStep = -1;
  }

  size_t nextIndex = (closestIndex + indexStep + numPoints) % numPoints;
  glm::vec2 pCurr = splinePoints[closestIndex].position;
  glm::vec2 pNext = splinePoints[nextIndex].position;

  glm::vec2 direction = glm::normalize(pNext - pCurr);

  // For clockwise tracks, reverse the direction
  if (getStartPositionConfig().isClockwise) {
    direction = -direction;
  }

  return direction;
}

std::vector<glm::vec2> SplineTrack::getBarrierVertices() const {
  std::vector<glm::vec2> barrierVertices;

  auto splinePoints = getSplinePoints(200);  // Use appropriate subdivisions

  if (splinePoints.size() < 2) return barrierVertices;

  size_t numPoints = splinePoints.size();

  for (size_t i = 0; i < numPoints; ++i) {
    const auto& current = splinePoints[i];
    const auto& next = splinePoints[(i + 1) % numPoints];

    glm::vec2 direction = glm::normalize(next.position - current.position);
    glm::vec2 perp(-direction.y, direction.x);

    // Left barrier point
    if (current.barrierDistance.x > 0.0f) {
      glm::vec2 leftBarrierPos = current.position + perp * (current.roadWidth + current.barrierDistance.x);
      barrierVertices.push_back(leftBarrierPos);
    }

    // Right barrier point
    if (current.barrierDistance.y > 0.0f) {
      glm::vec2 rightBarrierPos = current.position - perp * (current.roadWidth + current.barrierDistance.y);
      barrierVertices.push_back(rightBarrierPos);
    }
  }

  return barrierVertices;
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

void SplineTrack::invalidateCache() {
  m_splinePointCache.clear();
  if (!m_nodes.empty()) {
    m_cacheValid = true;
  }
  else {
    m_cacheValid = false;
  }
}

