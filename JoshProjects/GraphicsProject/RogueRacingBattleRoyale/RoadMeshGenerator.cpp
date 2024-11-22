// RoadMeshGenerator.cpp

#include "RoadMeshGenerator.h"
#include <iostream>

// MeshData implementations
RoadMeshGenerator::MeshData::MeshData() : vao(0), vbo(0), ibo(0) {}

RoadMeshGenerator::MeshData::~MeshData() {
  cleanup();
}

RoadMeshGenerator::MeshData::MeshData(const MeshData& other) {
  vertices = other.vertices;
  indices = other.indices;
  vao = 0;
  vbo = 0;
  ibo = 0;
  if (!vertices.empty() && !indices.empty()) {
    createBuffers(*this);
  }
}

RoadMeshGenerator::MeshData& RoadMeshGenerator::MeshData::operator=(const MeshData& other) {
  if (this != &other) {
    cleanup();
    vertices = other.vertices;
    indices = other.indices;
    vao = 0;
    vbo = 0;
    ibo = 0;
    if (!vertices.empty() && !indices.empty()) {
      createBuffers(*this);
    }
  }
  return *this;
}

void RoadMeshGenerator::MeshData::cleanup() {
  if (vao != 0) {
    glDeleteVertexArrays(1, &vao);
    vao = 0;
  }
  if (vbo != 0) {
    glDeleteBuffers(1, &vbo);
    vbo = 0;
  }
  if (ibo != 0) {
    glDeleteBuffers(1, &ibo);
    ibo = 0;
  }
}

// StartLineMeshData implementations
RoadMeshGenerator::StartLineMeshData::StartLineMeshData() : vao(0), vbo(0), ibo(0) {}

RoadMeshGenerator::StartLineMeshData::~StartLineMeshData() {
  cleanup();
}

RoadMeshGenerator::StartLineMeshData::StartLineMeshData(const StartLineMeshData& other) {
  vertices = other.vertices;
  indices = other.indices;
  vao = 0;
  vbo = 0;
  ibo = 0;
  if (!vertices.empty() && !indices.empty()) {
    createStartLineBuffers(*this);
  }
}

RoadMeshGenerator::StartLineMeshData& RoadMeshGenerator::StartLineMeshData::operator=(const StartLineMeshData& other) {
  if (this != &other) {
    cleanup();
    vertices = other.vertices;
    indices = other.indices;
    vao = 0;
    vbo = 0;
    ibo = 0;
    if (!vertices.empty() && !indices.empty()) {
      createStartLineBuffers(*this);
    }
  }
  return *this;
}

void RoadMeshGenerator::StartLineMeshData::cleanup() {
  if (vao != 0) {
    glDeleteVertexArrays(1, &vao);
    vao = 0;
  }
  if (vbo != 0) {
    glDeleteBuffers(1, &vbo);
    vbo = 0;
  }
  if (ibo != 0) {
    glDeleteBuffers(1, &ibo);
    ibo = 0;
  }
}

// BarrierMeshData implementations
RoadMeshGenerator::BarrierMeshData::BarrierMeshData(const BarrierMeshData& other)
  : leftSide(other.leftSide)
  , rightSide(other.rightSide) {
}

RoadMeshGenerator::BarrierMeshData& RoadMeshGenerator::BarrierMeshData::operator=(const BarrierMeshData& other) {
  if (this != &other) {
    leftSide = other.leftSide;
    rightSide = other.rightSide;
  }
  return *this;
}

RoadMeshGenerator::BarrierMeshData RoadMeshGenerator::generateBarrierMesh(const SplineTrack& track, int baseLOD) {
  BarrierMeshData mesh;
  auto splinePoints = track.getSplinePoints(baseLOD);

  if (splinePoints.size() < 2) return mesh;

  const float BARRIER_WIDTH = 7.5f;
  std::vector<RoadVertex> leftVertices, rightVertices;
  std::vector<GLuint> leftIndices, rightIndices;

  float totalDistance = 0.0f;
  size_t numPoints = splinePoints.size();

  for (size_t i = 0; i < numPoints; ++i) {
    const auto& current = splinePoints[i];
    const auto& next = splinePoints[(i + 1) % numPoints];

    // Calculate direction and perpendicular
    glm::vec2 direction = next.position - current.position;
    glm::vec2 perp = glm::normalize(glm::vec2(-direction.y, direction.x));

    // Calculate barrier positions
    glm::vec2 leftRoadEdge = current.position + perp * current.roadWidth;
    glm::vec2 rightRoadEdge = current.position - perp * current.roadWidth;

    // Left barrier vertices
    if (current.barrierDistance.x > 0.0f) {
      // Start from road edge, move out past offroad area by barrier distance
      glm::vec2 leftBarrierPos = leftRoadEdge + perp * current.barrierDistance.x;
      glm::vec2 leftBarrierEnd = leftBarrierPos + perp * BARRIER_WIDTH;

      RoadVertex v1, v2;
      v1.position = leftBarrierPos;
      v2.position = leftBarrierEnd;
      v1.uv = glm::vec2(0.0f, totalDistance * 0.01f);
      v2.uv = glm::vec2(1.0f, totalDistance * 0.01f);
      v1.distanceAlong = totalDistance;
      v2.distanceAlong = totalDistance;
      v1.depth = 0.2f;
      v2.depth = 0.2f;

      leftVertices.push_back(v1);
      leftVertices.push_back(v2);
    }

    // Right barrier vertices
    if (current.barrierDistance.y > 0.0f) {
      // Start from road edge, move out past offroad area by barrier distance
      glm::vec2 rightBarrierPos = rightRoadEdge - perp * current.barrierDistance.y;
      glm::vec2 rightBarrierEnd = rightBarrierPos - perp * BARRIER_WIDTH;

      RoadVertex v1, v2;
      v1.position = rightBarrierPos;
      v2.position = rightBarrierEnd;
      v1.uv = glm::vec2(0.0f, totalDistance * 0.01f);
      v2.uv = glm::vec2(1.0f, totalDistance * 0.01f);
      v1.distanceAlong = totalDistance;
      v2.distanceAlong = totalDistance;
      v1.depth = 0.2f;
      v2.depth = 0.2f;

      rightVertices.push_back(v1);
      rightVertices.push_back(v2);
    }

    // Create indices
    if (i < numPoints - 1) {
      if (current.barrierDistance.x > 0.0f && next.barrierDistance.x > 0.0f) {
        GLuint baseIndex = static_cast<GLuint>((leftVertices.size() / 2 - 1) * 2);
        leftIndices.push_back(baseIndex);
        leftIndices.push_back(baseIndex + 1);
        leftIndices.push_back(baseIndex + 2);

        leftIndices.push_back(baseIndex + 2);
        leftIndices.push_back(baseIndex + 1);
        leftIndices.push_back(baseIndex + 3);
      }

      if (current.barrierDistance.y > 0.0f && next.barrierDistance.y > 0.0f) {
        GLuint baseIndex = static_cast<GLuint>((rightVertices.size() / 2 - 1) * 2);
        rightIndices.push_back(baseIndex);
        rightIndices.push_back(baseIndex + 1);
        rightIndices.push_back(baseIndex + 2);

        rightIndices.push_back(baseIndex + 2);
        rightIndices.push_back(baseIndex + 1);
        rightIndices.push_back(baseIndex + 3);
      }
    }

    totalDistance += 1.0;
  }

  // Close the loop
  if (numPoints > 2) {
    if (splinePoints[0].barrierDistance.x > 0.0f &&
      splinePoints[numPoints - 1].barrierDistance.x > 0.0f &&
      !leftVertices.empty()) {
      GLuint lastIndex = static_cast<GLuint>((leftVertices.size() / 2 - 1) * 2);
      leftIndices.push_back(lastIndex);
      leftIndices.push_back(lastIndex + 1);
      leftIndices.push_back(0);

      leftIndices.push_back(0);
      leftIndices.push_back(lastIndex + 1);
      leftIndices.push_back(1);
    }

    if (splinePoints[0].barrierDistance.y > 0.0f &&
      splinePoints[numPoints - 1].barrierDistance.y > 0.0f &&
      !rightVertices.empty()) {
      GLuint lastIndex = static_cast<GLuint>((rightVertices.size() / 2 - 1) * 2);
      rightIndices.push_back(lastIndex);
      rightIndices.push_back(lastIndex + 1);
      rightIndices.push_back(0);

      rightIndices.push_back(0);
      rightIndices.push_back(lastIndex + 1);
      rightIndices.push_back(1);
    }
  }

  mesh.leftSide.vertices = std::move(leftVertices);
  mesh.leftSide.indices = std::move(leftIndices);
  mesh.rightSide.vertices = std::move(rightVertices);
  mesh.rightSide.indices = std::move(rightIndices);

  createBuffers(mesh.leftSide);
  createBuffers(mesh.rightSide);

  return mesh;
}

void RoadMeshGenerator::createBuffers(MeshData& mesh) {
  // Cleanup any existing buffers
  mesh.cleanup();

  if (mesh.vertices.empty() || mesh.indices.empty()) {
    std::cout << "Cannot create buffers: No vertex or index data\n";
    return;
  }

  // Generate and bind VAO
  glGenVertexArrays(1, &mesh.vao);
  glBindVertexArray(mesh.vao);

  // Create and populate vertex buffer
  glGenBuffers(1, &mesh.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
  glBufferData(GL_ARRAY_BUFFER,
    mesh.vertices.size() * sizeof(RoadVertex),
    mesh.vertices.data(),
    GL_STATIC_DRAW);

  // Create and populate index buffer
  glGenBuffers(1, &mesh.ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
    mesh.indices.size() * sizeof(GLuint),
    mesh.indices.data(),
    GL_STATIC_DRAW);

  // Position attribute (vec2)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RoadVertex),
    (void*)offsetof(RoadVertex, position));

  // UV attribute (vec2)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RoadVertex),
    (void*)offsetof(RoadVertex, uv));

  // Distance attribute (float)
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(RoadVertex),
    (void*)offsetof(RoadVertex, distanceAlong));

  // Depth attribute (float)
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(RoadVertex),
    (void*)offsetof(RoadVertex, depth));

  glBindVertexArray(0);

  // Debug output
  std::cout << "Created buffers - VAO: " << mesh.vao
    << ", VBO: " << mesh.vbo
    << ", IBO: " << mesh.ibo
    << ", Vertices: " << mesh.vertices.size()
    << ", Indices: " << mesh.indices.size() << "\n";
}

void RoadMeshGenerator::createStartLineBuffers(StartLineMeshData& mesh) {
  // Cleanup any existing buffers
  mesh.cleanup();

  if (mesh.vertices.empty() || mesh.indices.empty()) {
    std::cout << "Cannot create start line buffers: No vertex or index data\n";
    return;
  }

  // Generate and bind VAO
  glGenVertexArrays(1, &mesh.vao);
  glBindVertexArray(mesh.vao);

  // Create and populate vertex buffer
  glGenBuffers(1, &mesh.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
  glBufferData(GL_ARRAY_BUFFER,
    mesh.vertices.size() * sizeof(RoadVertex),
    mesh.vertices.data(),
    GL_STATIC_DRAW);

  // Create and populate index buffer
  glGenBuffers(1, &mesh.ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
    mesh.indices.size() * sizeof(GLuint),
    mesh.indices.data(),
    GL_STATIC_DRAW);

  // Setup vertex attributes exactly as in createBuffers
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(RoadVertex),
    (void*)offsetof(RoadVertex, position));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(RoadVertex),
    (void*)offsetof(RoadVertex, uv));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(RoadVertex),
    (void*)offsetof(RoadVertex, distanceAlong));

  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(RoadVertex),
    (void*)offsetof(RoadVertex, depth));

  glBindVertexArray(0);

  std::cout << "Created start line buffers - VAO: " << mesh.vao
    << ", VBO: " << mesh.vbo
    << ", IBO: " << mesh.ibo
    << ", Vertices: " << mesh.vertices.size()
    << ", Indices: " << mesh.indices.size() << "\n";
}

glm::vec2 RoadMeshGenerator::calculateSmoothPerpendicular(
  const glm::vec2& prev,
  const glm::vec2& curr,
  const glm::vec2& next) {

  // Calculate the incoming and outgoing directions
  glm::vec2 inDir = glm::normalize(curr - prev);
  glm::vec2 outDir = glm::normalize(next - curr);

  // Calculate the angle between the directions
  float dot = glm::dot(inDir, outDir);
  dot = glm::clamp(dot, -1.0f, 1.0f);
  float angle = std::acos(dot);

  // Calculate weighted average direction based on the angle
  glm::vec2 avgDir;
  if (angle > 0.01f) { // If there's a significant turn
    // Use a weighted average that favors the sharper turning direction
    float inWeight = 1.0f + angle;
    float outWeight = 1.0f + angle;
    avgDir = glm::normalize((inDir * inWeight + outDir * outWeight) * 0.5f);
  }
  else {
    // For nearly straight sections, use simple average
    avgDir = glm::normalize(inDir + outDir);
  }

  // Calculate perpendicular vector
  glm::vec2 perp(-avgDir.y, avgDir.x);

  // Scale the perpendicular vector based on the angle to prevent overlap
  float scaleFactor = 1.0f / (std::cos(angle * 0.5f) + 0.01f);
  scaleFactor = glm::clamp(scaleFactor, 1.0f, 1.5f);

  return perp * scaleFactor;
}

RoadMeshGenerator::MeshData RoadMeshGenerator::generateRoadMesh(const SplineTrack& track, int baseLOD) {
  MeshData mesh;
  auto splinePoints = track.getSplinePoints(baseLOD);

  if (splinePoints.size() < 2) {
    std::cout << "Not enough spline points to generate mesh\n";
    return mesh;
  }

  float totalDistance = 0.0f;
  std::vector<RoadVertex> vertices;
  std::vector<GLuint> indices;

  vertices.reserve((splinePoints.size() + 1) * 3);
  indices.reserve(splinePoints.size() * 6);

  for (size_t i = 0; i < splinePoints.size(); ++i) {
    const auto& current = splinePoints[i];
    const auto& next = splinePoints[(i + 1) % splinePoints.size()];

    glm::vec2 direction = glm::normalize(next.position - current.position);
    glm::vec2 perp(-direction.y, direction.x);

    RoadVertex leftEdge, center, rightEdge;

    leftEdge.position = current.position + perp * current.roadWidth;
    center.position = current.position;
    rightEdge.position = current.position - perp * current.roadWidth;

    leftEdge.uv = glm::vec2(0.0f, totalDistance * 0.01f);
    center.uv = glm::vec2(0.5f, totalDistance * 0.01f);
    rightEdge.uv = glm::vec2(1.0f, totalDistance * 0.01f);

    leftEdge.distanceAlong = totalDistance;
    center.distanceAlong = totalDistance;
    rightEdge.distanceAlong = totalDistance;

    float baseDepth = 0.01f * static_cast<float>(i) / splinePoints.size();
    center.depth = baseDepth + 0.01f;
    leftEdge.depth = baseDepth;
    rightEdge.depth = baseDepth;

    vertices.push_back(leftEdge);
    vertices.push_back(center);
    vertices.push_back(rightEdge);

    if (i < splinePoints.size() - 1) {
      GLuint baseIndex = static_cast<GLuint>(i * 3);
      indices.push_back(baseIndex);
      indices.push_back(baseIndex + 1);
      indices.push_back(baseIndex + 3);

      indices.push_back(baseIndex + 3);
      indices.push_back(baseIndex + 1);
      indices.push_back(baseIndex + 4);

      indices.push_back(baseIndex + 1);
      indices.push_back(baseIndex + 2);
      indices.push_back(baseIndex + 4);

      indices.push_back(baseIndex + 4);
      indices.push_back(baseIndex + 2);
      indices.push_back(baseIndex + 5);
    }

    totalDistance += glm::length(next.position - current.position);
  }

  // Close the loop
  if (splinePoints.size() > 2) {
    GLuint lastIndex = static_cast<GLuint>((splinePoints.size() - 1) * 3);
    GLuint firstIndex = 0;

    indices.push_back(lastIndex);
    indices.push_back(lastIndex + 1);
    indices.push_back(firstIndex);

    indices.push_back(firstIndex);
    indices.push_back(lastIndex + 1);
    indices.push_back(firstIndex + 1);

    indices.push_back(lastIndex + 1);
    indices.push_back(lastIndex + 2);
    indices.push_back(firstIndex + 1);

    indices.push_back(firstIndex + 1);
    indices.push_back(lastIndex + 2);
    indices.push_back(firstIndex + 2);
  }

  mesh.vertices = std::move(vertices);
  mesh.indices = std::move(indices);
  createBuffers(mesh);
  return mesh;
}

RoadMeshGenerator::StartLineMeshData RoadMeshGenerator::generateStartLineMesh(const SplineTrack& track) {
  StartLineMeshData mesh;
  const TrackNode* startNode = track.getStartLineNode();
  if (!startNode) {
    std::cout << "No start node found\n";
    return mesh;
  }

  // Get direction
  glm::vec2 direction = track.getTrackDirectionAtNode(startNode);
  direction = glm::normalize(direction);
  glm::vec2 perp(-direction.y, direction.x);

  float roadWidth = startNode->getRoadWidth();
  float lineWidth = roadWidth * 2.0f;
  float lineThickness = 10.0f;

  glm::vec2 center = startNode->getPosition();
  glm::vec2 halfWidth = perp * (lineWidth * 0.5f);
  glm::vec2 halfThickness = direction * (lineThickness * 0.5f);

  RoadVertex v1, v2, v3, v4;
  v1.position = center - halfWidth - halfThickness;
  v2.position = center + halfWidth - halfThickness;
  v3.position = center + halfWidth + halfThickness;
  v4.position = center - halfWidth + halfThickness;

  // UV coordinates
  v1.uv = glm::vec2(0.0f, 0.0f);
  v2.uv = glm::vec2(1.0f, 0.0f);
  v3.uv = glm::vec2(1.0f, 1.0f);
  v4.uv = glm::vec2(0.0f, 1.0f);

  // Distance along and depth
  v1.distanceAlong = v2.distanceAlong = v3.distanceAlong = v4.distanceAlong = 0.0f;
  float startLineDepth = -0.01f;
  v1.depth = v2.depth = v3.depth = v4.depth = startLineDepth;

  mesh.vertices = { v1, v2, v3, v4 };
  mesh.indices = { 0, 1, 2, 2, 3, 0 };

  createStartLineBuffers(mesh);
  return mesh;
}

RoadMeshGenerator::OffroadMeshData RoadMeshGenerator::generateOffroadMesh(const SplineTrack& track, int baseLOD) {
  OffroadMeshData mesh;
  auto splinePoints = track.getSplinePoints(baseLOD);

  if (splinePoints.size() < 2) return mesh;

  float totalDistance = 0.0f;
  std::vector<RoadVertex> leftVertices;
  std::vector<RoadVertex> rightVertices;
  std::vector<GLuint> leftIndices;
  std::vector<GLuint> rightIndices;

  // Reserve space
  leftVertices.reserve((splinePoints.size() + 1) * 2);
  rightVertices.reserve((splinePoints.size() + 1) * 2);
  leftIndices.reserve(splinePoints.size() * 6);
  rightIndices.reserve(splinePoints.size() * 6);

  for (size_t i = 0; i < splinePoints.size(); ++i) {
    const auto& current = splinePoints[i];
    const auto& next = splinePoints[(i + 1) % splinePoints.size()];
    const auto& prev = splinePoints[(i > 0) ? i - 1 : splinePoints.size() - 1];

    glm::vec2 perp = calculateSmoothPerpendicular(prev.position, current.position, next.position);

    // Left side offroad
    {
      RoadVertex roadEdge, offroadEdge;
      roadEdge.position = current.position + perp * current.roadWidth;
      roadEdge.uv = glm::vec2(1.0f, totalDistance * 0.01f);
      roadEdge.distanceAlong = totalDistance;
      roadEdge.depth = 0.1f * static_cast<float>(i) / splinePoints.size();

      offroadEdge.position = current.position + perp * (current.roadWidth + current.offroadWidth.x);
      offroadEdge.uv = glm::vec2(0.0f, totalDistance * 0.01f);
      offroadEdge.distanceAlong = totalDistance;
      offroadEdge.depth = roadEdge.depth;

      leftVertices.push_back(roadEdge);
      leftVertices.push_back(offroadEdge);
    }

    // Right side offroad
    {
      RoadVertex roadEdge, offroadEdge;
      roadEdge.position = current.position - perp * current.roadWidth;
      roadEdge.uv = glm::vec2(0.0f, totalDistance * 0.01f);
      roadEdge.distanceAlong = totalDistance;
      roadEdge.depth = 0.1f * static_cast<float>(i) / splinePoints.size();

      offroadEdge.position = current.position - perp * (current.roadWidth + current.offroadWidth.y);
      offroadEdge.uv = glm::vec2(1.0f, totalDistance * 0.01f);
      offroadEdge.distanceAlong = totalDistance;
      offroadEdge.depth = roadEdge.depth;

      rightVertices.push_back(roadEdge);
      rightVertices.push_back(offroadEdge);
    }

    if (i < splinePoints.size() - 1) {
      GLuint baseIndex = static_cast<GLuint>(i * 2);
      GLuint nextBaseIndex = baseIndex + 2;

      // Left side indices
      leftIndices.push_back(baseIndex);
      leftIndices.push_back(baseIndex + 1);
      leftIndices.push_back(nextBaseIndex);
      leftIndices.push_back(nextBaseIndex);
      leftIndices.push_back(baseIndex + 1);
      leftIndices.push_back(nextBaseIndex + 1);

      // Right side indices
      rightIndices.push_back(baseIndex);
      rightIndices.push_back(baseIndex + 1);
      rightIndices.push_back(nextBaseIndex);
      rightIndices.push_back(nextBaseIndex);
      rightIndices.push_back(baseIndex + 1);
      rightIndices.push_back(nextBaseIndex + 1);
    }

    totalDistance += glm::length(next.position - current.position);
  }

  // Close the loop
  if (leftVertices.size() >= 4) {
    GLuint lastIndex = static_cast<GLuint>(leftVertices.size() - 2);

    // Left side
    leftIndices.push_back(lastIndex);
    leftIndices.push_back(lastIndex + 1);
    leftIndices.push_back(0);
    leftIndices.push_back(0);
    leftIndices.push_back(lastIndex + 1);
    leftIndices.push_back(1);

    // Right side
    rightIndices.push_back(lastIndex);
    rightIndices.push_back(lastIndex + 1);
    rightIndices.push_back(0);
    rightIndices.push_back(0);
    rightIndices.push_back(lastIndex + 1);
    rightIndices.push_back(1);
  }

  mesh.leftSide.vertices = std::move(leftVertices);
  mesh.leftSide.indices = std::move(leftIndices);
  mesh.rightSide.vertices = std::move(rightVertices);
  mesh.rightSide.indices = std::move(rightIndices);

  createBuffers(mesh.leftSide);
  createBuffers(mesh.rightSide);

  return mesh;
}
