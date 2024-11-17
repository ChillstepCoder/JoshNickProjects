// RoadMeshGenerator.cpp

#include "RoadMeshGenerator.h"

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



